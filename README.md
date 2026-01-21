# Game Development Benchmark

A benchmark suite for evaluating AI model capabilities in game development tasks, inspired by SWE-Bench.

## Overview

This benchmark evaluates AI coding assistants on their ability to:
- **Bug Fix**: Identify and fix bugs in existing game code
- **Feature**: Add new functionality to games
- **Optimization**: Improve game performance
- **Mini-Game**: Create complete mini-games from specifications

Currently supports **Pygame** with 10 initial tasks based on a Pong baseline game.

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

### Difficulty Tiers

| Tier | Difficulty | Scope |
|------|------------|-------|
| 1 | Simple | Single function fixes |
| 2 | Medium | Multi-function changes |
| 3 | Complex | System-level modifications |
| 4 | Expert | Architectural changes |

## Current Tasks

| ID | Category | Tier | Description |
|----|----------|------|-------------|
| pong-001 | bug-fix | 1 | Ball passes through paddle edge |
| pong-002 | bug-fix | 1 | Score displays wrong player |
| pong-003 | bug-fix | 1 | Ball stuck after scoring |
| pong-004 | bug-fix | 2 | AI paddle jitters at boundaries |
| pong-005 | bug-fix | 2 | Ball angle always same |
| pong-006 | feature | 2 | Add pause functionality |
| pong-007 | feature | 2 | Add ball speed increase |
| pong-008 | feature | 3 | Add power-ups |
| pong-009 | optimization | 2 | Reduce CPU usage in main loop |
| pong-010 | mini-game | 3 | Create 4-player variant |

## Project Structure

```
GameDevelopmentBenchmark/
├── tasks/                      # Benchmark tasks
│   └── pygame/
│       ├── bug-fix/            # Bug fix tasks
│       ├── feature/            # Feature tasks
│       ├── optimization/       # Optimization tasks
│       └── mini-game/          # Mini-game tasks
│
├── baselines/                  # Reference game implementations
│   └── pygame/
│       └── pong/               # Pong baseline with tests
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
tasks/pygame/<category>/<task-id>/
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

## Running the Baseline Game

```bash
# Play Pong interactively
python baselines/pygame/pong/main.py

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
for task in tasks/pygame/*/*/; do
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

### Phase 1: Pygame (Current)
- [x] Core infrastructure
- [x] Pong baseline game
- [x] 10 initial tasks
- [ ] Snake baseline + tasks
- [ ] Breakout baseline + tasks
- [ ] Space Invaders baseline + tasks

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
