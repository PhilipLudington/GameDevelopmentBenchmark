# Game Development Benchmark

A benchmark suite for evaluating AI model capabilities in game development tasks, inspired by SWE-Bench.

## Overview

This benchmark evaluates AI coding assistants on their ability to:
- **Bug Fix**: Identify and fix bugs in existing game code
- **Feature**: Add new functionality to games
- **Optimization**: Improve game performance
- **Mini-Game**: Create complete mini-games from specifications

### Scale

**225 tasks** across **3 game engines**, **8 baseline games**, and **7 task categories**.

| Engine | Language | Tasks | Description |
|--------|----------|------:|-------------|
| **Pygame** | Python | 165 | Classic arcade games — Pong, Snake, Breakout, Asteroids, Space Invaders, Tetris, Platformer, Tower Defense |
| **Julius** | C | 50 | Caesar III open-source city builder — memory safety, game logic, crash fixes, visual rendering |
| **Quake** | C | 10 | Quake engine — bug fixes, features, optimization |

## Task Inventory

### Pygame (165 tasks)

| Game | Bug Fix | Feature | Mini-Game | Optimization | Total |
|------|--------:|--------:|----------:|-------------:|------:|
| Snake | 7 | 14 | 4 | 2 | **27** |
| Breakout | 7 | 14 | 3 | 2 | **26** |
| Space Invaders | 5 | 12 | 3 | 3 | **23** |
| Pong | 6 | 10 | 4 | 2 | **22** |
| Asteroids | 2 | 11 | 2 | 2 | **17** |
| Platformer | 2 | 12 | 2 | 1 | **17** |
| Tower Defense | 2 | 12 | 2 | 1 | **17** |
| Tetris | 3 | 10 | 2 | 1 | **16** |
| **Total** | **34** | **95** | **22** | **14** | **165** |

### Julius — Caesar III (50 tasks)

| Category | Tasks | Description |
|----------|------:|-------------|
| Memory Safety | 19 | Buffer overflows, use-after-free, null pointer dereferences |
| Game Logic | 14 | Simulation bugs, pathfinding, resource calculations |
| Crash Fix | 10 | Segfaults, assertion failures, infinite loops |
| Visual | 7 | Rendering glitches, sprite issues, UI corruption |

### Quake Engine (10 tasks)

| Category | Tasks | Description |
|----------|------:|-------------|
| Bug Fix | 4 | Engine bugs and gameplay issues |
| Feature | 3 | New engine capabilities |
| Optimization | 3 | Performance improvements |

## Quick Start

### Installation

```bash
# Clone the repository
git clone https://github.com/PhilipLudington/GameDevelopmentBenchmark.git
cd GameDevelopmentBenchmark

# Create and activate virtual environment
python3 -m venv venv
source venv/bin/activate  # On Windows: venv\Scripts\activate

# Install dependencies and project
pip install -r requirements.txt
pip install -e .  # Install project in development mode
```

### Running Evaluations

```bash
# Run a single task with a mock model (for testing)
python evaluation/runner.py \
  --task tasks/pygame/bug-fix/pong-001/ \
  --model mock:pass \
  --verbose

# Run with an actual model
python evaluation/runner.py \
  --task tasks/pygame/bug-fix/pong-001/ \
  --model openai:gpt-4

# Run the full benchmark suite
python scripts/run_benchmark.py \
  --model openai:gpt-4 \
  --model anthropic:claude-3-opus \
  --output results/runs/
```

### Environment Variables

For API-based models, set the appropriate API keys:

```bash
export OPENAI_API_KEY="your-key-here"
export ANTHROPIC_API_KEY="your-key-here"
```

## Task Categories & Tiers

### Categories

