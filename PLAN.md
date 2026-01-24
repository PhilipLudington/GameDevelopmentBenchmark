# Pygame Benchmark Implementation Plan

## Overview

Implement Phase 1 (Pygame) of the Game Development Benchmark, targeting M1 (Proof of Concept: 50 tasks) first, then M2 (Pygame Complete: 200+ tasks).

**Key Decisions:**
- Model interface: Support both API-based and CLI-based invocation
- Task sourcing: Synthetic tasks first (controlled, reproducible)
- First baseline game: Pong (simplest, validates infrastructure)

---

## Directory Structure

```
GameDevelopmentBenchmark/
├── DESIGN.md
├── PLAN.md
├── README.md
├── LICENSE
├── tasks/
│   └── pygame/
│       ├── bug-fix/
│       │   └── <task-id>/
│       │       ├── task.json          # Task metadata
│       │       ├── prompt.md          # Problem statement for AI
│       │       ├── game/              # Broken game code
│       │       ├── solution/          # Reference solution (hidden)
│       │       └── tests/             # Automated tests
│       ├── feature/
│       ├── optimization/
│       └── mini-game/
├── evaluation/
│   ├── runner.py                      # Main evaluation harness
│   ├── test_runner.py                 # Unit/integration test executor
│   ├── gameplay_bot.py                # Automated gameplay framework
│   ├── performance.py                 # Performance benchmark runner
│   └── report.py                      # Results aggregation
├── harness/
│   ├── sandbox.py                     # Isolated execution environment
│   ├── pygame_headless.py             # Headless pygame utilities
│   └── metrics.py                     # Metric collection
├── models/
│   ├── base.py                        # Abstract model interface
│   ├── api_model.py                   # API-based models (OpenAI, Anthropic)
│   └── cli_model.py                   # CLI-based models (ollama, llama.cpp, claude)
├── baselines/
│   └── pygame/
│       └── pong/                      # First baseline game
├── results/
│   └── runs/                          # Evaluation run outputs (JSON)
├── leaderboard/
│   └── index.html                     # Static leaderboard (GitHub Pages)
├── scripts/
│   ├── create_task.py                 # Task scaffolding tool
│   ├── validate_task.py               # Task validation
│   └── run_benchmark.py               # Full benchmark runner
├── .github/
│   └── workflows/
│       ├── ci.yml                     # PR validation
│       └── benchmark.yml              # Scheduled benchmark runs
├── pyproject.toml
└── requirements.txt
```

---

## Implementation Phases

### Phase 1.1: Core Infrastructure

#### 1.1.1 Project Setup
- [x] Create `pyproject.toml` with project metadata
- [x] Create `requirements.txt` with dependencies
- [x] Set up basic directory structure

#### 1.1.2 Task Format & Schema
- [x] Define `task.json` JSON schema:
  ```json
  {
    "id": "pong-collision-001",
    "name": "Ball passes through paddle",
    "category": "bug-fix",
    "tier": 1,
    "engine": "pygame",
    "description": "Fix collision detection bug",
    "evaluation": ["unit-test", "gameplay"],
    "tags": ["collision", "physics"],
    "baseline": "pong"
  }
  ```
- [x] Define `prompt.md` template
- [x] Create JSON schema validation

#### 1.1.3 Model Interface
- [x] `models/base.py`: Abstract `ModelInterface` class
  - `generate(prompt: str, context: dict) -> str`
  - `get_name() -> str`
  - `get_config() -> dict`
- [x] `models/api_model.py`: HTTP API implementation
  - Support OpenAI, Anthropic, custom endpoints
  - Configurable via environment variables
- [x] `models/cli_model.py`: CLI implementation
  - Support ollama, llama.cpp, claude (Claude Code CLI)
  - Subprocess management with timeout

#### 1.1.4 Evaluation Harness
- [x] `evaluation/runner.py`: Main orchestrator
  - Load task from directory
  - Invoke model with prompt
  - Apply generated code to sandbox
  - Run evaluation phases
  - Collect results
- [x] `evaluation/test_runner.py`: pytest executor
  - Timeout and resource limits
  - Parse test results
- [x] `harness/sandbox.py`: Isolated execution
  - Temp directory management
  - Dependency installation
  - Cleanup

#### 1.1.5 Headless Pygame Support
- [x] `harness/pygame_headless.py`:
  - SDL_VIDEODRIVER=dummy setup
  - Mock display surface
  - Frame capture utilities
  - Input event injection

