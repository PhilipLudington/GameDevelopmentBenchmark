#!/bin/bash
# Test all Quake tasks
# Usage: ./scripts/test_quake_tasks.sh

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

echo "=============================================="
echo "Testing All Quake Tasks (Solution Versions)"
echo "=============================================="
echo ""

TOTAL=0
PASSED=0

# Use find to get absolute paths
while IFS= read -r test_dir; do
    task_name=$(basename "$(dirname "$test_dir")")
    ((TOTAL++))

    echo -n "$task_name: "

    # Run in subshell to avoid directory issues
    result=$(
        cd "$test_dir" || exit 1
        make clean >/dev/null 2>&1

        # Try test_solution first, fall back to test
        if grep -q "test_solution" Makefile 2>/dev/null; then
            make test_solution 2>&1
        else
            make test 2>&1
        fi
    )
    exit_code=$?

    # Check for success
    if [ $exit_code -eq 0 ] && echo "$result" | grep -qiE "(passed|PASS)"; then
        # Extract results line
        summary=$(echo "$result" | grep -iE "([0-9]+ passed|Results:)" | tail -1)
        echo "PASS - $summary"
        ((PASSED++))
    else
        echo "FAIL"
        # Show last few lines on failure
        echo "$result" | tail -3 | sed 's/^/    /'
    fi
    echo ""
done < <(find "$PROJECT_DIR/tasks/quake" -path "*/tests/Makefile" -exec dirname {} \; | sort)

echo "=============================================="
echo "SUMMARY: $PASSED / $TOTAL tasks passing"
echo "=============================================="

[ $PASSED -eq $TOTAL ]