| Category | Description | Example |
|----------|-------------|---------|
| Bug Fix | Fix broken game functionality | Collision detection not working |
| Feature | Add new game mechanics | Implement pause functionality |
| Optimization | Improve performance | Reduce CPU usage in game loop |
| Mini-Game | Create game variants | Build 4-player Pong |
| Crash Fix | Fix crashes and segfaults (C engines) | Null pointer dereference in save/load |
| Memory Safety | Fix memory bugs (C engines) | Buffer overflow in map parser |
| Game Logic | Fix simulation/AI bugs (C engines) | Pathfinding ignores road networks |
| Visual | Fix rendering issues (C engines) | Sprite z-ordering incorrect |

### Difficulty Tiers

| Tier | Difficulty | Scope |
|------|------------|-------|
| 1 | Simple | Single function fixes |
| 2 | Medium | Multi-function changes |
| 3 | Complex | System-level modifications |
| 4 | Expert | Architectural changes |

## Project Structure

```
GameDevelopmentBenchmark/
├── tasks/                      # Benchmark tasks (225 total)
│   ├── pygame/                 #   165 tasks across 8 games
│   │   ├── bug-fix/
│   │   ├── feature/
│   │   ├── optimization/
│   │   └── mini-game/
│   ├── julius/                 #   50 tasks (Caesar III)
│   │   ├── crash-fix/
│   │   ├── game-logic/
│   │   ├── memory-safety/
│   │   └── visual/
│   └── quake/                  #   10 tasks (Quake engine)
│       ├── bug-fix/
│       ├── feature/
│       └── optimization/
│
├── baselines/                  # Reference game implementations
│   ├── pygame/
│   │   ├── pong/               # Pong baseline with tests
│   │   ├── snake/              # Snake baseline with tests
│   │   ├── breakout/           # Breakout baseline with tests
│   │   ├── asteroids/          # Asteroids baseline with tests
│   │   ├── space_invaders/     # Space Invaders baseline with tests
│   │   ├── tetris/             # Tetris baseline with tests
│   │   ├── platformer/         # Platformer baseline with tests
│   │   └── tower_defense/      # Tower Defense baseline with tests
│   └── quake/                  # Quake engine baseline
│
├── evaluation/                 # Evaluation harness
│   ├── runner.py               # Main evaluation orchestrator
│   ├── test_runner.py          # Pytest executor
│   ├── gameplay_bot.py         # Automated gameplay testing
│   ├── performance.py          # Performance benchmarking
│   └── report.py               # Results & HTML reports
│
├── harness/                    # Execution utilities
│   ├── sandbox.py              # Isolated execution environment
│   ├── pygame_headless.py      # Headless pygame support
│   └── metrics.py              # Performance metrics
│
├── models/                     # AI model interfaces
│   ├── base.py                 # Abstract interface
│   ├── api_model.py            # OpenAI, Anthropic, etc.
│   └── cli_model.py            # Ollama, llama.cpp, mock
│
├── scripts/                    # CLI tools
│   ├── create_task.py          # Task scaffolding
│   ├── validate_task.py        # Task validation
│   └── run_benchmark.py        # Benchmark runner
│
├── schemas/                    # JSON schemas
│   └── task_schema.json        # Task definition schema
│
├── results/runs/               # Evaluation outputs
├── leaderboard/                # Static leaderboard
└── .github/workflows/          # CI/CD pipelines
```

## Model Support

### API Models

```bash
# OpenAI
python evaluation/runner.py --model openai:gpt-4
python evaluation/runner.py --model openai:gpt-3.5-turbo

# Anthropic
python evaluation/runner.py --model anthropic:claude-3-opus-20240229
python evaluation/runner.py --model anthropic:claude-3-sonnet-20240229
```

### CLI Models

```bash
# Claude Code (uses local Claude CLI - no API costs!)
python evaluation/runner.py --model claude:sonnet
python evaluation/runner.py --model claude:opus

# Ollama
python evaluation/runner.py --model ollama:codellama
python evaluation/runner.py --model ollama:deepseek-coder

# Mock (for testing)
python evaluation/runner.py --model mock:pass
python evaluation/runner.py --model mock:echo
```

## Creating New Tasks

