"""Tests for the Julius evaluation system.

Tests the full pipeline:
1. JuliusSandbox: clone, patch, build
2. JuliusTestRunner: run tests with ASan
3. ASan parser: parse error output
4. JuliusEvaluator: end-to-end evaluation
5. Integration with main runner
"""

import pytest
from pathlib import Path
from unittest.mock import Mock, MagicMock, patch
import tempfile
import os

from evaluation.asan_parser import (
    ASanErrorType,
    ASanReport,
    ASanError,
    parse_asan_output,
    has_asan_error,
    get_asan_summary,
)
from evaluation.julius_test_runner import JuliusTestRunner, JuliusTestResult
from evaluation.julius_evaluator import (
    JuliusEvaluator,
    JuliusEvaluationResult,
    JuliusTaskConfig,
    format_julius_result,
)
from harness.julius_sandbox import (
    JuliusSandbox,
    JuliusSandboxConfig,
    BuildResult,
    CloneResult,
)
from harness.patch_utils import (
    parse_unified_diff,
    extract_model_patch,
    compare_patches,
    validate_patch,
)


class TestASanParser:
    """Tests for AddressSanitizer output parsing."""

    def test_parse_double_free(self):
        """Test parsing double-free error from ASan output."""
        asan_output = """
==12345==ERROR: AddressSanitizer: attempting double-free on 0x602000000010 in thread T0:
    #0 0x7f123 in free (/lib/libc.so.6+0x123)
    #1 0x4012ab in smacker_close src/core/smacker.c:546:5
    #2 0x401123 in main test_smacker.c:42:5

0x602000000010 is located 0 bytes inside of 400-byte region [0x602000000010,0x6020000001a0)
freed by thread T0 here:
    #0 0x7f123 in free (/lib/libc.so.6+0x123)
    #1 0x4011ab in read_frame_info src/core/smacker.c:458:9

SUMMARY: AddressSanitizer: double-free (/lib/libc.so.6+0x123) in free
"""
        report = parse_asan_output(asan_output)

        assert report.has_errors
        assert report.error_count == 1
        assert report.errors[0].error_type == ASanErrorType.DOUBLE_FREE

    def test_parse_heap_buffer_overflow(self):
        """Test parsing heap-buffer-overflow error."""
        asan_output = """
==54321==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x602000000110 at pc 0x401234
READ of size 4 at 0x602000000110 thread T0
    #0 0x401234 in process_data src/data.c:123:10

SUMMARY: AddressSanitizer: heap-buffer-overflow src/data.c:123:10 in process_data
"""
        report = parse_asan_output(asan_output)

        assert report.has_errors
        assert report.errors[0].error_type == ASanErrorType.HEAP_BUFFER_OVERFLOW

    def test_parse_use_after_free(self):
        """Test parsing use-after-free error."""
        asan_output = """
==99999==ERROR: AddressSanitizer: heap-use-after-free on address 0x602000000010
READ of size 8 at 0x602000000010 thread T0
    #0 0x401234 in access_freed src/mem.c:50:5

SUMMARY: AddressSanitizer: heap-use-after-free src/mem.c:50 in access_freed
"""
        report = parse_asan_output(asan_output)

        assert report.has_errors
        assert report.errors[0].error_type == ASanErrorType.USE_AFTER_FREE

    def test_no_asan_errors(self):
        """Test that clean output produces empty report."""
        clean_output = """
Running tests...
Test 1: PASS
Test 2: PASS
All tests completed successfully.
"""
        report = parse_asan_output(clean_output)

        assert not report.has_errors
        assert report.error_count == 0

    def test_has_asan_error_helper(self):
        """Test quick check helper function."""
        assert has_asan_error("==123==ERROR: AddressSanitizer: double-free")
        assert has_asan_error("==456==ERROR: LeakSanitizer: detected memory leaks")
        assert not has_asan_error("All tests passed successfully")

    def test_get_asan_summary(self):
        """Test summary extraction."""
        output = "SUMMARY: AddressSanitizer: double-free in free"
        summary = get_asan_summary(output)
        assert summary == "double-free in free"


