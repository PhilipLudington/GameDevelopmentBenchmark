"""Tests for the AirTower gate wrappers (`run-tests.sh`, `run-build.sh`) and
`scripts/airtower_gate.py`, against `~/.claude/docs/airtower.md`'s results formats.

The wrapper tests copy only the wrapper script into a temporary folder, with a stand-in
`venv/bin/python` that records its environment and then runs this venv's Python, and a
tiny `tests/` suite, so the real suite is never run from inside itself.
"""

import json
import os
import shutil
import stat
import subprocess
import sys
from pathlib import Path

import pytest

from scripts.airtower_gate import build_check, count_test_results

REPO_ROOT = Path(__file__).resolve().parents[2]


# --- counting a pytest report ------------------------------------------------


def _report(*outcomes, collectors=()):
    return {
        "tests": [{"nodeid": f"t::{i}", "outcome": o} for i, o in enumerate(outcomes)],
        "collectors": [{"nodeid": n, "outcome": "failed"} for n in collectors],
    }


def test_skipped_and_xfailed_tests_are_neither_passed_nor_failed():
    results = count_test_results(_report("passed", "skipped", "xfailed", "xpassed"), 0)
    assert results == {"passed": 2, "failed": 0, "total": 2, "failures": []}


def test_failed_and_errored_tests_are_failures():
    results = count_test_results(_report("passed", "failed", "error"), 1)
    assert (results["passed"], results["failed"], results["total"]) == (1, 2, 3)
    assert results["failures"] == ["t::1", "t::2"]


def test_a_module_that_fails_to_collect_is_a_failure():
    results = count_test_results(_report("passed", collectors=["tests/test_x.py"]), 2)
    assert results["failed"] >= 1
    assert "collection error: tests/test_x.py" in results["failures"]


def test_a_run_with_no_test_is_red():
    results = count_test_results(_report("skipped"), 0)
    assert results["passed"] == 0
    assert results["failed"] == 1
    assert results["failures"] == ["no tests ran under tests/ (none collected, or all skipped)"]


def test_a_pytest_exit_other_than_0_or_1_is_red():
    results = count_test_results(_report("passed"), 2)
    assert results["failed"] == 1
    assert results["failures"] == ["pytest exited 2"]


# --- run-tests.sh -------------------------------------------------------------


def _fake_venv(root: Path) -> Path:
    bin_dir = root / "venv" / "bin"
    bin_dir.mkdir(parents=True)
    fake = bin_dir / "python"
    fake.write_text(
        "#!/bin/sh\n"
        'env >> "$ENV_DUMP"\n'
        f'PYTHONPATH="{REPO_ROOT}" exec "{sys.executable}" "$@"\n'
    )
    fake.chmod(fake.stat().st_mode | stat.S_IXUSR)
    return fake


def _wrapper_tree(tmp_path: Path, test_source: str, with_venv: bool = True) -> Path:
    shutil.copy(REPO_ROOT / "run-tests.sh", tmp_path / "run-tests.sh")
    (tmp_path / "tests").mkdir()
    (tmp_path / "tests" / "test_sample.py").write_text(test_source)
    if with_venv:
        _fake_venv(tmp_path)
    return tmp_path


def _run(tree: Path, *args: str):
    env = dict(os.environ)
    env["ENV_DUMP"] = str(tree / "env-dump.txt")
    env["ANTHROPIC_API_KEY"] = "sk-test-anthropic"
    env["OPENAI_API_KEY"] = "sk-test-openai"
    env["AZURE_OPENAI_API_KEY"] = "sk-test-azure"
    return subprocess.run(
        ["bash", str(tree / "run-tests.sh"), *args],
        env=env, capture_output=True, text=True, timeout=120,
    )


PASS_AND_SKIP = (
    "import pytest\n"
    "def test_ok():\n    assert True\n"
    "@pytest.mark.skip\ndef test_skipped():\n    pass\n"
)
ONE_FAILS = "def test_ok():\n    assert True\ndef test_bad():\n    assert False\n"


def test_gate_run_writes_the_results_file_and_exits_0(tmp_path):
    tree = _wrapper_tree(tmp_path, PASS_AND_SKIP)
    proc = _run(tree)
    assert proc.returncode == 0, proc.stdout + proc.stderr
    results = json.loads((tree / ".test-results.json").read_text())
    assert results == {"passed": 1, "failed": 0, "total": 1, "failures": []}


def test_a_failing_test_makes_the_gate_exit_non_zero(tmp_path):
    tree = _wrapper_tree(tmp_path, ONE_FAILS)
    proc = _run(tree)
    assert proc.returncode == 1
    results = json.loads((tree / ".test-results.json").read_text())
    assert (results["passed"], results["failed"], results["total"]) == (1, 1, 2)
    assert results["failures"] == ["tests/test_sample.py::test_bad"]


def test_the_test_run_never_sees_the_api_keys(tmp_path):
    tree = _wrapper_tree(tmp_path, PASS_AND_SKIP)
    assert _run(tree).returncode == 0
    dump = (tree / "env-dump.txt").read_text()
    assert "sk-test-anthropic" not in dump
    assert "sk-test-openai" not in dump
    assert "sk-test-azure" not in dump


def test_a_filtered_run_never_writes_the_results_file(tmp_path):
    tree = _wrapper_tree(tmp_path, ONE_FAILS)
    (tree / ".test-results.json").write_text("left alone\n")
    proc = _run(tree, "--filter", "test_ok")
    assert proc.returncode == 0, proc.stdout + proc.stderr
    assert proc.stdout.splitlines()[0] == (
        "Filtered run (--filter test_ok): .test-results.json is not written"
    )
    assert (tree / ".test-results.json").read_text() == "left alone\n"
    assert "sk-test-anthropic" not in (tree / "env-dump.txt").read_text()


def test_no_venv_writes_a_red_results_file_naming_the_setup(tmp_path):
    tree = _wrapper_tree(tmp_path, PASS_AND_SKIP, with_venv=False)
    proc = _run(tree)
    assert proc.returncode == 1
    results = json.loads((tree / ".test-results.json").read_text())
    assert results["failed"] == 1
    assert "CLAUDE.md § Build" in results["failures"][0]


# --- run-build.sh -------------------------------------------------------------


def test_build_check_passes_on_this_repo():
    results = build_check(REPO_ROOT, sys.executable)
    assert set(results) == {"success", "errors", "warnings", "messages"}
    assert results["success"] is True, results["messages"]
    assert results["errors"] == 0


def test_a_file_that_does_not_compile_is_a_build_error(tmp_path):
    (tmp_path / "evaluation").mkdir()
    (tmp_path / "evaluation" / "broken.py").write_text("def x(:\n")
    results = build_check(tmp_path, sys.executable)
    assert results["success"] is False
    assert results["errors"] >= 1
    assert any(m.startswith("compile evaluation:") for m in results["messages"])


def test_run_build_with_no_venv_writes_a_red_results_file(tmp_path):
    shutil.copy(REPO_ROOT / "run-build.sh", tmp_path / "run-build.sh")
    proc = subprocess.run(["bash", str(tmp_path / "run-build.sh")], capture_output=True, text=True)
    assert proc.returncode == 1
    results = json.loads((tmp_path / ".build-results.json").read_text())
    assert results["success"] is False
    assert "CLAUDE.md § Build" in results["messages"][0]