### Phase 1.2: First Baseline Game (Pong)

#### 1.2.1 Pong Implementation
- [x] `baselines/pygame/pong/main.py`: Working Pong game
  - Two paddles (player + AI)
  - Ball physics
  - Scoring system
  - Game states (menu, playing, game over)
- [x] Modular structure for easy bug injection
- [x] Headless mode support

#### 1.2.2 Pong Test Suite
- [x] Unit tests for game logic
- [x] Integration tests for gameplay
- [ ] Gameplay bot that can win

### Phase 1.3: Automated Gameplay Framework

#### 1.3.1 Bot Infrastructure
- [x] `evaluation/gameplay_bot.py`:
  ```python
  class GameplayBot(ABC):
      def on_frame(self, surface, game_state): ...
      def get_action(self) -> Action: ...
      def is_objective_complete(self) -> bool: ...
  ```
- [x] `RandomBot`: Random valid actions
- [x] `ScriptedBot`: Action sequences from file
- [x] `PongBot`: Rule-based Pong player

#### 1.3.2 Metrics Collection
- [x] `harness/metrics.py`:
  - Frame time tracking
  - Memory profiling
  - Game event logging

### Phase 1.4: First Tasks (10 Pong tasks)

Create initial task set from Pong baseline:

| ID | Category | Tier | Description | Status |
|----|----------|------|-------------|--------|
| pong-001 | bug-fix | 1 | Ball passes through paddle edge | Done |
| pong-002 | bug-fix | 1 | Score displays wrong player | Done |
| pong-003 | bug-fix | 1 | Ball stuck after scoring | Done |
| pong-004 | bug-fix | 2 | AI paddle jitters at boundaries | Done |
| pong-005 | bug-fix | 2 | Ball angle always same | Done |
| pong-006 | feature | 2 | Add pause functionality | Done |
| pong-007 | feature | 2 | Add ball speed increase | Done |
| pong-008 | feature | 3 | Add power-ups | Done |
| pong-009 | optimization | 2 | Reduce CPU usage in main loop | Done |
| pong-010 | mini-game | 3 | Create 4-player variant | Done |

### Phase 1.5: Expand to M1 (50 tasks)

#### Additional Baseline Games
- [x] Snake baseline (15 tasks complete)
  - Bug fixes (6):
    - snake-001: Direction reversal with quick inputs (tier 1)
    - snake-002: Food spawns on snake body (tier 1)
    - snake-003: Score not reset on new game (tier 1)
    - snake-004: Wall collision off by one (tier 1)
    - snake-005: High score resets when returning to menu (tier 1)
    - snake-006: Snake grows in wrong direction (tier 2)
  - Features (7):
    - snake-feature-001: Speed increase as snake grows (tier 2)
    - snake-feature-002: Add pause functionality (tier 1)
    - snake-feature-003: Add wrap-around walls mode (tier 2)
    - snake-feature-004: Add bonus food with timer (tier 2)
    - snake-feature-005: Add obstacles on the grid (tier 3)
    - snake-feature-006: Add length display (tier 1)
    - snake-feature-007: Add grid toggle (tier 1)
  - Optimization (1):
    - snake-optimization-001: Optimize collision detection with set (tier 2)
  - Mini-game (1):
    - snake-mini-001: Create two-player snake (tier 3)
- [x] Breakout baseline (15 tasks complete)
  - Bug fixes (6):
    - breakout-001: Ball passes through brick corners (tier 1)
    - breakout-002: Score callback receives wrong value (tier 1)
    - breakout-003: Ball stuck in paddle (tier 1)
    - breakout-004: Lives not resetting on new game (tier 1)
    - breakout-005: Ball escapes through screen corners (tier 2)
    - breakout-006: Paddle moves off screen edges (tier 1)
  - Features (7):
    - breakout-feature-001: Multi-ball power-up (tier 3)
    - breakout-feature-002: Paddle width power-up (tier 2)
    - breakout-feature-003: Multiple levels with patterns (tier 3)
    - breakout-feature-004: Multi-hit bricks (tier 2)
    - breakout-feature-005: Sound effects (tier 1)
    - breakout-feature-006: Ball speed increase (tier 2)
    - breakout-feature-007: Pause countdown (tier 1)
  - Optimization (1):
    - breakout-optimization-001: Optimize collision detection (tier 2)
  - Mini-game (1):
    - breakout-mini-001: Two-player cooperative mode (tier 3)
