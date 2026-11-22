"""The results files behind the AirTower gate wrappers, `run-tests.sh` and `run-build.sh`.

`results` turns a pytest-json-report file into `.test-results.json`; `build` checks the
installed code (byte-compile, import, `pip check`) and writes `.build-results.json`.
Both formats are `~/.claude/docs/airtower.md` § Test / Build Status Integration. The
wrappers call this module; run them, not it.
"""

from __future__ import annotations

import compileall
import contextlib
import importlib
import io
import json
import pkgutil
import subprocess
import sys
import traceback
import warnings
from pathlib import Path

# Packages every module of which is imported by the build check.
IMPORTED_PACKAGES = ("evaluation", "harness", "models", "scripts")
# Packages only byte-compiled: the baseline games start pygame when imported.
COMPILED_ONLY = ("baselines",)

PASSED_OUTCOMES = {"passed", "xpassed"}
FAILED_OUTCOMES = {"failed", "error"}


def count_test_results(report: dict, pytest_exit: int) -> dict:
    """Count one outcome per test from a pytest-json-report dict.

    A skipped or xfailed test is neither passed nor failed. A module that fails to
    collect is one failure. A run that ran no test, or that pytest ended with an exit
    code other than 0 (all passed) or 1 (some failed), is red.
    """
    passed = 0
    failed = 0
    failures: list[str] = []
    for test in report.get("tests", []):
        outcome = test.get("outcome")
        if outcome in PASSED_OUTCOMES:
            passed += 1
        elif outcome in FAILED_OUTCOMES:
            failed += 1
            failures.append(test.get("nodeid", "<unnamed test>"))
    for collector in report.get("collectors", []):
        if collector.get("outcome") == "failed":
            failed += 1
            failures.append(f"collection error: {collector.get('nodeid') or '<root>'}")

    if passed + failed == 0:
        failures.append("no tests ran under tests/ (none collected, or all skipped)")
        failed += 1
    elif pytest_exit not in (0, 1):
        failures.append(f"pytest exited {pytest_exit}")
        failed += 1
    elif pytest_exit == 1 and failed == 0:
        failures.append("pytest exited 1 with no failed test in its report")
        failed += 1

    return {"passed": passed, "failed": failed, "total": passed + failed, "failures": failures}


def build_check(root: Path, python: str) -> dict:
    """Byte-compile the packaged code, import every importable module, run `pip check`."""
    errors: list[str] = []
    warning_messages: list[str] = []

    def record(caught: list[warnings.WarningMessage]) -> None:
        for w in caught:
            warning_messages.append(f"{w.filename}:{w.lineno}: {w.category.__name__}: {w.message}")

    for package in IMPORTED_PACKAGES + COMPILED_ONLY:
        log = io.StringIO()
        with warnings.catch_warnings(record=True) as caught, contextlib.redirect_stdout(log):
            warnings.simplefilter("always")
            ok = compileall.compile_dir(str(root / package), force=True, quiet=1)
        record(caught)
        if not ok:
            lines = [line.strip() for line in log.getvalue().splitlines() if line.strip()]
            errors.append(f"compile {package}: " + (" | ".join(lines) or "compileall failed"))

    if not errors:
        sys.path.insert(0, str(root))
        for package in IMPORTED_PACKAGES:
            with warnings.catch_warnings(record=True) as caught:
                warnings.simplefilter("always")
                try:
                    module = importlib.import_module(package)
                    names = [
                        info.name
                        for info in pkgutil.walk_packages(module.__path__, prefix=f"{package}.")
                        if info.name.rpartition(".")[2] != "__main__"
                    ]
                except Exception:
                    errors.append(f"import {package}: {traceback.format_exc(limit=1).strip()}")
                    names = []
            record(caught)
            for name in names:
                with warnings.catch_warnings(record=True) as caught:
                    warnings.simplefilter("always")
                    try:
                        importlib.import_module(name)
                    except Exception as exc:
                        errors.append(f"import {name}: {type(exc).__name__}: {exc}")
                record(caught)

    check = subprocess.run([python, "-m", "pip", "check"], capture_output=True, text=True)
    if check.returncode != 0:
        lines = [line for line in (check.stdout + check.stderr).splitlines() if line.strip()]
        errors.append("pip check: " + (" | ".join(lines) or f"exited {check.returncode}"))

    return {
        "success": not errors,
        "errors": len(errors),
        "warnings": len(warning_messages),
        "messages": errors + warning_messages,
    }


def _write(path: Path, data: dict) -> None:
    path.write_text(json.dumps(data, indent=2) + "\n", encoding="utf-8")


def main(argv: list[str]) -> int:
    usage = "usage: airtower_gate.py results <report.json> <pytest exit> <out> | build <root> <out>"
    if len(argv) == 4 and argv[0] == "results":
        report_path, exit_text, out = Path(argv[1]), argv[2], Path(argv[3])
        report = json.loads(report_path.read_text()) if report_path.is_file() else {}
        results = count_test_results(report, int(exit_text))
        _write(out, results)
        for name in results["failures"]:
            print(f"FAILED {name}")
        print(f"tests: {results['passed']} passed, {results['failed']} failed")
        return 1 if results["failures"] else 0
    if len(argv) == 3 and argv[0] == "build":
        results = build_check(Path(argv[1]), sys.executable)
        _write(Path(argv[2]), results)
        for message in results["messages"]:
            print(message)
        print(f"build: {results['errors']} error(s), {results['warnings']} warning(s)")
        return 0 if results["success"] else 1
    print(usage, file=sys.stderr)
    return 2


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
