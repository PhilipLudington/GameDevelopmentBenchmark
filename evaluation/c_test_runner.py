"""Test runner for C/Makefile-based tests."""

import os
import re
import shutil
import subprocess
import time
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, List, Optional

from harness.c_sandbox import CSandbox


@dataclass
class CTestResult:
    """Result of running C tests."""

    passed: int
    failed: int
    errors: int
    skipped: int
    total: int
    success: bool
    output: str
    elapsed_time: float = 0.0
    test_cases: Optional[List[Dict]] = None
    compilation_error: Optional[str] = None


class CTestRunner:
    """Runs Makefile-based C tests within a sandbox environment.

    Handles:
    - Copying test files to sandbox
    - Running make targets
    - Parsing test output for results
    - Handling compilation errors
    """

    def __init__(
        self,
        sandbox: CSandbox,
        timeout: int = 120,
    ):
        """Initialize the C test runner.

        Args:
            sandbox: CSandbox containing the game and test files
            timeout: Maximum test execution time in seconds
        """
        self.sandbox = sandbox
        self.timeout = timeout

    def run(self, test_dir: Optional[Path] = None, target: str = "test") -> CTestResult:
        """Run C tests in the sandbox.

        This method:
        1. Copies test files to the sandbox
        2. Runs `make <target>`
        3. Parses output for test results

        Args:
            test_dir: Path to test directory to copy (if not already in sandbox)
            target: Make target to run (default: "test")

        Returns:
            CTestResult with test outcomes
        """
        if not self.sandbox.working_dir:
            return CTestResult(
                passed=0,
                failed=0,
                errors=1,
                skipped=0,
                total=0,
                success=False,
                output="Sandbox not initialized",
            )

        # Copy tests if provided externally
        if test_dir and test_dir.exists():
            sandbox_tests = self.sandbox.working_dir / "tests"
            if sandbox_tests.exists():
                shutil.rmtree(sandbox_tests)
            shutil.copytree(test_dir, sandbox_tests)

        # Check tests directory exists
        tests_path = self.sandbox.working_dir / "tests"
        if not tests_path.exists():
            return CTestResult(
                passed=0,
                failed=0,
                errors=1,
                skipped=0,
                total=0,
                success=False,
                output=f"Test directory not found: {tests_path}",
            )

        # Run make
        start_time = time.time()

        try:
            result = self._run_make(tests_path, target)
            elapsed = time.time() - start_time

            return self._parse_results(result, elapsed)

        except subprocess.TimeoutExpired as e:
            elapsed = time.time() - start_time
            return CTestResult(
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
            return CTestResult(
                passed=0,
                failed=0,
                errors=1,
                skipped=0,
                total=0,
                success=False,
                output=f"Test execution error: {e}",
                elapsed_time=elapsed,
            )

    def _run_make(self, tests_path: Path, target: str) -> subprocess.CompletedProcess:
        """Run make command.

        Args:
            tests_path: Path to tests directory
            target: Make target

        Returns:
            CompletedProcess result
        """
        env = os.environ.copy()

        cmd = ["make", "-C", str(tests_path), target]

        return subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            timeout=self.timeout + 30,  # Extra time for compilation
            env=env,
        )

    def _parse_results(
        self,
        result: subprocess.CompletedProcess,
        elapsed: float,
    ) -> CTestResult:
        """Parse make/test output to extract test results.

        Args:
            result: Subprocess result from make
            elapsed: Elapsed time

        Returns:
            CTestResult with parsed outcomes
        """
        output = result.stdout + result.stderr

        # Check for compilation errors
        compilation_error = self._check_compilation_error(output)
        if compilation_error:
            return CTestResult(
                passed=0,
                failed=0,
                errors=1,
                skipped=0,
                total=0,
                success=False,
                output=output,
                elapsed_time=elapsed,
                compilation_error=compilation_error,
            )

        # Parse test results from output
        passed, failed, total = self._parse_test_output(output)

        # If we couldn't parse results, check return code
        if total == 0:
            if result.returncode == 0:
                # Assume success if make returned 0
                passed = 1
                total = 1
            else:
                # Assume failure if make returned non-zero
                failed = 1
                total = 1

        return CTestResult(
            passed=passed,
            failed=failed,
            errors=0,
            skipped=0,
            total=total,
            success=(failed == 0 and result.returncode == 0),
            output=output,
            elapsed_time=elapsed,
        )

    def _check_compilation_error(self, output: str) -> Optional[str]:
        """Check output for compilation errors.

        Args:
            output: Make/compiler output

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
        ]

        for pattern in error_patterns:
            match = re.search(pattern, output, re.IGNORECASE)
            if match:
                return match.group(0)

        return None

    def _parse_test_output(self, output: str) -> tuple[int, int, int]:
        """Parse test output for results.

        Supports multiple output formats:
        - "Results: X/Y tests passed"
        - "X passed, Y failed"
        - "PASS/FAIL" counting

        Args:
            output: Test output text

        Returns:
            Tuple of (passed, failed, total)
        """
        passed = 0
        failed = 0
        total = 0

        # Format: "Results: X/Y tests passed" (Quake test format)
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

        # Format: Count PASS/FAIL lines
        pass_count = len(re.findall(r"^\s*PASS\b", output, re.MULTILINE | re.IGNORECASE))
        fail_count = len(re.findall(r"^\s*FAIL\b", output, re.MULTILINE | re.IGNORECASE))

        if pass_count > 0 or fail_count > 0:
            passed = pass_count
            failed = fail_count
            total = passed + failed
            return passed, failed, total

        # Format: Count individual test results in verbose output
        pass_count = output.count("  PASS")
        fail_count = output.count("  FAIL")

        if pass_count > 0 or fail_count > 0:
            passed = pass_count
            failed = fail_count
            total = passed + failed
            return passed, failed, total

        return passed, failed, total

    def run_comparison(self) -> Dict[str, CTestResult]:
        """Run both game and solution tests for comparison.

        Returns:
            Dictionary with "game" and "solution" test results
        """
        results = {}

        # Run game (incomplete) version
        game_result = self.run(target="test")
        results["game"] = game_result

        # Run solution (complete) version
        solution_result = self.run(target="test_solution")
        results["solution"] = solution_result

        return results


def convert_to_test_result(c_result: CTestResult):
    """Convert CTestResult to standard TestResult for compatibility.

    Args:
        c_result: CTestResult from C test runner

    Returns:
        TestResult compatible with evaluation runner
    """
    from evaluation.test_runner import TestResult

    return TestResult(
        passed=c_result.passed,
        failed=c_result.failed,
        errors=c_result.errors,
        skipped=c_result.skipped,
        total=c_result.total,
        success=c_result.success,
        output=c_result.output,
        elapsed_time=c_result.elapsed_time,
        test_cases=c_result.test_cases,
    )