- [x] Space Invaders baseline (10 tasks complete - higher difficulty)
  - Bug fixes (4):
    - space_invaders-001: Bullets pass through aliens at high speed (tier 2) - tunneling bug
    - space_invaders-002: Alien fleet speed calculation overflow (tier 3) - math/division bug
    - space_invaders-003: Shield collision not symmetric (tier 2) - erosion direction bug
    - space_invaders-004: Dead aliens can still shoot (tier 2) - race condition bug
  - Features (4):
    - space_invaders-feature-001: Mystery ship bonus (tier 3)
    - space_invaders-feature-002: Progressive difficulty waves (tier 3)
    - space_invaders-feature-003: Power-up system (tier 4)
    - space_invaders-feature-004: Alien formation attack patterns (tier 4)
  - Optimization (1):
    - space_invaders-optimization-001: Spatial partitioning collision (tier 3)
  - Mini-game (1):
    - space_invaders-mini-001: Two-player cooperative mode (tier 4)

#### Task Distribution (M1):
| Category     | Tier 1 | Tier 2 | Tier 3 | Tier 4 | Total |
|--------------|--------|--------|--------|--------|-------|
| Bug Fix      | 10     | 8      | 5      | 2      | 25    |
| Feature      | 3      | 5      | 5      | 2      | 15    |
| Optimization | 0      | 2      | 3      | 0      | 5     |
| Mini-Game    | 0      | 0      | 3      | 2      | 5     |
| **Total**    | 13     | 15     | 16     | 6      | **50**|

---

## Task Creation Workflow

```bash
# Create new task from baseline
python scripts/create_task.py \
  --category bug-fix \
  --tier 2 \
  --baseline pong \
  --id "pong-ball-angle" \
  --name "Ball angle always same"

# Validate task structure
python scripts/validate_task.py tasks/pygame/bug-fix/pong-ball-angle/

# Run single task evaluation
python evaluation/runner.py \
  --task tasks/pygame/bug-fix/pong-ball-angle/ \
  --model openai:gpt-4 \
  --output results/runs/

# Run full benchmark
python scripts/run_benchmark.py \
  --model openai:gpt-4 \
  --model anthropic:claude-3 \
  --model claude:sonnet \
  --output results/runs/
```

---

## Dependencies

```txt
# Core
pygame>=2.5.0
pytest>=7.0.0
pytest-timeout>=2.0.0
pytest-json-report>=1.5.0

# Model interfaces
httpx>=0.25.0          # API calls
openai>=1.0.0          # OpenAI SDK
anthropic>=0.18.0      # Anthropic SDK

# Evaluation
psutil>=5.9.0          # Resource monitoring
pillow>=10.0.0         # Screenshot comparison

# Tools
jsonschema>=4.0.0      # Task validation
click>=8.0.0           # CLI tools
jinja2>=3.0.0          # Report generation
pyyaml>=6.0.0          # Config files
```

---

## Verification Plan

### 1. Infrastructure Verification
```bash
# Run harness unit tests
pytest tests/harness/ -v

# Verify headless pygame works
python -c "from harness.pygame_headless import HeadlessGame; print('OK')"
```

### 2. Baseline Game Verification
```bash
# Run Pong in headless mode
python baselines/pygame/pong/main.py --headless --frames 100

# Run Pong tests
pytest baselines/pygame/pong/tests/ -v
```

### 3. End-to-End Task Verification
```bash
# Create and validate a task
python scripts/create_task.py --category bug-fix --tier 1 --baseline pong --id test-001

# Run evaluation with mock model
python evaluation/runner.py \
  --task tasks/pygame/bug-fix/test-001/ \
  --model mock:pass \
  --verbose

# Verify results JSON generated
cat results/runs/latest.json | jq .
```

### 4. CI Verification
```bash
# Run CI workflow locally (using act or similar)
act -j validate
```

---

## Implementation Order

1. **Week 1: Foundation** ✅
   - Project setup (pyproject.toml, requirements)
   - Directory structure
   - Task JSON schema
   - Basic sandbox

2. **Week 2: Pong Baseline** ✅
   - Working Pong game
   - Headless support
   - Unit tests for Pong

