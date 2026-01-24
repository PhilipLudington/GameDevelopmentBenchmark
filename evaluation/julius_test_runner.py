"""Test runner for Julius benchmark tasks.

This module handles running tests for Julius tasks, including:
- Compiling test binaries against Julius source
- Running tests with AddressSanitizer
- Parsing test output and ASan reports
- Determining test pass/fail status
"""

import re
import shutil
import subprocess
import time
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, List, Optional

from evaluation.asan_parser import ASanReport, parse_asan_output, has_asan_error
from harness.julius_sandbox import JuliusSandbox, TestExecutionResult


@dataclass
class JuliusTestResult:
    """Result of running Julius tests."""

    passed: int
    failed: int
    errors: int
    total: int
    success: bool
    output: str
    elapsed_time: float = 0.0
    compilation_error: Optional[str] = None
    asan_report: Optional[ASanReport] = None
    test_cases: Optional[List[Dict]] = None


class JuliusTestRunner:
    """Runs tests for Julius benchmark tasks.

    Handles the complete test lifecycle:
    1. Copy test files to sandbox
    2. Compile tests with ASan
    3. Run test executable
    4. Parse results and ASan output
    """

    def __init__(
        self,
        sandbox: JuliusSandbox,
        timeout: int = 120,
    ):
        """Initialize the Julius test runner.

        Args:
            sandbox: JuliusSandbox with Julius repository cloned
            timeout: Maximum test execution time in seconds
        """
        self.sandbox = sandbox
        self.timeout = timeout

    def run(
        self,
        test_dir: Path,
        extra_sources: Optional[List[str]] = None,
    ) -> JuliusTestResult:
        """Run Julius tests from a task's test directory.

        This method:
        1. Reads Makefile or compiles tests directly
        2. Runs the test executable
        3. Parses test output and ASan reports
        4. Returns structured results

        Args:
            test_dir: Path to task's tests/ directory
            extra_sources: Additional source files to link

        Returns:
            JuliusTestResult with test outcomes
        """
        start_time = time.time()

        if not test_dir.exists():
            return JuliusTestResult(
                passed=0,
                failed=0,
                errors=1,
                total=0,
                success=False,
                output=f"Test directory not found: {test_dir}",
            )

        # Check for Makefile
        makefile = test_dir / "Makefile"
        if makefile.exists():
            return self._run_makefile_tests(test_dir, start_time)
        else:
            return self._run_direct_compile_tests(test_dir, extra_sources, start_time)

    def _run_makefile_tests(
        self,
        test_dir: Path,
        start_time: float,
    ) -> JuliusTestResult:
        """Run tests using Makefile.

        Args:
            test_dir: Test directory with Makefile
            start_time: Test start time

        Returns:
            JuliusTestResult
        """
        if not self.sandbox.working_dir:
            return JuliusTestResult(
                passed=0,
                failed=0,
                errors=1,
                total=0,
                success=False,
                output="Sandbox not initialized",
                elapsed_time=time.time() - start_time,
            )

        # Copy test directory to sandbox
        sandbox_tests = self.sandbox.working_dir / "tests"
        if sandbox_tests.exists():
            shutil.rmtree(sandbox_tests)
        shutil.copytree(test_dir, sandbox_tests)

        # Set up environment with Julius source path
        import os
        env = os.environ.copy()
        if self.sandbox.repo_dir:
            env["JULIUS_SRC"] = str(self.sandbox.repo_dir / "src")

        # Enable ASan in environment
        if self.sandbox.config.enable_asan:
            env["ASAN_OPTIONS"] = "detect_leaks=1:abort_on_error=0:print_stacktrace=1"
            env["CFLAGS"] = "-fsanitize=address -fno-omit-frame-pointer -g"
            env["LDFLAGS"] = "-fsanitize=address"

        try:
            # Run make test
            result = subprocess.run(
                ["make", "-C", str(sandbox_tests), "test"],
                capture_output=True,
                text=True,
                timeout=self.timeout + 60,  # Extra time for compilation
                env=env,
            )

            elapsed = time.time() - start_time
            output = result.stdout + result.stderr

            # Check for compilation errors
            compilation_error = self._check_compilation_error(output)
            if compilation_error:
                return JuliusTestResult(
                    passed=0,
                    failed=0,
                    errors=1,
                    total=0,
                    success=False,
                    output=output,
                    elapsed_time=elapsed,
                    compilation_error=compilation_error,
                )

            # Parse ASan output
            asan_report = parse_asan_output(output)

            # Parse test results
            passed, failed, total = self._parse_test_output(output)

            # Determine success
            # Success requires: tests pass AND no ASan errors
            success = (
                result.returncode == 0
                and failed == 0
                and not asan_report.has_errors
            )

            return JuliusTestResult(
                passed=passed,
                failed=failed,
                errors=0,
                total=total,
                success=success,
                output=output,
                elapsed_time=elapsed,
                asan_report=asan_report,
            )

        except subprocess.TimeoutExpired as e:
            elapsed = time.time() - start_time
            output = (e.stdout or "") + (e.stderr or "")
            return JuliusTestResult(
                passed=0,
                failed=0,
                errors=1,
                total=0,
                success=False,
                output=f"Test timeout after {self.timeout}s\n{output}",
                elapsed_time=elapsed,
            )

        except Exception as e:
            elapsed = time.time() - start_time
            return JuliusTestResult(
                passed=0,
                failed=0,
                errors=1,
                total=0,
                success=False,
                output=f"Test execution error: {e}",
                elapsed_time=elapsed,
            )

    def _run_direct_compile_tests(
        self,
        test_dir: Path,
        extra_sources: Optional[List[str]],
        start_time: float,
    ) -> JuliusTestResult:
        """Compile and run tests directly without Makefile.

        Args:
            test_dir: Test directory with .c files
            extra_sources: Additional source files to link
            start_time: Test start time

        Returns:
            JuliusTestResult
        """
        # Find test source files
        test_sources = list(test_dir.glob("test_*.c"))
        if not test_sources:
            test_sources = list(test_dir.glob("*.c"))

        if not test_sources:
            return JuliusTestResult(
                passed=0,
                failed=0,
                errors=1,
                total=0,
                success=False,
                output="No test source files found",
                elapsed_time=time.time() - start_time,
            )

        # Build tests
        source_names = [s.name for s in test_sources]
        if extra_sources:
            source_names.extend(extra_sources)

        build_result = self.sandbox.build_test(
            test_dir=test_dir,
            test_sources=source_names,
            output_name="test_runner",
        )

        if not build_result.success:
            elapsed = time.time() - start_time
            return JuliusTestResult(
                passed=0,
                failed=0,
                errors=1,
                total=0,
                success=False,
                output=build_result.stdout + build_result.stderr,
                elapsed_time=elapsed,
                compilation_error=build_result.error,
            )

        # Run tests
        test_executable = build_result.build_dir / "test_runner"
        exec_result = self.sandbox.run_test(test_executable, timeout=self.timeout)

        elapsed = time.time() - start_time
        output = exec_result.stdout + exec_result.stderr

        # Parse ASan output
        asan_report = parse_asan_output(output)

        # Parse test results
        passed, failed, total = self._parse_test_output(output)

        # Handle case where we couldn't parse results
        if total == 0:
            if exec_result.success and not asan_report.has_errors:
                passed = 1
                total = 1
            else:
                failed = 1
                total = 1

        success = (
            exec_result.success
            and failed == 0
            and not asan_report.has_errors
        )

        return JuliusTestResult(
            passed=passed,
            failed=failed,
            errors=0,
            total=total,
            success=success,
            output=output,
            elapsed_time=elapsed,
            asan_report=asan_report,
        )

    def _check_compilation_error(self, output: str) -> Optional[str]:
        """Check output for compilation errors.

        Args:
            output: Compiler output

        Returns:
            Error message if compilation failed, None otherwise
        """
        # Common GCC/Clang error patterns
        error_patterns = [
            r"error:\s+(.+)",
            r"undefined reference to\s+(.+)",
            r"fatal error:\s+(.+)",
            r"cannot find\s+-l(\w+)",
            r"No rule to make target",
            r"ld: .+ not found",
        ]

        for pattern in error_patterns:
            match = re.search(pattern, output, re.IGNORECASE)
            if match:
                return match.group(0)

        return None

    def _parse_test_output(self, output: str) -> tuple[int, int, int]:
        """Parse test output for results.

        Supports multiple output formats common in C test frameworks.

        Args:
            output: Test output text

        Returns:
            Tuple of (passed, failed, total)
        """
        passed = 0
        failed = 0
        total = 0

        # Format: "Results: X/Y tests passed"
        match = re.search(r"Results:\s*(\d+)/(\d+)\s*tests?\s*passed", output, re.IGNORECASE)
        if match:
            passed = int(match.group(1))
            total = int(match.group(2))
            failed = total - passed
            return passed, failed, total

        # Format: "X passed, Y failed"
        match = re.search(r"(\d+)\s+passed.*?(\d+)\s+failed", output, re.IGNORECASE)
        if match:
            passed = int(match.group(1))
            failed = int(match.group(2))
            total = passed + failed
            return passed, failed, total

        # Format: "Tests: X passed, Y failed, Z total"
        match = re.search(r"Tests?:\s*(\d+)\s+passed,\s*(\d+)\s+failed,\s*(\d+)\s+total", output, re.IGNORECASE)
        if match:
            passed = int(match.group(1))
            failed = int(match.group(2))
            total = int(match.group(3))
            return passed, failed, total

        # Count PASS/FAIL lines
        pass_count = len(re.findall(r"^\s*\[?PASS\]?\s", output, re.MULTILINE | re.IGNORECASE))
        fail_count = len(re.findall(r"^\s*\[?FAIL\]?\s", output, re.MULTILINE | re.IGNORECASE))

        if pass_count > 0 or fail_count > 0:
            passed = pass_count
            failed = fail_count
            total = passed + failed
            return passed, failed, total

        # Count OK/FAIL patterns (common in simple test frameworks)
        ok_count = len(re.findall(r"\.\.\.\s*OK", output, re.IGNORECASE))
        fail_count = len(re.findall(r"\.\.\.\s*FAIL", output, re.IGNORECASE))

        if ok_count > 0 or fail_count > 0:
            passed = ok_count
            failed = fail_count
            total = passed + failed
            return passed, failed, total

        # Check for simple assertion failures
        assertion_fails = len(re.findall(r"Assertion .+ failed", output, re.IGNORECASE))
        if assertion_fails > 0:
            failed = assertion_fails
            total = assertion_fails
            return 0, failed, total

        return passed, failed, total

    def run_with_asan_check(
        self,
        test_dir: Path,
        expected_asan_error: Optional[str] = None,
    ) -> JuliusTestResult:
        """Run tests specifically checking for ASan errors.

        This is useful for memory safety tasks where we expect
        the buggy code to trigger ASan and the fix to eliminate it.

        Args:
            test_dir: Test directory
            expected_asan_error: If set, expect this ASan error type in buggy code

        Returns:
            JuliusTestResult with ASan analysis
        """
        result = self.run(test_dir)

        # Annotate result with ASan-specific info
        if result.asan_report:
            if result.asan_report.has_errors:
                error_types = result.asan_report.get_error_types()
                result.output += f"\n\nASan detected: {[e.value for e in error_types]}"

                if expected_asan_error:
                    # Check if expected error was found
                    found = any(
                        expected_asan_error.lower() in e.value.lower()
                        for e in error_types
                    )
                    if found:
                        result.output += f"\n(Expected error '{expected_asan_error}' was found)"
                    else:
                        result.output += f"\n(Expected error '{expected_asan_error}' was NOT found)"

        return result


def convert_to_test_result(julius_result: JuliusTestResult):
    """Convert JuliusTestResult to standard TestResult for compatibility.

    Args:
        julius_result: JuliusTestResult from Julius test runner

    Returns:
        TestResult compatible with evaluation runner
    """
    from evaluation.test_runner import TestResult

    return TestResult(
        passed=julius_result.passed,
        failed=julius_result.failed,
        errors=julius_result.errors,
        skipped=0,
        total=julius_result.total,
        success=julius_result.success,
        output=julius_result.output,
        elapsed_time=julius_result.elapsed_time,
        test_cases=julius_result.test_cases,
    )
