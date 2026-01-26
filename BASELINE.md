# Game Development Benchmark - Baseline Results

**Model:** Claude 3.5 Haiku (`claude:haiku`)
**Date:** 2026-01-25
**Total Tasks:** 225

## Summary

| Engine | Passed | Total | Rate |
|--------|--------|-------|------|
| Pygame | 35 | 165 | 21% |
| Julius | 36 | 50 | 72% |
| Quake | 4 | 10 | 40% |
| **Total** | **75** | **225** | **33%** |

---

## Pygame (Python 2D Games)

**Run ID:** 20260125_161401
**Pass Rate:** 35/165 (21%)

### By Game

| Game | Passed | Total | Rate |
|------|--------|-------|------|
| Pong | 13 | 22 | 59% |
| Snake | 11 | 27 | 41% |
| Breakout | 6 | 26 | 23% |
| Space Invaders | 3 | 23 | 13% |
| Platformer | 1 | 17 | 6% |
| Tower Defense | 1 | 17 | 6% |
| Asteroids | 0 | 17 | 0% |
| Tetris | 0 | 16 | 0% |

### By Category

| Category | Passed | Total | Rate |
|----------|--------|-------|------|
| Bug Fix | 14 | 34 | 41% |
| Feature | 15 | 95 | 16% |
| Optimization | 3 | 14 | 21% |
| Mini-Game | 3 | 22 | 14% |

### By Tier

| Tier | Passed | Total | Rate |
|------|--------|-------|------|
| 1 (Easy) | 10 | 18 | 56% |
| 2 (Moderate) | 10 | 18 | 56% |
| 3 (Hard) | 10 | 74 | 14% |
| 4 (Very Hard) | 4 | 39 | 10% |
| 5 (Expert) | 1 | 16 | 6% |

### Individual Results

#### Pong (13/22)

| Task | Result | Category | Tier |
|------|--------|----------|------|
| pong-001 | PASS | bug-fix | 1 |
| pong-002 | PASS | bug-fix | 1 |
| pong-003 | FAIL | bug-fix | 1 |
| pong-004 | PASS | bug-fix | 2 |
| pong-005 | PASS | bug-fix | 2 |
| pong-006 | PASS | feature | 2 |
| pong-007 | PASS | feature | 2 |
| pong-008 | FAIL | feature | 3 |
| pong-009 | PASS | optimization | 2 |
| pong-010 | PASS | mini-game | 3 |
| pong-bug-006 | FAIL | bug-fix | 3 |
| pong-feature-011 | PASS | feature | 3 |
| pong-feature-012 | FAIL | feature | 3 |
| pong-feature-013 | FAIL | feature | 4 |
| pong-feature-014 | FAIL | feature | 4 |
| pong-feature-015 | PASS | feature | 3 |
| pong-feature-016 | FAIL | feature | 4 |
| pong-feature-017 | PASS | feature | 3 |
| pong-mini-002 | FAIL | mini-game | 4 |
| pong-mini-003 | PASS | mini-game | 3 |
| pong-mini-004 | FAIL | mini-game | 4 |
| pong-optimization-001 | PASS | optimization | 3 |

#### Snake (11/27)

| Task | Result | Category | Tier |
|------|--------|----------|------|
| snake-001 | FAIL | bug-fix | 1 |
| snake-002 | FAIL | bug-fix | 1 |
| snake-003 | PASS | bug-fix | 1 |
| snake-004 | FAIL | bug-fix | 1 |
| snake-005 | PASS | bug-fix | 1 |
| snake-006 | PASS | bug-fix | 2 |
| snake-bug-007 | PASS | bug-fix | 2 |
| snake-feature-001 | FAIL | feature | 2 |
| snake-feature-002 | PASS | feature | 1 |
| snake-feature-003 | FAIL | feature | 2 |
| snake-feature-004 | FAIL | feature | 2 |
| snake-feature-005 | PASS | feature | 3 |
| snake-feature-006 | PASS | feature | 1 |
| snake-feature-007 | PASS | feature | 1 |
| snake-feature-008 | FAIL | feature | 3 |
| snake-feature-009 | FAIL | feature | 3 |
| snake-feature-010 | FAIL | feature | 3 |
| snake-feature-011 | FAIL | feature | 4 |
| snake-feature-012 | PASS | feature | 2 |
| snake-feature-013 | PASS | feature | 2 |
| snake-feature-014 | PASS | feature | 2 |
| snake-mini-001 | FAIL | mini-game | 3 |
| snake-mini-002 | FAIL | mini-game | 4 |
| snake-mini-003 | FAIL | mini-game | 4 |
| snake-mini-004 | FAIL | mini-game | 5 |
| snake-optimization-001 | FAIL | optimization | 2 |
| snake-optimization-002 | FAIL | optimization | 3 |