3. **Week 3: Evaluation Harness** ✅
   - Model interface (API + CLI)
   - Test runner
   - Basic evaluation flow

4. **Week 4: First Tasks** ✅
   - 10 Pong-based tasks
   - Gameplay bot for Pong
   - End-to-end validation

5. **Week 5: CI/CD & Polish** (Partial)
   - GitHub Actions workflows (deferred)
   - Basic leaderboard (pending)
   - Documentation ✅

6. **Weeks 6-8: Scale to M1** (Pending)
   - Additional baseline games
   - Expand to 50 tasks
   - Model baseline runs

---

## Current Progress

**Status: 70 tasks across 3 engines (Pygame M1 ✅, Quake ✅, Julius MJ1 ✅)**

- Core infrastructure fully operational
- **70 tasks created and validated:**
  - **Pygame (50 tasks):**
    - 10 Pong tasks
    - 15 Snake tasks (baseline game + full task suite)
    - 15 Breakout tasks (baseline game + full task suite)
    - 10 Space Invaders tasks (baseline game + higher difficulty tasks)
  - **Quake (10 tasks):** Expert-level C engine tasks (tier 4-5)
  - **Julius (10 tasks):** Memory-safety, game-logic, and visual bugs (tier 1-4)
- Four Pygame baseline games fully implemented:
  - Pong: with test coverage
  - Snake: 43 unit tests
  - Breakout: 52 unit tests
  - Space Invaders: 62 unit tests
- Evaluation harness working with multiple model providers:
  - API: OpenAI, Anthropic
  - CLI: Ollama, llama.cpp, Claude Code
- Julius track integrated with main evaluation runner

**Recent Completions (Julius MJ1 - Complete):**
1. ✅ Completed all 10 Julius tasks with standalone C tests:
   - julius-001: Double free in smacker decoder (tier 3, memory-safety)
   - julius-002: Dangling pointer on localized_filename (tier 3, memory-safety)
   - julius-003: Sheep out-of-bounds destination (tier 3, memory-safety)
   - julius-004: Tooltip trailing newline (tier 1, visual)
   - julius-005: Hotkey config ordering mismatch (tier 2, game-logic)
   - julius-006: Clone building validation bypass (tier 2, game-logic)
   - julius-007: Buffer overflow in filename handling (tier 3, memory-safety)
   - julius-008: Integer overflow in resource calculation (tier 3, memory-safety)
   - julius-009: Null pointer dereference in building lookup (tier 2, memory-safety)
   - julius-010: Use-after-free in UI window callback (tier 4, memory-safety)

**Previous Completions (Phase 1 - Pygame):**
1. ✅ Improved pong-001 prompt to help models succeed (added detailed fix instructions)
2. ✅ Created Snake baseline game with full test coverage
3. ✅ Created all 15 Snake tasks
4. ✅ Created Breakout baseline game with 52 unit tests
5. ✅ Created all 15 Breakout tasks
6. ✅ Created Space Invaders baseline game with 62 unit tests
7. ✅ Created all 10 Space Invaders tasks (higher difficulty)

**Next Steps:**
1. Run model baselines to validate all 70 tasks
2. Set up CI/CD when ready
3. Expand to M2 (200+ tasks) or Julius MJ2 (50 tasks)

---

## Phase 2: Quake 1 Engine Tasks (Expert Difficulty)

### Overview

Added Quake 1 (id Software GPL release) as a new engine for expert-level tasks. These tasks are significantly harder than Pygame tasks, requiring deep understanding of:
- 1990s game engine architecture
- BSP rendering and visibility
- Memory management (zone allocator)
- Network prediction
- Low-level audio mixing

### Quake Task Distribution

| ID | Category | Tier | Description |
|----|----------|------|-------------|
| quake-001 | bug-fix | 4 | BSP node child pointer corruption causes missing geometry |
| quake-002 | bug-fix | 5 | Zone memory allocator corruption on fragmentation |
| quake-003 | bug-fix | 5 | Network prediction desync causes player teleportation |
| quake-004 | bug-fix | 4 | Sound mixing overflow causes audio distortion |
| quake-opt-001 | optimization | 4 | Optimize PVS decompression for large maps |
| quake-opt-002 | optimization | 4 | Optimize entity linked list with spatial hash |
| quake-opt-003 | optimization | 5 | Optimize lightmap interpolation for dynamic lights |
| quake-feat-001 | feature | 5 | Implement skeletal animation system |
| quake-feat-002 | feature | 5 | Implement portal-based occlusion culling |
| quake-feat-003 | feature | 5 | Implement procedural terrain generation |

