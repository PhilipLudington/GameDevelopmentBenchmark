#!/usr/bin/env bash
# AirTower build wrapper: a read-only health check of the venv, never its setup. It
# byte-compiles the packaged code, imports every module of evaluation, harness, models
# and scripts (baselines are only compiled), runs `pip check`, and writes
# .build-results.json. Always build through this script so the results file AirTower
# reads stays fresh. Set the venv up once with the CLAUDE.md § Build commands.
set -uo pipefail

cd "$(dirname "$0")" || exit 1

RESULTS_FILE=".build-results.json"
PYTHON="venv/bin/python"
SETUP_HINT="no venv/: set it up once with the CLAUDE.md § Build commands (python3 -m venv venv; venv/bin/pip install -r requirements.txt; venv/bin/pip install -e .)"

red() {
  printf '{\n  "success": false,\n  "errors": 1,\n  "warnings": 0,\n  "messages": [%s]\n}\n' \
    "$(printf '%s' "$1" | sed 's/\\/\\\\/g; s/"/\\"/g; s/^/"/; s/$/"/')" > "$RESULTS_FILE"
}

if [ "$#" -ne 0 ]; then
  echo "usage: ./run-build.sh" >&2
  exit 2
fi

# A results file left over from an earlier run must never survive a run that died
# before writing its own: that would show a stale green.
rm -f "$RESULTS_FILE"

if [ ! -x "$PYTHON" ]; then
  echo "run-build.sh: $SETUP_HINT" >&2
  red "run-build.sh: $SETUP_HINT"
  exit 1
fi

"$PYTHON" -m scripts.airtower_gate build . "$RESULTS_FILE"
status=$?

if [ ! -f "$RESULTS_FILE" ]; then
  red "run-build.sh: the build check exited $status before writing results"
  [ "$status" -eq 0 ] && status=1
fi

exit "$status"
