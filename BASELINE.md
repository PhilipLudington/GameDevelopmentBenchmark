# Game Development Benchmark - Baseline Results

**Model:** Claude 3.5 Haiku (`claude:haiku`)
**Date:** 2026-01-25
**Total Tasks:** 110

## Summary

| Engine | Passed | Total | Rate |
|--------|--------|-------|------|
| Pygame | 18 | 50 | 36% |
| Julius | 36 | 50 | 72% |
| Quake | 4 | 10 | 40% |
| **Total** | **58** | **110** | **53%** |

---

## Pygame (Python 2D Games)

**Run ID:** 20260123_113358
**Pass Rate:** 18/50 (36%)

### By Game

| Game | Passed | Total | Rate |
|------|--------|-------|------|
| Pong | 8 | 10 | 80% |
| Snake | 6 | 15 | 40% |
| Breakout | 4 | 15 | 27% |
| Space Invaders | 0 | 10 | 0% |

### By Category

| Category | Passed | Total | Rate |
|----------|--------|-------|------|
| Bug Fix | 9 | 21 | 43% |
| Feature | 6 | 19 | 32% |
| Optimization | 2 | 4 | 50% |
| Mini-Game | 1 | 4 | 25% |

### By Tier

| Tier | Passed | Total | Rate |
|------|--------|-------|------|
| 1 (Easy) | 7 | 12 | 58% |
| 2 (Moderate) | 6 | 15 | 40% |
| 3 (Hard) | 3 | 11 | 27% |
| 4 (Very Hard) | 2 | 6 | 33% |

### Individual Results

#### Pong (8/10)

| Task | Result | Category | Tier |
|------|--------|----------|------|
| pong-001 | FAIL | bug-fix | 1 |
| pong-002 | FAIL | bug-fix | 1 |
| pong-003 | PASS | bug-fix | 1 |
| pong-004 | PASS | bug-fix | 2 |
| pong-005 | PASS | bug-fix | 2 |
| pong-006 | PASS | feature | 2 |
| pong-007 | PASS | feature | 2 |
| pong-008 | PASS | feature | 3 |
| pong-009 | PASS | optimization | 2 |
| pong-010 | PASS | mini-game | 3 |

#### Snake (6/15)

| Task | Result | Category | Tier |
|------|--------|----------|------|
| snake-001 | FAIL | bug-fix | 1 |
| snake-002 | FAIL | bug-fix | 1 |
| snake-003 | PASS | bug-fix | 1 |
| snake-004 | PASS | bug-fix | 1 |
| snake-005 | PASS | bug-fix | 1 |
| snake-006 | PASS | bug-fix | 2 |
| snake-feature-001 | FAIL | feature | 2 |
| snake-feature-002 | PASS | feature | 1 |
| snake-feature-003 | FAIL | feature | 2 |
| snake-feature-004 | FAIL | feature | 2 |
| snake-feature-005 | FAIL | feature | 3 |
| snake-feature-006 | PASS | feature | 1 |
| snake-feature-007 | FAIL | feature | 1 |
| snake-mini-001 | FAIL | mini-game | 3 |
| snake-optimization-001 | FAIL | optimization | 2 |

#### Breakout (4/15)

| Task | Result | Category | Tier |
|------|--------|----------|------|
| breakout-001 | FAIL | bug-fix | 1 |
| breakout-002 | FAIL | bug-fix | 1 |
| breakout-003 | FAIL | bug-fix | 1 |
| breakout-004 | PASS | bug-fix | 1 |
| breakout-005 | PASS | bug-fix | 2 |
| breakout-006 | FAIL | bug-fix | 1 |
| breakout-feature-001 | PASS | feature | 3 |
| breakout-feature-002 | FAIL | feature | 2 |
| breakout-feature-003 | FAIL | feature | 3 |
| breakout-feature-004 | FAIL | feature | 2 |
| breakout-feature-005 | FAIL | feature | 1 |
| breakout-feature-006 | FAIL | feature | 2 |
| breakout-feature-007 | FAIL | feature | 1 |
| breakout-mini-001 | FAIL | mini-game | 3 |
| breakout-optimization-001 | PASS | optimization | 2 |

#### Space Invaders (0/10)

