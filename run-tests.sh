#!/usr/bin/env bash
# AirTower test wrapper: runs `pytest tests/` in the venv and writes .test-results.json.
# Always run the tests through this script, never pytest directly, so the results file
# AirTower reads stays fresh. Task validation is part of the suite
# (tests/scripts/test_validate_task.py validates every shipped task).
#
#   ./run-tests.sh                     the gate: whole suite, writes .test-results.json
#   ./run-tests.sh --filter <pattern>  only the tests pytest's -k <pattern> selects;
#                                      never writes or removes .test-results.json
#
# ANTHROPIC_API_KEY, OPENAI_API_KEY and AZURE_OPENAI_API_KEY (every key models/ reads)
# are unset for the whole wrapper, so no test can reach a paid API.
set -uo pipefail
unset ANTHROPIC_API_KEY OPENAI_API_KEY AZURE_OPENAI_API_KEY

cd "$(dirname "$0")" || exit 1

RESULTS_FILE=".test-results.json"
PYTHON="venv/bin/python"
SETUP_HINT="no venv/: set it up once with the CLAUDE.md § Build commands (python3 -m venv venv; venv/bin/pip install -r requirements.txt; venv/bin/pip install -e .)"

red() {
  printf '{\n  "passed": 0,\n  "failed": 1,\n  "total": 1,\n  "failures": [%s]\n}\n' \
    "$(printf '%s' "$1" | sed 's/\\/\\\\/g; s/"/\\"/g; s/^/"/; s/$/"/')" > "$RESULTS_FILE"
}

if [ "${1:-}" = "--filter" ]; then
  if [ -z "${2:-}" ] || [ "$#" -ne 2 ]; then
    echo "usage: ./run-tests.sh [--filter <pattern>]" >&2
    exit 2
  fi
  echo "Filtered run (--filter $2): $RESULTS_FILE is not written"
  if [ ! -x "$PYTHON" ]; then
    echo "run-tests.sh: $SETUP_HINT" >&2
    exit 1
  fi
  # pytest exits 5 when the pattern selects no test, so an empty filter run is red.
  exec "$PYTHON" -m pytest tests/ -k "$2"
elif [ "$#" -ne 0 ]; then
  echo "usage: ./run-tests.sh [--filter <pattern>]" >&2
  exit 2
fi

# A results file left over from an earlier run must never survive a run that died
# before writing its own: that would show a stale green.
rm -f "$RESULTS_FILE"

if [ ! -x "$PYTHON" ]; then
  echo "run-tests.sh: $SETUP_HINT" >&2
  red "run-tests.sh: $SETUP_HINT"
  exit 1
fi

REPORT_DIR="$(mktemp -d "${TMPDIR:-/tmp}/gdb-run-tests.XXXXXX")" || exit 1
trap 'rm -rf "$REPORT_DIR"' EXIT
REPORT="$REPORT_DIR/report.json"

"$PYTHON" -m pytest tests/ --json-report --json-report-file="$REPORT" -q
pytest_status=$?

"$PYTHON" -m scripts.airtower_gate results "$REPORT" "$pytest_status" "$RESULTS_FILE"
status=$?

if [ ! -f "$RESULTS_FILE" ]; then
  red "run-tests.sh: the results step exited $status before writing results (pytest exited $pytest_status)"
  [ "$status" -eq 0 ] && status=1
fi

exit "$status"