#### Breakout (6/26)

| Task | Result | Category | Tier |
|------|--------|----------|------|
| breakout-001 | PASS | bug-fix | 1 |
| breakout-002 | PASS | bug-fix | 1 |
| breakout-003 | FAIL | bug-fix | 1 |
| breakout-004 | PASS | bug-fix | 1 |
| breakout-005 | PASS | bug-fix | 2 |
| breakout-006 | FAIL | bug-fix | 1 |
| breakout-bug-007 | FAIL | bug-fix | 2 |
| breakout-feature-001 | FAIL | feature | 3 |
| breakout-feature-002 | FAIL | feature | 2 |
| breakout-feature-003 | FAIL | feature | 3 |
| breakout-feature-004 | FAIL | feature | 2 |
| breakout-feature-005 | FAIL | feature | 1 |
| breakout-feature-006 | FAIL | feature | 2 |
| breakout-feature-007 | FAIL | feature | 1 |
| breakout-feature-008 | FAIL | feature | 3 |
| breakout-feature-009 | FAIL | feature | 3 |
| breakout-feature-010 | FAIL | feature | 4 |
| breakout-feature-011 | FAIL | feature | 4 |
| breakout-feature-012 | FAIL | feature | 4 |
| breakout-feature-013 | FAIL | feature | 5 |
| breakout-feature-014 | FAIL | feature | 5 |
| breakout-mini-001 | FAIL | mini-game | 3 |
| breakout-mini-002 | FAIL | mini-game | 4 |
| breakout-mini-003 | PASS | mini-game | 3 |
| breakout-optimization-001 | PASS | optimization | 2 |
| breakout-optimization-002 | FAIL | optimization | 3 |

#### Space Invaders (3/23)

| Task | Result | Category | Tier |
|------|--------|----------|------|
| space_invaders-001 | PASS | bug-fix | 2 |
| space_invaders-002 | FAIL | bug-fix | 3 |
| space_invaders-003 | FAIL | bug-fix | 2 |
| space_invaders-004 | PASS | bug-fix | 2 |
| space_invaders-bug-005 | FAIL | bug-fix | 3 |
| space_invaders-feature-001 | FAIL | feature | 3 |
| space_invaders-feature-002 | FAIL | feature | 3 |
| space_invaders-feature-003 | FAIL | feature | 4 |
| space_invaders-feature-004 | PASS | feature | 4 |
| space_invaders-feature-005 | FAIL | feature | 3 |
| space_invaders-feature-006 | FAIL | feature | 3 |
| space_invaders-feature-007 | FAIL | feature | 4 |
| space_invaders-feature-008 | FAIL | feature | 4 |
| space_invaders-feature-009 | FAIL | feature | 4 |
| space_invaders-feature-010 | FAIL | feature | 5 |
| space_invaders-feature-011 | FAIL | feature | 5 |
| space_invaders-feature-012 | FAIL | feature | 5 |
| space_invaders-mini-001 | FAIL | mini-game | 4 |
| space_invaders-mini-002 | FAIL | mini-game | 5 |
| space_invaders-mini-003 | FAIL | mini-game | 5 |
| space_invaders-optimization-001 | FAIL | optimization | 3 |
| space_invaders-optimization-002 | FAIL | optimization | 4 |
| space_invaders-optimization-003 | FAIL | optimization | 4 |

#### Asteroids (0/17)

| Task | Result | Category | Tier |
|------|--------|----------|------|
| asteroids-bug-001 | FAIL | bug-fix | 3 |
| asteroids-bug-002 | FAIL | bug-fix | 3 |
| asteroids-feature-001 | FAIL | feature | 3 |
| asteroids-feature-002 | FAIL | feature | 3 |
| asteroids-feature-003 | FAIL | feature | 4 |
| asteroids-feature-004 | FAIL | feature | 4 |
| asteroids-feature-005 | FAIL | feature | 4 |
| asteroids-feature-006 | FAIL | feature | 4 |
| asteroids-feature-007 | FAIL | feature | 4 |
| asteroids-feature-008 | FAIL | feature | 5 |
| asteroids-feature-009 | FAIL | feature | 5 |
| asteroids-feature-010 | FAIL | feature | 5 |
| asteroids-feature-011 | FAIL | feature | 5 |
| asteroids-mini-001 | FAIL | mini-game | 4 |
| asteroids-mini-002 | FAIL | mini-game | 5 |
| asteroids-optimization-001 | FAIL | optimization | 4 |
| asteroids-optimization-002 | FAIL | optimization | 4 |