| Task | Result | Category | Tier |
|------|--------|----------|------|
| space_invaders-001 | FAIL | bug-fix | 2 |
| space_invaders-002 | FAIL | bug-fix | 3 |
| space_invaders-003 | FAIL | bug-fix | 2 |
| space_invaders-004 | FAIL | bug-fix | 2 |
| space_invaders-feature-001 | FAIL | feature | 3 |
| space_invaders-feature-002 | FAIL | feature | 3 |
| space_invaders-feature-003 | FAIL | feature | 4 |
| space_invaders-feature-004 | FAIL | feature | 4 |
| space_invaders-mini-001 | FAIL | mini-game | 4 |
| space_invaders-optimization-001 | FAIL | optimization | 3 |

---

## Julius (C Systems Programming)

**Run ID:** 20260125_004623
**Pass Rate:** 36/50 (72%)

### By Category

| Category | Passed | Total | Rate |
|----------|--------|-------|------|
| Memory Safety | 15 | 19 | 79% |
| Game Logic | 11 | 14 | 79% |
| Crash Fix | 6 | 10 | 60% |
| Visual/UI | 4 | 7 | 57% |

### By Tier

| Tier | Passed | Total | Rate |
|------|--------|-------|------|
| 1 (Easy) | 1 | 3 | 33% |
| 2 (Moderate) | 17 | 20 | 85% |
| 3 (Hard) | 16 | 22 | 73% |
| 4 (Very Hard) | 2 | 5 | 40% |

### Individual Results

#### Memory Safety (15/19)

| Task | Result | Tier | Description |
|------|--------|------|-------------|
| julius-001 | PASS | 3 | Double free in smacker decoder |
| julius-002 | PASS | 3 | Dangling pointer on localized_filename |
| julius-003 | FAIL | 3 | Sheep OOB destination |
| julius-007 | PASS | 3 | Buffer overflow in filename handling |
| julius-008 | PASS | 3 | Integer overflow in resource calc |
| julius-009 | PASS | 2 | Null pointer dereference in building |
| julius-010 | FAIL | 4 | Use-after-free in UI callback |
| julius-011 | PASS | 3 | Stack buffer overflow in translation string |
| julius-012 | PASS | 3 | Heap buffer overflow in config parsing |
| julius-013 | PASS | 2 | Uninitialized memory in figure creation |
| julius-014 | PASS | 3 | Double free in sound cleanup |
| julius-015 | FAIL | 4 | Use-after-free in building deletion callback |
| julius-016 | PASS | 3 | Integer underflow in resource subtraction |
| julius-017 | PASS | 2 | Null pointer in empty walker list |
| julius-018 | PASS | 3 | Out-of-bounds read in sprite lookup |
| julius-019 | PASS | 4 | Race condition in message queue |
| julius-020 | PASS | 3 | Memory leak in error handling path |
| julius-021 | PASS | 2 | Off-by-one error in array bounds |
| julius-022 | FAIL | 3 | Type confusion with building ID cast |

#### Game Logic (11/14)

| Task | Result | Tier | Description |
|------|--------|------|-------------|
| julius-005 | PASS | 2 | Hotkey config ordering error |
| julius-006 | FAIL | 2 | Clone building ignores disallowed buildings |
| julius-033 | PASS | 2 | Storage building undo linking |
| julius-034 | PASS | 2 | Palace upgrade validation |
| julius-035 | PASS | 2 | Popularity decay rounding error |
| julius-036 | FAIL | 3 | Plague spread rate integer math |
| julius-037 | PASS | 3 | Tower archer range ignores elevation |
| julius-038 | PASS | 2 | Granary cart pathfinding loop |
| julius-039 | PASS | 3 | Waypoint deletion corruption |
| julius-040 | PASS | 2 | Fireworks timing offset |
| julius-041 | FAIL | 3 | Enemy formation rank overflow |
| julius-042 | PASS | 2 | Trade caravan follower index |
| julius-043 | PASS | 3 | Prefect message queue overflow |
| julius-044 | PASS | 2 | Building cost refund error |

#### Crash Fix (6/10)