**Total: 10 Quake tasks (all tier 4-5 expert difficulty)**

### Key Differences from Pygame Tasks

1. **Language**: C instead of Python
2. **Complexity**: Large codebase (50k+ lines) vs small games
3. **Architecture**: Systems programming (memory, networking, rendering)
4. **Testing**: Custom test harnesses that mock Quake subsystems
5. **Timeout**: 300-600 seconds (vs 120 for Pygame)

### Quake Source Reference

- **Repository:** https://github.com/id-Software/Quake
- **License:** GPL v2
- **Key directories:**
  - `WinQuake/` - Main engine source
  - `QW/` - QuakeWorld networking

### Task Tier Distribution (Including Quake)

| Category     | Tier 1 | Tier 2 | Tier 3 | Tier 4 | Tier 5 | Total |
|--------------|--------|--------|--------|--------|--------|-------|
| Bug Fix      | 10     | 8      | 5      | 4      | 2      | 29    |
| Feature      | 3      | 5      | 5      | 2      | 3      | 18    |
| Optimization | 0      | 2      | 3      | 2      | 1      | 8     |
| Mini-Game    | 0      | 0      | 3      | 2      | 0      | 5     |
| **Total**    | 13     | 15     | 16     | 10     | 6      | **60**|

---

## Phase 3: Julius Track (Systems Programming / Historical Bugs)

### Overview