class TestPatchUtils:
    """Tests for patch manipulation utilities."""

    def test_parse_unified_diff(self):
        """Test parsing a unified diff."""
        patch_content = """diff --git a/src/core/smacker.c b/src/core/smacker.c
index 00c127f..e289c40 100644
--- a/src/core/smacker.c
+++ b/src/core/smacker.c
@@ -434,6 +434,16 @@ static int read_header(smacker s)
     return 1;
 }

+static void free_frame_info(smacker s)
+{
+    free(s->frame_sizes);
+    free(s->frame_offsets);
+    free(s->frame_types);
+    s->frame_sizes = 0;
+    s->frame_offsets = 0;
+    s->frame_types = 0;
+}
+
 static int read_frame_info(smacker s)
"""
        patch = parse_unified_diff(patch_content)

        assert len(patch.files) == 1
        assert patch.files[0].old_path == "src/core/smacker.c"
        assert patch.files[0].new_path == "src/core/smacker.c"
        assert len(patch.files[0].hunks) >= 1

    def test_extract_model_patch_from_code_block(self):
        """Test extracting patch from markdown code block."""
        model_response = """
I've analyzed the bug and here's my fix:

```diff
diff --git a/src/core/smacker.c b/src/core/smacker.c
--- a/src/core/smacker.c
+++ b/src/core/smacker.c
@@ -450,6 +450,7 @@ static int read_frame_info(smacker s)
     if (!s->frame_sizes) {
+        s->frame_sizes = 0;
         return 0;
     }
```

This fix nullifies the pointer after freeing to prevent double-free.
"""
        patch = extract_model_patch(model_response)

        assert patch is not None
        assert "diff --git" in patch
        assert "+        s->frame_sizes = 0;" in patch

    def test_extract_model_patch_no_patch(self):
        """Test extraction when no patch is present."""
        model_response = "I don't understand the problem."
        patch = extract_model_patch(model_response)
        assert patch is None

    def test_validate_patch(self):
        """Test patch validation."""
        valid_patch = """diff --git a/src/test.c b/src/test.c
--- a/src/test.c
+++ b/src/test.c
@@ -1,3 +1,4 @@
 #include <stdio.h>
+#include <stdlib.h>
 int main() {}
"""
        is_valid, error = validate_patch(valid_patch)
        assert is_valid
        assert error == ""

    def test_compare_patches_identical(self):
        """Test comparing identical patches."""
        patch1 = """diff --git a/src/test.c b/src/test.c
--- a/src/test.c
+++ b/src/test.c
@@ -1,3 +1,4 @@
 context
+added line
 context
"""
        similarity = compare_patches(patch1, patch1)
        assert similarity == 1.0

    def test_compare_patches_different(self):
        """Test comparing different patches."""
        patch1 = """diff --git a/src/test.c b/src/test.c
--- a/src/test.c
+++ b/src/test.c
@@ -1,3 +1,4 @@
 context
+added line 1
 context
"""
        patch2 = """diff --git a/src/test.c b/src/test.c
--- a/src/test.c
+++ b/src/test.c
@@ -1,3 +1,4 @@
 context
+completely different line
 context
"""
        similarity = compare_patches(patch1, patch2)
        assert 0.0 < similarity < 1.0


class TestJuliusSandboxConfig:
    """Tests for Julius sandbox configuration."""

    def test_default_config(self):
        """Test default configuration values."""
        config = JuliusSandboxConfig()
        assert config.timeout == 300
        assert config.enable_asan is True
        assert config.build_type == "Debug"
        assert config.cc == "clang"

    def test_custom_config(self):
        """Test custom configuration."""
        config = JuliusSandboxConfig(
            timeout=600,
            enable_asan=False,
            enable_ubsan=True,
            build_type="Release",
        )
        assert config.timeout == 600
        assert config.enable_asan is False
        assert config.enable_ubsan is True
        assert config.build_type == "Release"


class TestJuliusEvaluationResult:
    """Tests for Julius evaluation result."""

    def test_score_calculation(self):
        """Test score percentage calculation."""
        result = JuliusEvaluationResult(
            task_id="test-001",
            model_name="test-model",
            success=True,
            compiles=True,
            no_asan_errors=True,
            tests_pass=True,
            matches_fix_structure=False,
            total_score=4.0,
            max_score=5.0,
        )
        assert result.score_percentage == 80.0

    def test_full_score(self):
        """Test full score with bonus."""
        result = JuliusEvaluationResult(
            task_id="test-001",
            model_name="test-model",
            success=True,
            compiles=True,
            no_asan_errors=True,
            tests_pass=True,
            matches_fix_structure=True,
            total_score=5.0,
            max_score=5.0,
        )
        assert result.score_percentage == 100.0


