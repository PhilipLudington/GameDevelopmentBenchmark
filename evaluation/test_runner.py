"""Test runner for executing pytest in sandbox environment."""

import json
import os
import re
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path

from harness.sandbox import Sandbox


@dataclass
class TestResult:
    """Result of running tests."""

    passed: int
    failed: int
    errors: int
    skipped: int
    total: int
    success: bool
    output: str
    elapsed_time: float = 0.0
    test_cases: list[dict] | None = None


class TestRunner:
    """Runs pytest tests within a sandbox environment.

    Handles:
    - Setting up Python path for game imports
    - Running pytest with timeout
    - Parsing test results
    - Capturing output
    """

    def __init__(
        self,
        sandbox: Sandbox,
        timeout: int = 60,
        python_executable: str | None = None,
    ):
        """Initialize the test runner.

        Args:
            sandbox: Sandbox containing the game and test files
            timeout: Maximum test execution time in seconds
            python_executable: Python executable to use
        """
        self.sandbox = sandbox
        self.timeout = timeout
        self.python_executable = python_executable or sys.executable

    def run(self, test_path: str | None = None) -> TestResult:
        """Run tests in the sandbox.

        Args:
            test_path: Optional specific test path (relative to sandbox root)

        Returns:
            TestResult with test outcomes
        """
        if not self.sandbox.working_dir:
            return TestResult(
                passed=0,
                failed=0,
                errors=1,
                skipped=0,
                total=0,
                success=False,
                output="Sandbox not initialized",
            )

        # Determine test directory
        if test_path:
            tests_dir = self.sandbox.working_dir / test_path
        else:
            tests_dir = self.sandbox.working_dir / "tests"

        if not tests_dir.exists():
            return TestResult(
                passed=0,
                failed=0,
                errors=1,
                skipped=0,
                total=0,
                success=False,
                output=f"Test directory not found: {tests_dir}",
            )

        # Set up environment
        env = os.environ.copy()
        env["SDL_VIDEODRIVER"] = "dummy"
        env["SDL_AUDIODRIVER"] = "dummy"

        # Add game directory to Python path
        game_dir = self.sandbox.game_dir
        if game_dir:
            python_path = env.get("PYTHONPATH", "")
            env["PYTHONPATH"] = f"{game_dir}:{python_path}" if python_path else str(game_dir)

        # Build pytest command
        cmd = [
            self.python_executable,
            "-m", "pytest",
            str(tests_dir),
            "-v",
            "--tb=short",
            f"--timeout={self.timeout}",
            "--json-report",
            "--json-report-file=.pytest_results.json",
        ]

        import time
        start_time = time.time()

        try:
            result = subprocess.run(
                cmd,
                cwd=str(self.sandbox.working_dir),
                env=env,
                capture_output=True,
                text=True,
                timeout=self.timeout + 30,
            )
            elapsed = time.time() - start_time

            # Parse results
            return self._parse_results(result, elapsed)

        except subprocess.TimeoutExpired as e:
            elapsed = time.time() - start_time
            return TestResult(
                passed=0,
                failed=0,
                errors=1,
                skipped=0,
                total=0,
                success=False,
                output=f"Test timeout after {self.timeout}s\n{e.stdout or ''}\n{e.stderr or ''}",
                elapsed_time=elapsed,
            )

        except Exception as e:
            elapsed = time.time() - start_time
            return TestResult(
                passed=0,
                failed=0,
                errors=1,
                skipped=0,
                total=0,
                success=False,
                output=f"Test execution error: {e}",
                elapsed_time=elapsed,
            )

    def _parse_results(self, result: subprocess.CompletedProcess, elapsed: float) -> TestResult:
        """Parse pytest output to extract test results.

        Args:
            result: Subprocess result from pytest
            elapsed: Elapsed time

        Returns:
            TestResult with parsed outcomes
        """
        output = result.stdout + result.stderr

        # Try to load JSON report if available
        json_report_path = self.sandbox.working_dir / ".pytest_results.json" if self.sandbox.working_dir else None
        test_cases = None

        if json_report_path and json_report_path.exists():
            try:
                with open(json_report_path) as f:
                    report = json.load(f)

                summary = report.get("summary", {})
                passed = summary.get("passed", 0)
                failed = summary.get("failed", 0)
                errors = summary.get("error", 0)
                skipped = summary.get("skipped", 0)
                total = summary.get("total", passed + failed + errors + skipped)

                # Extract individual test cases
                tests = report.get("tests", [])
                test_cases = [
                    {
                        "name": t.get("nodeid", ""),
                        "outcome": t.get("outcome", ""),
                        "duration": t.get("duration", 0),
                    }
                    for t in tests
                ]

                return TestResult(
                    passed=passed,
                    failed=failed,
                    errors=errors,
                    skipped=skipped,
                    total=total,
                    success=result.returncode == 0,
                    output=output,
                    elapsed_time=elapsed,
                    test_cases=test_cases,
                )

            except (json.JSONDecodeError, KeyError):
                pass

        # Fallback: parse pytest output
        return self._parse_pytest_output(output, result.returncode, elapsed)

    def _parse_pytest_output(self, output: str, return_code: int, elapsed: float) -> TestResult:
        """Parse pytest text output as fallback.

        Args:
            output: pytest stdout/stderr
            return_code: pytest exit code
            elapsed: Elapsed time

        Returns:
            TestResult with parsed outcomes
        """
        passed = 0
        failed = 0
        errors = 0
        skipped = 0

        # Look for summary line like "1 passed, 2 failed, 1 error in 0.5s"
        summary_pattern = r"(\d+)\s+passed|(\d+)\s+failed|(\d+)\s+error|(\d+)\s+skipped"
        matches = re.findall(summary_pattern, output)

        for match in matches:
            if match[0]:
                passed = int(match[0])
            if match[1]:
                failed = int(match[1])
            if match[2]:
                errors = int(match[2])
            if match[3]:
                skipped = int(match[3])

        # Alternative: count PASSED/FAILED in verbose output
        if passed == 0 and failed == 0:
            passed = output.count(" PASSED")
            failed = output.count(" FAILED")
            errors = output.count(" ERROR")
            skipped = output.count(" SKIPPED")

        total = passed + failed + errors + skipped

        return TestResult(
            passed=passed,
            failed=failed,
            errors=errors,
            skipped=skipped,
            total=total,
            success=return_code == 0,
            output=output,
            elapsed_time=elapsed,
        )

    def run_specific_tests(self, test_ids: list[str]) -> TestResult:
        """Run specific tests by their IDs.

        Args:
            test_ids: List of test IDs (e.g., ["test_module.py::test_function"])

        Returns:
            TestResult with outcomes
        """
        if not self.sandbox.working_dir:
            return TestResult(
                passed=0,
                failed=0,
                errors=1,
                skipped=0,
                total=0,
                success=False,
                output="Sandbox not initialized",
            )

        env = os.environ.copy()
        env["SDL_VIDEODRIVER"] = "dummy"
        env["SDL_AUDIODRIVER"] = "dummy"

        game_dir = self.sandbox.game_dir
        if game_dir:
            python_path = env.get("PYTHONPATH", "")
            env["PYTHONPATH"] = f"{game_dir}:{python_path}" if python_path else str(game_dir)

        cmd = [
            self.python_executable,
            "-m", "pytest",
            "-v",
            "--tb=short",
            f"--timeout={self.timeout}",
        ] + test_ids

        import time
        start_time = time.time()

        try:
            result = subprocess.run(
                cmd,
                cwd=str(self.sandbox.working_dir),
                env=env,
                capture_output=True,
                text=True,
                timeout=self.timeout + 30,
            )
            elapsed = time.time() - start_time
            return self._parse_pytest_output(result.stdout + result.stderr, result.returncode, elapsed)

        except subprocess.TimeoutExpired:
            elapsed = time.time() - start_time
            return TestResult(
                passed=0,
                failed=0,
                errors=1,
                skipped=0,
                total=len(test_ids),
                success=False,
                output=f"Test timeout after {self.timeout}s",
                elapsed_time=elapsed,
            )

        except Exception as e:
            elapsed = time.time() - start_time
            return TestResult(
                passed=0,
                failed=0,
                errors=1,
                skipped=0,
                total=len(test_ids),
                success=False,
                output=str(e),
                elapsed_time=elapsed,
            )