A parallel track using [Julius](https://github.com/bvschaik/julius), the open-source Caesar III reimplementation, to test AI capabilities on:
- **C systems programming** in a large legacy codebase (2,700+ commits)
- **Real historical bugs** extracted from git history (SWE-Bench methodology)
- **Memory safety issues** (double frees, OOB, dangling pointers)
- **Complex game simulation** (city builder vs arcade games)

This provides orthogonal signal to the main track—can models work with existing codebases and debug real-world issues?

### Key Differences from Main Track

| Main Track (Pygame/Quake) | Julius Track |
|---------------------------|--------------|
| Python/C synthetic bugs | C historical bugs |
| Simple 2D games / engine code | Complex city simulation |
| Injected bugs | Real git commit reversions |
| Fresh task codebases | 2,700+ commit legacy codebase |
| Game creation/modification | Bug archaeology |

### Asset Requirements

Julius requires original Caesar III assets to run gameplay tests:

| CI Type | Assets | Scope |
|---------|--------|-------|
| Public CI (GitHub Actions) | None | Memory safety, unit tests only |
| Private CI | Caesar III (~$6) | Full test suite |
| Benchmark users | Own copy | Full reproduction |

**Policy:** Assets never committed. ~40% of tasks runnable without assets.

### Bug Categories

#### J1: Memory Safety (ASan/Valgrind testable)
| Bug | Commit | Tier | Assets |
|-----|--------|------|--------|
| Double free in smacker decoder | `f722d9c` | 3 | No |
| Dangling pointer on localized_filename | `6603f5d` | 3 | No |
| Out-of-bounds during invasions | `96c67ee` | 4 | Yes |
| Out-of-bounds when drawing map | `5876540` | 4 | Yes |
| Sheep out-of-bounds destination | `5a37aa8` | 3 | No (mocked) |

#### J2: Crash Fixes
| Bug | Commit | Tier | Assets |
|-----|--------|------|--------|
| Carthago freeze during large invasions | Wiki | 5 | Yes |
| Crash in file save dialog | `24a67bb` | 3 | Yes |
| Water buildings corrupted on collapse | `6c7ddd4` | 4 | Yes |

#### J3: Game Logic
| Bug | Commit | Tier | Assets |
|-----|--------|------|--------|
| Buildings deleted even when cancel pressed | `392b967` | 2 | Yes |
| Clone building ignores disallowed buildings | `2c12e32` | 2 | No (unit) |
| Trees not forming forest on elevation | `2be5a0f` | 3 | Yes |
| Multiple horn sounds on large invasions | `638fc28` | 3 | Yes |
| Hotkey config ordering error | #757 | 2 | No |

#### J4: Visual/UI
| Bug | Commit | Tier | Assets |
|-----|--------|------|--------|
| Off-by-one animation offsets | `a1c1a74` | 2 | Yes |
| Amphitheater spectator offset by 1px | `3791789` | 2 | Yes |
| Tooltip trailing newline | `f75d681` | 1 | No |
| Build menu hitbox misplaced | `39f7328` | 3 | Yes |

### Directory Structure

```
tasks/julius/
├── memory-safety/
│   └── j001-smacker-double-free/
│       ├── task.json           # Metadata, commit ref, difficulty
│       ├── buggy.patch         # Patch to revert fix
│       ├── prompt.md           # Bug report for AI
│       ├── tests/
│       │   ├── test_crash.c    # ASan test (no assets)
│       │   └── Makefile
│       └── solution/
│           └── fix.patch       # Original fix
├── crash-fix/
├── game-logic/
└── visual/

harness/
├── julius_sandbox.py           # Clone Julius, apply patches, build
├── julius_test_runner.py       # Run tests with ASan/Valgrind
└── patch_utils.py              # Git patch manipulation

evaluation/
├── julius_evaluator.py         # Scoring logic for Julius tasks
└── asan_parser.py              # Parse AddressSanitizer output
```

### Evaluation Protocol

```
For each task:
  1. Clone fresh Julius at buggy commit
  2. Apply buggy.patch to revert fix
  3. Verify bug exists (test fails or ASan triggers)
  4. Present code + bug report to AI model
  5. Apply model's proposed fix (as patch)
  6. Run test suite with ASan
  7. Score:
     - Compiles: 1 point
     - No new ASan errors: 1 point
     - Test passes: 2 points
     - Matches original fix structure (bonus): 1 point
```

### Infrastructure Required

#### 3.1 Julius Build Harness
- [x] `harness/julius_sandbox.py`:
  - Clone Julius repo at specific commit
  - Apply/revert patches
  - Build with CMake + ASan flags
  - Manage build cache for performance
- [x] `harness/patch_utils.py`:
  - Parse unified diff format
  - Apply model output as patch
  - Validate patch applies cleanly

#### 3.2 Julius Test Runner
- [x] `evaluation/julius_test_runner.py`:
  - Run compiled test binaries
  - Capture ASan output
  - Parse test results
  - Handle timeout for freeze bugs
- [x] `evaluation/asan_parser.py`:
  - Parse AddressSanitizer reports
  - Categorize error types
  - Extract stack traces

#### 3.3 Julius Evaluator
- [x] `evaluation/julius_evaluator.py`:
  - Scoring logic per evaluation protocol
  - Patch similarity comparison
  - Integration with main benchmark runner

### Milestones

#### MJ1: Proof of Concept (10 tasks) - **Complete** ✅
- [x] Julius build harness working
- [x] ASan-based evaluation operational
- [x] Integration with main benchmark runner
- [x] 10 tasks created (10/10 done)
- [x] All tasks validated (fixed code passes, buggy code fails)
- [x] No assets required for any task
- [x] Baseline results: Claude Haiku 80% (8/10)

**Task list for MJ1:**
| ID | Bug | Commit | Tier | Status |
|----|-----|--------|------|--------|
| julius-001 | Double free in smacker decoder | `f722d9c` | 3 | ✅ Done |
| julius-002 | Dangling pointer localized_filename | `6603f5d` | 3 | ✅ Done |
| julius-003 | Sheep OOB destination | `5a37aa8` | 3 | ✅ Done |
| julius-004 | Tooltip trailing newline | `f75d681` | 1 | ✅ Done |
| julius-005 | Hotkey config ordering | `pr758` | 2 | ✅ Done |
| julius-006 | Clone building validation | `2c12e32` | 2 | ✅ Done |
| julius-007 | Buffer overflow in filename handling | `synthetic` | 3 | ✅ Done |
| julius-008 | Integer overflow in resource calc | `synthetic` | 3 | ✅ Done |
| julius-009 | Null pointer dereference in building | `synthetic` | 2 | ✅ Done |
| julius-010 | Use-after-free in UI callback | `synthetic` | 4 | ✅ Done |

#### MJ2: Core Test Suite (50 tasks)
- [ ] 50 tasks across J1-J3 categories
- [ ] Hybrid CI operational (public + private)
- [ ] Asset-free subset (~20 tasks) publicly runnable
- [ ] Documentation for benchmark users
- [ ] Integration with main benchmark runner

#### MJ3: Full Coverage (100+ tasks)
- [ ] 100+ tasks including visual bugs (J4)
- [ ] Screenshot comparison pipeline
- [ ] Cross-reference with main track metrics
- [ ] Public leaderboard integration

### Implementation Order

1. **Julius Infrastructure** ✅
   - [x] Clone and build Julius locally
   - [x] Verify ASan builds work
   - [x] Create `julius_sandbox.py` with basic clone/build
   - [x] Create `patch_utils.py` for patch manipulation

2. **First Bug Extraction** ✅
   - [x] Identify commit `f722d9c` (smacker double free)
   - [x] Create `buggy.patch` by reversing the fix
   - [x] Write minimal test that triggers the bug
   - [x] Verify test passes on fixed code, fails on buggy

3. **Test Harness** ✅
   - [x] Create `julius_test_runner.py`
   - [x] Create `asan_parser.py`
   - [x] End-to-end test: buggy → model fix → evaluate (tests/evaluation/test_julius_evaluator.py)

4. **MJ1 Tasks** ✅
   - [x] julius-001: Double free in smacker decoder (f722d9c)
   - [x] julius-002: Dangling pointer localized_filename (6603f5d)
   - [x] julius-003: Sheep OOB destination (5a37aa8)
   - [x] julius-004: Tooltip trailing newline (f75d681)
   - [x] julius-005: Hotkey config ordering (pr758)
   - [x] julius-006: Clone building validation (2c12e32)
   - [x] julius-007: Buffer overflow in filename handling (synthetic)
   - [x] julius-008: Integer overflow in resource calc (synthetic)
   - [x] julius-009: Null pointer dereference in building (synthetic)
   - [x] julius-010: Use-after-free in UI callback (synthetic)
   - [x] All tasks work without assets

5. **Integration** ✅
   - [x] Add `engine: julius` to task schema
   - [x] Update `run_benchmark.py` to discover Julius tasks
   - [x] Update `evaluation/runner.py` with Julius dispatch

### Task Tier Distribution (Current: 70 tasks)

| Engine | Category     | Tier 1 | Tier 2 | Tier 3 | Tier 4 | Tier 5 | Total |
|--------|--------------|--------|--------|--------|--------|--------|-------|
| Pygame | Bug Fix      | 10     | 8      | 5      | 2      | 0      | 25    |
| Pygame | Feature      | 3      | 5      | 5      | 2      | 0      | 15    |
| Pygame | Optimization | 0      | 2      | 3      | 0      | 0      | 5     |
| Pygame | Mini-Game    | 0      | 0      | 3      | 2      | 0      | 5     |
| Quake  | Bug Fix      | 0      | 0      | 0      | 2      | 2      | 4     |
| Quake  | Feature      | 0      | 0      | 0      | 0      | 3      | 3     |
| Quake  | Optimization | 0      | 0      | 0      | 2      | 1      | 3     |
| Julius | Memory Safety| 0      | 2      | 5      | 1      | 0      | 8     |
| Julius | Game Logic   | 0      | 2      | 0      | 0      | 0      | 2     |
| **Total** |           | **13** | **19** | **21** | **9**  | **6**  | **70**|

*Julius MJ1 complete: 10 tasks across memory-safety, game-logic, and visual categories.*

### References

- [Julius GitHub](https://github.com/bvschaik/julius)
- [Augustus GitHub](https://github.com/Keriew/augustus) (fork with more fixes)
- [Caesar 3 Bugs Wiki](https://github.com/bvschaik/julius/wiki/Caesar-3-bugs)
- [Improvements Wiki](https://github.com/bvschaik/julius/wiki/Improvements-from-Caesar-3)
- [SWE-Bench](https://www.swebench.com/) - Methodology inspiration

---

## Phase 4: CI/CD & Reporting (Final)

*This phase is intentionally last - to be implemented when ready for GitHub automation.*

### GitHub Actions
- [ ] `.github/workflows/ci.yml`:
  - Validate task JSON schemas
  - Run solution tests
  - Lint code
- [ ] `.github/workflows/benchmark.yml`:
  - Manual/scheduled trigger
  - Run benchmark suite
  - Upload results artifact

### Leaderboard
- [ ] `leaderboard/index.html`: Static results display (placeholder exists)
- [x] `evaluation/report.py`: Generate JSON results
- [ ] GitHub Pages deployment