| Task | Result | Tier | Description |
|------|--------|------|-------------|
| julius-023 | PASS | 4 | Water building data corruption on collapse |
| julius-024 | PASS | 3 | Build menu hitbox calculation error |
| julius-025 | FAIL | 2 | Reservoir placement double-charge |
| julius-026 | FAIL | 3 | Gatehouse infinite road respawn |
| julius-027 | PASS | 3 | Division by zero in population ratio |
| julius-028 | PASS | 2 | Division by zero in resource ratio |
| julius-029 | PASS | 3 | Infinite loop in pathfinding |
| julius-030 | FAIL | 4 | Stack overflow from deep recursion |
| julius-031 | PASS | 2 | Assert failure in invalid state |
| julius-032 | FAIL | 3 | Crash on malformed save file |

#### Visual/UI (4/7)

| Task | Result | Tier | Description |
|------|--------|------|-------------|
| julius-004 | PASS | 1 | Tooltip trailing newline |
| julius-045 | PASS | 2 | Animation offset calculation |
| julius-046 | PASS | 2 | Amphitheater spectator offset |
| julius-047 | FAIL | 1 | Button highlight color lookup |
| julius-048 | PASS | 2 | Scrollbar position rounding jitter |
| julius-049 | FAIL | 2 | Cursor hotspot offset |
| julius-050 | FAIL | 1 | Panel text truncation |

---

## Quake (C Engine Programming - Expert)

**Run ID:** 20260124_163954
**Pass Rate:** 4/10 (40%)

### By Category

| Category | Passed | Total | Rate |
|----------|--------|-------|------|
| Bug Fix | 4 | 4 | 100% |
| Feature | 0 | 3 | 0% |
| Optimization | 0 | 3 | 0% |

### By Tier

| Tier | Passed | Total | Rate |
|------|--------|-------|------|
| 4 (Very Hard) | 2 | 4 | 50% |
| 5 (Expert) | 2 | 6 | 33% |

### Individual Results

| Task | Result | Category | Tier | Description |
|------|--------|----------|------|-------------|
| quake-001 | PASS | bug-fix | 4 | BSP node child pointer corruption |
| quake-002 | PASS | bug-fix | 5 | Zone memory allocator corruption |
| quake-003 | PASS | bug-fix | 5 | Network prediction desync |
| quake-004 | PASS | bug-fix | 4 | Sound mixing overflow |
| quake-feat-001 | FAIL | feature | 5 | Skeletal animation system |
| quake-feat-002 | FAIL | feature | 5 | Portal-based occlusion culling |
| quake-feat-003 | FAIL | feature | 5 | Procedural terrain generation |
| quake-opt-001 | FAIL | optimization | 4 | Optimize PVS decompression |
| quake-opt-002 | FAIL | optimization | 4 | Optimize entity linked list |
| quake-opt-003 | FAIL | optimization | 5 | Optimize lightmap interpolation |

---

## Key Observations

### Strengths
- **Julius Memory Safety:** 79% pass rate on memory bugs (null pointers, buffer overflows, double frees)
- **Quake Bug Fixes:** 100% pass rate on tier 4-5 bug fixes in complex C engine code
- **Simple Games:** Pong tasks have 80% pass rate

### Weaknesses
- **Space Invaders:** 0% pass rate - complex game with higher difficulty tasks
- **Quake Features/Optimization:** 0% pass rate on tier 5 feature and optimization tasks
- **Visual/UI Tasks:** Generally lower pass rates across engines

### Difficulty Scaling
- Tier 1-2: Generally good performance (50-85%)
- Tier 3: Mixed results (27-73%)
- Tier 4-5: Low pass rates for features/optimization, but reasonable for bug fixes

---

## Reproduction

```bash
# Run Pygame baseline
python scripts/run_benchmark.py -m claude:haiku --engine pygame -o results/runs/

# Run Julius baseline
python scripts/run_benchmark.py -m claude:haiku --engine julius -o results/runs/

# Run Quake baseline
python scripts/run_benchmark.py -m claude:haiku --engine quake -o results/runs/

# Run all engines
python scripts/run_benchmark.py -m claude:haiku -o results/runs/
```

---

## Raw Data

| Run ID | Engine | Directory |
|--------|--------|-----------|
| 20260123_113358 | Pygame | `results/runs/20260123_113358/` |
| 20260124_163954 | Quake | `results/runs/20260124_163954/` |
| 20260125_004623 | Julius | `results/runs/20260125_004623/` |
