# The task rules are the truth; shipped tasks change to fit them (2026-11-22)

**Decided by:** Philip, 2026-11-22 · **Status:** Decided
**Led to:** Bug 1 (task validation), PR 7; `DESIGN.md` § Task Validation Rules

## What prompted it
Bug 1: `scripts/validate_task.py` fails 99 of the 225 shipped tasks, so the Phase 5 test gate, which runs task validation, cannot be green. Later batches of generated tasks drifted from the January 2026 rules in `schemas/task_schema.json`:
- 40 Pygame tasks (Space Invaders, Tower Defense) have ids with `_`.
- 34 Julius tasks set `expected_asan_error` to `null`.
- Julius tasks use the word `"synthetic"` as their `commit`.
- All 50 Julius tasks ship patch files instead of a `game/` folder, a layout the validator does not model.

Bug 1 left open which side was wrong. The plan review that took up the fix (PR 7) found seven detailed questions underneath three missing rules: which side is the truth, how strict the validator should be for each engine, and whether the fix may change content and published history.

## What we weighed
- **The tasks are right; change the rules to fit** (the review's recommendation): no task file or published score changes. It lost, because it makes the drift the standard.
- **The rules are right; change the tasks to fit**: won. Philip: *""Rules are right" sounds like the correct choice, but we should have some way of linking the old names to the new ones, right?"*
- **How renamed ids link to past scores:** a map file of old and new ids that the tools read, so published history is never edited; or rewriting the 200 old ids in `results/run-log.json` and the raw run folders. The rewrite won after weighing the cost: five runs from 2026-02-14 change only their task ids, not their scores, the leaderboard is rebuilt from the log, and git keeps the old copy.
- **Validator strictness:** only reach 0 failures, or check each engine's real layout and test that broken tasks still fail. The layout check won, because a gate that passes everything proves nothing.
- **Writing the 9 missing Pong reference solutions** in this fix, as a separate item, or accepting the gap. A separate item won, so that the new game code gets its own review.

## What we decided, and why
When a shipped task and the rules disagree, the rules are right and the task changes. Renamed ids are renamed everywhere, including the published run log and raw run folders, with no map kept. A task's id must equal its folder name, which also renames 9 Snake tasks. Optional fields are left out instead of set to `null`. The one exception is `commit`, which accepts a hex hash or exactly `"synthetic"`, because `mock:solution` and the Julius evaluator need that word; a side check during the session confirmed that changing it would break scoring. The validator checks each engine's layout (Julius: `buggy.patch` and `solution/fix.patch`, no `game/`), its tests include broken tasks that must fail, and the gate runs plain validation until every task has a reference solution. The 9 Pong solutions are a separate plan item, due before the Phase 5 `mock:solution` check.

## What it does not settle
- When `--strict` joins the gate: after the Pong solutions land.
- Whether the task generators should check the rules themselves when they make a task.
