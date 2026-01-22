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

### Phase 1.5: CI/CD & Reporting

#### GitHub Actions
- [ ] `.github/workflows/ci.yml`:
  - Validate task JSON schemas
  - Run solution tests
  - Lint code
- [ ] `.github/workflows/benchmark.yml`:
  - Manual/scheduled trigger
  - Run benchmark suite
  - Upload results artifact

#### Leaderboard
- [ ] `leaderboard/index.html`: Static results display
- [x] `evaluation/report.py`: Generate JSON results
- [ ] GitHub Pages deployment

### Phase 1.6: Expand to M1 (50 tasks)

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

**Status: M1 Complete (50 tasks achieved!)**

- Core infrastructure fully operational
- **50 tasks created and validated:**
  - 10 Pong tasks
  - 15 Snake tasks (baseline game + full task suite)
  - 15 Breakout tasks (baseline game + full task suite)
  - 10 Space Invaders tasks (baseline game + higher difficulty tasks)
- Four baseline games fully implemented:
  - Pong: with test coverage
  - Snake: 43 unit tests
  - Breakout: 52 unit tests
  - Space Invaders: 62 unit tests
- Evaluation harness working with multiple model providers:
  - API: OpenAI, Anthropic
  - CLI: Ollama, llama.cpp, Claude Code
- pong-001 prompt improved with clearer guidance

**Completed:**
1. ✅ Improved pong-001 prompt to help models succeed (added detailed fix instructions)
2. ✅ Created Snake baseline game with full test coverage
3. ✅ Created all 15 Snake tasks:
   - 6 bug-fix tasks (5 tier 1, 1 tier 2)
   - 7 feature tasks (3 tier 1, 3 tier 2, 1 tier 3)
   - 1 optimization task (tier 2)
   - 1 mini-game task (tier 3)
4. ✅ Created Breakout baseline game with 52 unit tests
5. ✅ Created all 15 Breakout tasks:
   - 6 bug-fix tasks (5 tier 1, 1 tier 2)
   - 7 feature tasks (2 tier 1, 3 tier 2, 2 tier 3)
   - 1 optimization task (tier 2)
   - 1 mini-game task (tier 3)
6. ✅ Created Space Invaders baseline game with 62 unit tests
7. ✅ Created all 10 Space Invaders tasks (higher difficulty as requested):
   - 4 bug-fix tasks (0 tier 1, 3 tier 2, 1 tier 3) - more challenging bugs
   - 4 feature tasks (0 tier 1, 0 tier 2, 2 tier 3, 2 tier 4) - complex features
   - 1 optimization task (tier 3) - spatial partitioning
   - 1 mini-game task (tier 4) - two-player cooperative

**Next Steps:**
1. Set up CI/CD when ready
2. Run model baselines to validate tasks
3. Expand to M2 (200+ tasks) with additional game engines