#### Tetris (0/16)

| Task | Result | Category | Tier |
|------|--------|----------|------|
| tetris-bug-001 | FAIL | bug-fix | 3 |
| tetris-bug-002 | FAIL | bug-fix | 3 |
| tetris-bug-003 | FAIL | bug-fix | 4 |
| tetris-feature-001 | FAIL | feature | 3 |
| tetris-feature-002 | FAIL | feature | 3 |
| tetris-feature-003 | FAIL | feature | 4 |
| tetris-feature-004 | FAIL | feature | 4 |
| tetris-feature-005 | FAIL | feature | 4 |
| tetris-feature-006 | FAIL | feature | 4 |
| tetris-feature-007 | FAIL | feature | 5 |
| tetris-feature-008 | FAIL | feature | 5 |
| tetris-feature-009 | FAIL | feature | 5 |
| tetris-feature-010 | FAIL | feature | 5 |
| tetris-mini-001 | FAIL | mini-game | 4 |
| tetris-mini-002 | FAIL | mini-game | 5 |
| tetris-optimization-001 | FAIL | optimization | 4 |

#### Platformer (1/17)

| Task | Result | Category | Tier |
|------|--------|----------|------|
| platformer-bug-001 | FAIL | bug-fix | 3 |
| platformer-bug-002 | FAIL | bug-fix | 3 |
| platformer-feature-001 | PASS | feature | 3 |
| platformer-feature-002 | FAIL | feature | 3 |
| platformer-feature-003 | FAIL | feature | 4 |
| platformer-feature-004 | FAIL | feature | 4 |
| platformer-feature-005 | FAIL | feature | 4 |
| platformer-feature-006 | FAIL | feature | 4 |
| platformer-feature-007 | FAIL | feature | 4 |
| platformer-feature-008 | FAIL | feature | 5 |
| platformer-feature-009 | FAIL | feature | 5 |
| platformer-feature-010 | FAIL | feature | 5 |
| platformer-feature-011 | FAIL | feature | 5 |
| platformer-feature-012 | FAIL | feature | 5 |
| platformer-mini-001 | FAIL | mini-game | 4 |
| platformer-mini-002 | FAIL | mini-game | 5 |
| platformer-optimization-001 | FAIL | optimization | 4 |

#### Tower Defense (1/17)

| Task | Result | Category | Tier |
|------|--------|----------|------|
| tower_defense-bug-001 | FAIL | bug-fix | 3 |
| tower_defense-bug-002 | FAIL | bug-fix | 3 |
| tower_defense-feature-001 | FAIL | feature | 3 |
| tower_defense-feature-002 | PASS | feature | 3 |
| tower_defense-feature-003 | FAIL | feature | 4 |
| tower_defense-feature-004 | FAIL | feature | 4 |
| tower_defense-feature-005 | FAIL | feature | 4 |
| tower_defense-feature-006 | FAIL | feature | 4 |
| tower_defense-feature-007 | FAIL | feature | 4 |
| tower_defense-feature-008 | FAIL | feature | 5 |
| tower_defense-feature-009 | FAIL | feature | 5 |
| tower_defense-feature-010 | FAIL | feature | 5 |
| tower_defense-feature-011 | FAIL | feature | 5 |
| tower_defense-feature-012 | FAIL | feature | 5 |
| tower_defense-mini-001 | FAIL | mini-game | 4 |
| tower_defense-mini-002 | FAIL | mini-game | 5 |
| tower_defense-optimization-001 | FAIL | optimization | 4 |

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
- **Simple Games:** Pong tasks have 59% pass rate, Snake 41%
- **Easy Tiers:** Tier 1-2 tasks show 56% pass rate

### Weaknesses
- **New M2 Games:** Asteroids and Tetris have 0% pass rate - complex games with higher difficulty tasks
- **High-Tier Tasks:** Tier 4-5 tasks show only 8% pass rate (5/55)
- **Feature Tasks:** 16% pass rate - harder than bug fixes
- **Quake Features/Optimization:** 0% pass rate on tier 5 feature and optimization tasks

### Difficulty Scaling
- Tier 1-2: Good performance (56%)
- Tier 3: Mixed results (14%)
- Tier 4-5: Low pass rates (8%)

### M1 vs M2 Comparison
- **M1 Tasks (Original 50):** Higher pass rates on original games
- **M2 Tasks (New 115):** Significantly harder, especially new games (Asteroids, Tetris, Platformer, Tower Defense)

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
| 20260125_161401 | Pygame | `results/runs/20260125_161401/` |
| 20260124_163954 | Quake | `results/runs/20260124_163954/` |
| 20260125_004623 | Julius | `results/runs/20260125_004623/` |
