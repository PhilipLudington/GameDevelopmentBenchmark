# Julius Baseline Results - Claude Haiku

**Date:** 2026-01-25
**Model:** claude:haiku (Claude 3.5 Haiku)
**Engine:** Julius (50 tasks)
**Run ID:** 20260125_004623

## Summary

| Metric | Value |
|--------|-------|
| **Total Pass Rate** | **36/50 (72%)** |
| Average Score | 0.86 |

## Results by Category

| Category | Passed | Total | Rate |
|----------|--------|-------|------|
| Memory Safety | 15 | 19 | 79% |
| Game Logic | 11 | 14 | 79% |
| Crash Fix | 6 | 10 | 60% |
| Visual/UI | 4 | 7 | 57% |

## Results by Tier

| Tier | Passed | Total | Rate |
|------|--------|-------|------|
| 1 (Easy) | 1 | 3 | 33% |
| 2 (Moderate) | 17 | 20 | 85% |
| 3 (Hard) | 16 | 22 | 73% |
| 4 (Very Hard) | 2 | 5 | 40% |

## Individual Task Results

### Passed (36 tasks)

| ID | Category | Tier | Description |
|----|----------|------|-------------|
| julius-001 | memory-safety | 3 | Double free in smacker decoder |
| julius-002 | memory-safety | 3 | Dangling pointer on localized_filename |
| julius-004 | visual | 1 | Tooltip trailing newline |
| julius-005 | game-logic | 2 | Hotkey config ordering error |
| julius-007 | memory-safety | 3 | Buffer overflow in filename handling |
| julius-008 | memory-safety | 3 | Integer overflow in resource calc |
| julius-009 | memory-safety | 2 | Null pointer dereference in building |
| julius-011 | memory-safety | 3 | Stack buffer overflow in translation string |
| julius-012 | memory-safety | 3 | Heap buffer overflow in config parsing |
| julius-013 | memory-safety | 2 | Uninitialized memory in figure creation |
| julius-014 | memory-safety | 3 | Double free in sound cleanup |
| julius-016 | memory-safety | 3 | Integer underflow in resource subtraction |
| julius-017 | memory-safety | 2 | Null pointer in empty walker list |
| julius-018 | memory-safety | 3 | Out-of-bounds read in sprite lookup |
| julius-019 | memory-safety | 4 | Race condition in message queue |
| julius-020 | memory-safety | 3 | Memory leak in error handling path |
| julius-021 | memory-safety | 2 | Off-by-one error in array bounds |
| julius-023 | crash-fix | 4 | Water building data corruption on collapse |
| julius-024 | crash-fix | 3 | Build menu hitbox calculation error |
| julius-027 | crash-fix | 3 | Division by zero in population ratio |
| julius-028 | crash-fix | 2 | Division by zero in resource ratio |
| julius-029 | crash-fix | 3 | Infinite loop in pathfinding |
| julius-031 | crash-fix | 2 | Assert failure in invalid state |
| julius-033 | game-logic | 2 | Storage building undo linking |
| julius-034 | game-logic | 2 | Palace upgrade validation |
| julius-035 | game-logic | 2 | Popularity decay rounding error |
| julius-037 | game-logic | 3 | Tower archer range ignores elevation |
| julius-038 | game-logic | 2 | Granary cart pathfinding loop |
| julius-039 | game-logic | 3 | Waypoint deletion corruption |
| julius-040 | game-logic | 2 | Fireworks timing offset |
| julius-042 | game-logic | 2 | Trade caravan follower index |
| julius-043 | game-logic | 3 | Prefect message queue overflow |
| julius-044 | game-logic | 2 | Building cost refund error |
| julius-045 | visual | 2 | Animation offset calculation |
| julius-046 | visual | 2 | Amphitheater spectator offset |
| julius-048 | visual | 2 | Scrollbar position rounding jitter |

### Failed (14 tasks)

| ID | Category | Tier | Description |
|----|----------|------|-------------|
| julius-003 | memory-safety | 3 | Sheep OOB destination |
| julius-006 | game-logic | 2 | Clone building ignores disallowed buildings |
| julius-010 | memory-safety | 4 | Use-after-free in UI callback |
| julius-015 | memory-safety | 4 | Use-after-free in building deletion callback |
| julius-022 | memory-safety | 3 | Type confusion with building ID cast |
| julius-025 | crash-fix | 2 | Reservoir placement double-charge |
| julius-026 | crash-fix | 3 | Gatehouse infinite road respawn |
| julius-030 | crash-fix | 4 | Stack overflow from deep recursion |
| julius-032 | crash-fix | 3 | Crash on malformed save file |
| julius-036 | game-logic | 3 | Plague spread rate integer math |
| julius-041 | game-logic | 3 | Enemy formation rank overflow |
| julius-047 | visual | 1 | Button highlight color lookup |
| julius-049 | visual | 2 | Cursor hotspot offset |
| julius-050 | visual | 1 | Panel text truncation |

## Comparison with MJ1 Baseline

| Metric | MJ1 (10 tasks) | MJ2 (50 tasks) | Delta |
|--------|----------------|----------------|-------|
| Pass Rate | 80% (8/10) | 72% (36/50) | -8% |
| Avg Score | 0.82 | 0.86 | +0.04 |

The slight decrease in pass rate is expected with the harder synthetic tasks in MJ2.

## Notes

- All tasks are asset-free (no Caesar III assets required)
- Evaluation uses standalone test harnesses with AddressSanitizer
- Scoring: compiles (0.2) + no ASan errors (0.2) + tests pass (0.4) + fix structure match (0.2)
- Run command: `python scripts/run_benchmark.py -m claude:haiku --engine julius`