class TestFormatJuliusResult:
    """Tests for result formatting."""

    def test_format_passing_result(self):
        """Test formatting a passing result."""
        result = JuliusEvaluationResult(
            task_id="julius-001",
            model_name="gpt-4",
            success=True,
            compiles=True,
            no_asan_errors=True,
            tests_pass=True,
            matches_fix_structure=True,
            total_score=5.0,
            elapsed_time=12.5,
            patch_similarity=0.95,
        )

        output = format_julius_result(result)

        assert "julius-001" in output
        assert "gpt-4" in output
        assert "PASSED" in output
        assert "5.0/5.0" in output
        assert "[✓]" in output

    def test_format_failing_result(self):
        """Test formatting a failing result."""
        result = JuliusEvaluationResult(
            task_id="julius-001",
            model_name="test-model",
            success=False,
            compiles=True,
            no_asan_errors=False,
            tests_pass=False,
            total_score=1.0,
            elapsed_time=5.0,
            error="ASan detected double-free",
        )

        output = format_julius_result(result)

        assert "FAILED" in output
        assert "1.0/5.0" in output
        assert "[✗]" in output
        assert "ASan detected double-free" in output


class TestJuliusTaskConfig:
    """Tests for Julius task configuration loading."""

    def test_load_task_config(self, tmp_path):
        """Test loading task configuration from JSON."""
        task_dir = tmp_path / "test-task"
        task_dir.mkdir()

        task_json = {
            "id": "julius-test",
            "name": "Test Task",
            "category": "memory-safety",
            "tier": 3,
            "engine": "julius",
            "description": "Test description",
            "evaluation": ["unit-test", "asan"],
            "commit": "abc123",
            "files_to_modify": ["src/test.c"],
            "requires_assets": False,
            "expected_asan_error": "double-free",
        }

        import json
        (task_dir / "task.json").write_text(json.dumps(task_json))
        (task_dir / "prompt.md").write_text("Fix the bug")
        (task_dir / "buggy.patch").write_text("diff content")

        # Create a mock model
        mock_model = Mock()
        mock_model.get_name.return_value = "test-model"

        evaluator = JuliusEvaluator(task_dir, mock_model)
        config = evaluator.load_task()

        assert config.id == "julius-test"
        assert config.commit == "abc123"
        assert config.expected_asan_error == "double-free"


class TestJuliusTestResultParsing:
    """Tests for parsing test output."""

    def test_parse_pass_fail_format(self):
        """Test parsing X passed, Y failed format."""
        output = "Running tests... 5 passed, 2 failed"

        runner = JuliusTestRunner(Mock())
        passed, failed, total = runner._parse_test_output(output)

        assert passed == 5
        assert failed == 2
        assert total == 7

    def test_parse_results_format(self):
        """Test parsing Results: X/Y format."""
        output = "Results: 3/4 tests passed"

        runner = JuliusTestRunner(Mock())
        passed, failed, total = runner._parse_test_output(output)

        assert passed == 3
        assert failed == 1
        assert total == 4


# Integration tests that require network/filesystem
@pytest.mark.integration
class TestJuliusIntegration:
    """Integration tests for the Julius evaluation system.

    These tests require network access and may take longer to run.
    Mark them with @pytest.mark.integration and skip by default.
    """

    @pytest.mark.skip(reason="Requires network access to clone Julius")
    def test_clone_julius(self):
        """Test cloning Julius repository."""
        config = JuliusSandboxConfig(use_cache=False)

        with JuliusSandbox(config) as sandbox:
            result = sandbox.clone(commit="f722d9c")
            assert result.success
            assert sandbox.repo_dir is not None
            assert (sandbox.repo_dir / "src" / "core" / "smacker.c").exists()

    @pytest.mark.skip(reason="Requires network access and build tools")
    def test_build_julius_with_asan(self):
        """Test building Julius with AddressSanitizer."""
        config = JuliusSandboxConfig(enable_asan=True)

        with JuliusSandbox(config) as sandbox:
            clone_result = sandbox.clone()
            assert clone_result.success

            build_result = sandbox.build()
            # Building Julius requires SDL2 and other deps
            # May fail in CI without proper setup


# Mark the entire test module for easy discovery
def test_module_importable():
    """Verify all Julius modules can be imported."""
    from evaluation.julius_evaluator import JuliusEvaluator
    from evaluation.julius_test_runner import JuliusTestRunner
    from evaluation.asan_parser import parse_asan_output
    from harness.julius_sandbox import JuliusSandbox
    from harness.patch_utils import apply_patch

    assert JuliusEvaluator is not None
    assert JuliusTestRunner is not None
    assert parse_asan_output is not None
    assert JuliusSandbox is not None
    assert apply_patch is not None
