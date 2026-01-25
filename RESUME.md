# MJ2 Julius Track Expansion - Progress Resume

## Status: COMPLETE

The Julius benchmark track has been expanded from 10 tasks (MJ1) to 50 tasks (MJ2).

## Summary of Work Completed

### Tasks Created

| Category | ID Range | Count | Status |
|----------|----------|-------|--------|
| Memory Safety | julius-011 to julius-022 | 12 | Complete |
| Crash Fix | julius-023 to julius-032 | 10 | Complete |
| Game Logic | julius-033 to julius-044 | 12 | Complete |
| Visual/UI | julius-045 to julius-050 | 6 | Complete |
| **Total New** | julius-011 to julius-050 | **40** | **Complete** |

Combined with existing MJ1 tasks (julius-001 to julius-010), there are now **50 total Julius tasks**.

## Directory Structure

All tasks are located under `tasks/julius/` organized by category:

```
tasks/julius/
├── memory-safety/
│   ├── julius-001 through julius-003  (MJ1)
│   ├── julius-007 through julius-010  (MJ1)
│   └── julius-011 through julius-022  (MJ2 - NEW)
├── game-logic/
│   ├── julius-005, julius-006  (MJ1)
│   └── julius-033 through julius-044  (MJ2 - NEW)
├── crash-fix/
│   └── julius-023 through julius-032  (MJ2 - NEW)
└── visual/
    ├── julius-004  (MJ1)
    └── julius-045 through julius-050  (MJ2 - NEW)
```

## Task Files Per Task

Each task contains:
- `task.json` - Task metadata (id, name, category, tier, files_to_modify, etc.)
- `prompt.md` - Bug report for AI to solve
- `buggy.patch` - Patch that introduces the bug
- `solution/fix.patch` - Reference solution patch
- `tests/Makefile` - Build configuration
- `tests/test_*.c` - Test implementation
- `tests/*_stubs.c` - Stub implementations for dependencies

## Category Distribution (50 tasks total)

| Category | Count | Percentage |
|----------|-------|------------|
| Memory Safety | 19 | 38% |
| Crash Fix | 10 | 20% |
| Game Logic | 14 | 28% |
| Visual/UI | 7 | 14% |

## Tier Distribution (50 tasks total)

| Tier | Count | Percentage |
|------|-------|------------|
| 1 (Easy) | 3 | 6% |
| 2 (Moderate) | 20 | 40% |
| 3 (Hard) | 21 | 42% |
| 4 (Very Hard) | 6 | 12% |

## MJ2 New Tasks Detail

### Memory Safety (julius-011 to julius-022)

| ID | Tier | Bug Type |
|----|------|----------|
| julius-011 | 3 | Stack buffer overflow in translation string |
| julius-012 | 3 | Heap buffer overflow in config parsing |
| julius-013 | 2 | Uninitialized memory in figure creation |
| julius-014 | 3 | Double free in sound cleanup |
| julius-015 | 4 | Use-after-free in building deletion callback |
| julius-016 | 3 | Integer underflow in resource subtraction |
| julius-017 | 2 | Null pointer in empty walker list |
| julius-018 | 3 | Out-of-bounds read in sprite lookup |
| julius-019 | 4 | Race condition in message queue |
| julius-020 | 3 | Memory leak in error handling path |
| julius-021 | 2 | Off-by-one error in array bounds |
| julius-022 | 3 | Type confusion with building ID cast |

### Crash Fix (julius-023 to julius-032)

| ID | Tier | Bug Type |
|----|------|----------|
| julius-023 | 4 | Water building data corruption on collapse |
| julius-024 | 3 | Build menu hitbox calculation error |
| julius-025 | 2 | Reservoir placement double-charge |
| julius-026 | 3 | Gatehouse infinite road respawn |
| julius-027 | 3 | Division by zero in population ratio |
| julius-028 | 2 | Division by zero in resource ratio |
| julius-029 | 3 | Infinite loop in pathfinding |
| julius-030 | 4 | Stack overflow from deep recursion |
| julius-031 | 2 | Assert failure in invalid state |
| julius-032 | 3 | Crash on malformed save file |

### Game Logic (julius-033 to julius-044)

| ID | Tier | Bug Type |
|----|------|----------|
| julius-033 | 2 | Storage building undo linking |
| julius-034 | 2 | Palace upgrade validation |
| julius-035 | 2 | Popularity decay rounding error |
| julius-036 | 3 | Plague spread rate integer math |
| julius-037 | 3 | Tower archer range ignores elevation |
| julius-038 | 2 | Granary cart pathfinding loop |
| julius-039 | 3 | Waypoint deletion corruption |
| julius-040 | 2 | Fireworks timing offset |
| julius-041 | 3 | Enemy formation rank overflow |
| julius-042 | 2 | Trade caravan follower index |
| julius-043 | 3 | Prefect message queue overflow |
| julius-044 | 2 | Building cost refund error |

### Visual/UI (julius-045 to julius-050)

| ID | Tier | Bug Type |
|----|------|----------|
| julius-045 | 2 | Animation offset calculation |
| julius-046 | 2 | Amphitheater spectator offset |
| julius-047 | 1 | Button highlight color lookup |
| julius-048 | 2 | Scrollbar position rounding jitter |
| julius-049 | 2 | Cursor hotspot offset |
| julius-050 | 1 | Panel text truncation |

## Asset Requirements

| Type | Count | Percentage |
|------|-------|------------|
| Asset-Free | 47 | 94% |
| Requires Assets | 3 | 6% |

All 40 new MJ2 tasks are asset-free.

## Next Steps (Verification)

To verify all tasks:

```bash
# Count total tasks
find tasks/julius -name "task.json" | wc -l
# Expected: 50

# Run benchmark validation with mock solution
python scripts/run_benchmark.py \
  --engine julius \
  --model mock:solution \
  --output results/validation/

# Expected: 50/50 pass
```

## Implementation Notes

- All tasks follow existing MJ1 patterns
- Tests are standalone and don't require full Julius build
- Evaluation uses ASan for memory safety tasks, unit-test for others
- Each test has buggy/fixed versions toggled via BUGGY_VERSION define
