#!/usr/bin/env python3
"""Validate that Julius benchmark tests properly discriminate buggy vs fixed code.

A valid test must:
1. PASS on fixed code (original commit)
2. FAIL on buggy code (after applying buggy.patch)

This script validates julius-002 and julius-006 tests.
"""

import json
import sys
from pathlib import Path

# Add project root to path
sys.path.insert(0, str(Path(__file__).parent))

from evaluation.julius_test_runner import JuliusTestRunner
from harness.julius_sandbox import JuliusSandbox, JuliusSandboxConfig


def validate_task(task_id: str, task_dir: Path) -> dict:
    """Validate a single task's tests.

    Returns dict with results for both fixed and buggy states.
    """
    print(f"\n{'='*60}")
    print(f"Validating {task_id}")
    print(f"{'='*60}")

    # Load task config
    task_json = task_dir / "task.json"
    if not task_json.exists():
        return {"error": f"task.json not found in {task_dir}"}

    with open(task_json) as f:
        task_config = json.load(f)

    commit = task_config.get("commit")
    if not commit:
        return {"error": "No commit specified in task.json"}

    test_dir = task_dir / "tests"
    if not test_dir.exists():
        return {"error": f"tests directory not found in {task_dir}"}

    buggy_patch = task_dir / "buggy.patch"
    if not buggy_patch.exists():
        return {"error": f"buggy.patch not found in {task_dir}"}

    results = {
        "task_id": task_id,
        "commit": commit,
        "fixed": None,
        "buggy": None,
        "valid": False,
    }

    # Test 1: Fixed code should PASS
    print(f"\n--- Testing FIXED code (commit {commit}) ---")
    config = JuliusSandboxConfig(enable_asan=True)

    with JuliusSandbox(config) as sandbox:
        clone_result = sandbox.clone(commit=commit)
        if not clone_result.success:
            results["fixed"] = {"error": f"Clone failed: {clone_result.error}"}
            return results

        print(f"  Cloned Julius to {sandbox.repo_dir}")

        runner = JuliusTestRunner(sandbox, timeout=120)
        fixed_result = runner.run(test_dir)

        results["fixed"] = {
            "success": fixed_result.success,
            "passed": fixed_result.passed,
            "failed": fixed_result.failed,
            "total": fixed_result.total,
            "compilation_error": fixed_result.compilation_error,
            "asan_errors": fixed_result.asan_report.has_errors if fixed_result.asan_report else False,
        }

        if fixed_result.success:
            print(f"  ✅ FIXED code: PASSED ({fixed_result.passed}/{fixed_result.total} tests)")
        else:
            print(f"  ❌ FIXED code: FAILED")
            if fixed_result.compilation_error:
                print(f"     Compilation error: {fixed_result.compilation_error}")
            if fixed_result.asan_report and fixed_result.asan_report.has_errors:
                print(f"     ASan errors detected")
            print(f"     Output (last 500 chars):\n{fixed_result.output[-500:]}")

    # Test 2: Buggy code should FAIL
    print(f"\n--- Testing BUGGY code (with buggy.patch applied) ---")

    with JuliusSandbox(config) as sandbox:
        clone_result = sandbox.clone(commit=commit)
        if not clone_result.success:
            results["buggy"] = {"error": f"Clone failed: {clone_result.error}"}
            return results

        # Apply buggy patch
        patch_result = sandbox.apply_buggy_patch(buggy_patch)
        if not patch_result.success:
            results["buggy"] = {"error": f"Patch failed: {patch_result.error}"}
            print(f"  ❌ Failed to apply buggy.patch: {patch_result.error}")
            return results

        print(f"  Applied buggy.patch")

        runner = JuliusTestRunner(sandbox, timeout=120)
        buggy_result = runner.run(test_dir)

        results["buggy"] = {
            "success": buggy_result.success,
            "passed": buggy_result.passed,
            "failed": buggy_result.failed,
            "total": buggy_result.total,
            "compilation_error": buggy_result.compilation_error,
            "asan_errors": buggy_result.asan_report.has_errors if buggy_result.asan_report else False,
        }

        if not buggy_result.success:
            reason = []
            if buggy_result.failed > 0:
                reason.append(f"{buggy_result.failed} tests failed")
            if buggy_result.asan_report and buggy_result.asan_report.has_errors:
                error_types = buggy_result.asan_report.get_error_types()
                reason.append(f"ASan: {[e.value for e in error_types]}")
            if buggy_result.compilation_error:
                reason.append(f"Compile error: {buggy_result.compilation_error}")
            print(f"  ✅ BUGGY code: FAILED as expected ({', '.join(reason)})")
        else:
            print(f"  ❌ BUGGY code: PASSED (should have FAILED!)")
            print(f"     This means the test does NOT detect the bug!")
            print(f"     Output (last 500 chars):\n{buggy_result.output[-500:]}")

    # Determine if task is valid
    fixed_ok = results["fixed"] and results["fixed"].get("success", False)
    buggy_fails = results["buggy"] and not results["buggy"].get("success", True)

    results["valid"] = fixed_ok and buggy_fails

    print(f"\n--- Validation Result ---")
    if results["valid"]:
        print(f"  ✅ {task_id}: VALID (fixed passes, buggy fails)")
    else:
        print(f"  ❌ {task_id}: INVALID")
        if not fixed_ok:
            print(f"     - Fixed code should PASS but didn't")
        if not buggy_fails:
            print(f"     - Buggy code should FAIL but didn't")

    return results


def main():
    # Task directories
    tasks = {
        "julius-002": Path("tasks/julius/memory-safety/julius-002"),
        "julius-006": Path("tasks/julius/game-logic/julius-006"),
    }

    # Optionally test working tasks for comparison
    if "--all" in sys.argv:
        tasks.update({
            "julius-001": Path("tasks/julius/memory-safety/julius-001"),
            "julius-003": Path("tasks/julius/memory-safety/julius-003"),
            "julius-004": Path("tasks/julius/visual/julius-004"),
            "julius-005": Path("tasks/julius/game-logic/julius-005"),
            # Synthetic tasks (may have linking issues)
            "julius-007": Path("tasks/julius/memory-safety/julius-007"),
            "julius-008": Path("tasks/julius/memory-safety/julius-008"),
            "julius-009": Path("tasks/julius/memory-safety/julius-009"),
            "julius-010": Path("tasks/julius/memory-safety/julius-010"),
        })

    print("=" * 60)
    print("Julius Benchmark Test Validation")
    print("=" * 60)
    print(f"Tasks to validate: {list(tasks.keys())}")

    all_results = {}
    for task_id, task_dir in tasks.items():
        if not task_dir.exists():
            print(f"\n⚠️  Skipping {task_id}: directory not found")
            continue

        results = validate_task(task_id, task_dir)
        all_results[task_id] = results

    # Summary
    print("\n" + "=" * 60)
    print("VALIDATION SUMMARY")
    print("=" * 60)

    valid_count = 0
    invalid_count = 0

    for task_id, results in all_results.items():
        if results.get("valid"):
            print(f"  ✅ {task_id}: Valid")
            valid_count += 1
        elif results.get("error"):
            print(f"  ⚠️  {task_id}: Error - {results['error']}")
            invalid_count += 1
        else:
            print(f"  ❌ {task_id}: Invalid")
            invalid_count += 1

    print(f"\nTotal: {valid_count} valid, {invalid_count} invalid/error")

    return 0 if invalid_count == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
