# Project Bugs

> **Written for Claude.** Precise for a later session, not easy reading. To review it, ask Claude to explain it in plain words and judge the explanation.

## [ ] Bug 1: Task validation rejects 99 of the 225 shipped tasks

**Status:** Open

**System:** task validation — `scripts/validate_task.py`, `schemas/task_schema.json`, the tasks' `task.json` and `solution/` folders

**Description:** `scripts/validate_task.py` fails 99 shipped tasks, so the Phase 5 test gate ("`pytest tests/` plus task validation") cannot be green. The checks and the tasks disagree in four ways:
- All 50 Julius tasks: no `game/` folder and no `.c`/`.h` file in `solution/`. They ship as patches (`buggy.patch` or `solution/fix.patch`), which `models/cli_model.py` `mock:solution` reads.
- 34 Julius tasks have `"expected_asan_error": null`, where the schema wants a string; 10 have `"commit": "synthetic"`, where the schema wants a hex hash.
- 40 Pygame tasks (Space Invaders, Tower Defense) have an `id` with an underscore, which the schema's `^[a-z0-9-]+$` rejects.
- 9 Pong tasks (`pong-002` to `pong-010`) have an empty `solution/` folder.

Which side is wrong (the validator and schema, or the tasks) is not yet decided per case.

**Steps to reproduce:**
1. `for t in tasks/*/*/*/; do venv/bin/python scripts/validate_task.py "$t" --quiet >/dev/null 2>&1 || echo "$t"; done | wc -l`

**Expected:** 0 (every shipped task validates).

**Actual:** 99 (2026-11-18, at `a490ae5`).

**Verified by:** the steps to reproduce above — answer key: **Expected** (`rules/plan-format.md` § Verified by)

**Found by:** writing `CLAUDE.md` (Phase 5), 2026-11-18, while running its validation loop as written.