### Using the Task Creator

```bash
python scripts/create_task.py \
  --category bug-fix \
  --tier 2 \
  --baseline pong \
  --id "my-new-task" \
  --name "Description of the bug" \
  --tag collision \
  --tag physics
```

### Task Structure

Each task requires:

```
tasks/<engine>/<category>/<task-id>/
├── task.json       # Task metadata (required)
├── prompt.md       # Problem description for AI (required)
├── game/           # Broken game code (required)
│   └── main.py
├── solution/       # Reference solution (optional)
│   └── main.py
└── tests/          # Automated tests (recommended)
    └── test_*.py
```

### Task JSON Schema

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
  "baseline": "pong",
  "timeout": 60
}
```

### Validating Tasks

```bash
python scripts/validate_task.py tasks/pygame/bug-fix/my-new-task/

# Strict mode (treat warnings as errors)
python scripts/validate_task.py tasks/pygame/bug-fix/my-new-task/ --strict
```

## Evaluation Methods

Tasks can specify multiple evaluation methods:

| Method | Description |
|--------|-------------|
| `unit-test` | Run pytest unit tests |
| `integration-test` | Run integration tests |
| `gameplay` | Automated gameplay with bots |
| `performance` | FPS, memory, CPU benchmarks |
| `visual` | Screenshot comparison |

## Running Baseline Games

```bash
# Play Pong interactively
python baselines/pygame/pong/main.py

# Play any baseline game
python baselines/pygame/snake/main.py
python baselines/pygame/breakout/main.py
python baselines/pygame/asteroids/main.py
python baselines/pygame/space_invaders/main.py
python baselines/pygame/tetris/main.py
python baselines/pygame/platformer/main.py
python baselines/pygame/tower_defense/main.py

# Run in headless mode (for testing)
python baselines/pygame/pong/main.py --headless --frames 1000

# Run baseline tests
pytest baselines/pygame/pong/tests/ -v
```

## CI/CD

### GitHub Actions Workflows

- **ci.yml**: Validates task schemas, runs tests, lints code on PRs
- **benchmark.yml**: Runs scheduled benchmarks and updates leaderboard

### Running Locally

```bash
# Validate all tasks
for task in tasks/*/*/*/*/; do
  python scripts/validate_task.py "$task" --quiet
done

# Run all tests
pytest tests/ -v
pytest baselines/pygame/pong/tests/ -v
```

## Benchmark Results

Results are saved as JSON files in `results/runs/`:

```bash
# View latest results
cat results/runs/*/report.json | jq .

# Generate HTML report
python scripts/run_benchmark.py -m mock:pass --report
open results/runs/*/report.html
```

## Roadmap

### Phase 1: Pygame ✅
- [x] Core infrastructure
- [x] Pong baseline + 22 tasks
- [x] Snake baseline + 27 tasks
- [x] Breakout baseline + 26 tasks
- [x] Asteroids baseline + 17 tasks
- [x] Space Invaders baseline + 23 tasks
- [x] Tetris baseline + 16 tasks
- [x] Platformer baseline + 17 tasks
- [x] Tower Defense baseline + 17 tasks

### Phase 1.5: C Game Engines ✅
- [x] Julius (Caesar III) — 50 tasks
- [x] Quake engine — 10 tasks

### Phase 2: Godot
- [ ] Godot project templates
- [ ] GDScript task support
- [ ] Scene/node manipulation tasks

### Phase 3: Unity/Unreal
- [ ] Unity C# support
- [ ] Unreal Blueprint support

## Contributing

1. Fork the repository
2. Create a feature branch
3. Add tasks or improvements
4. Run tests and validation
5. Submit a pull request

See [CONTRIBUTING.md](CONTRIBUTING.md) for detailed guidelines.

## License

MIT License - see [LICENSE](LICENSE) for details.

## Acknowledgments

Inspired by [SWE-Bench](https://github.com/princeton-nlp/SWE-bench) for software engineering evaluation.
