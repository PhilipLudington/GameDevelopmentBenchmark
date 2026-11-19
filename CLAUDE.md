# Game Development Benchmark

A SWE-Bench-style benchmark of game-development tasks for AI models: 225 tasks across
Pygame (165), Julius / Caesar III (50) and Quake (10). Design: `DESIGN.md`. Plan:
`PLAN.md`. Debt: `DEBT.md`.

## Public repo

This repo is public on GitHub. Everything committed is published, and so is every
decision record under `docs/decisions/`.

- Nothing private goes into a commit or a decision record: no money, health, other
  people or client terms (`~/.claude/rules/decision-format.md` § Public repos).
- No local secret paths or key files: API keys are read from the environment only, and
  no file that holds them is named.
- The Caesar III game files are never committed.

## Build

Python only; there is no compile step. Set up the virtual environment once:

```bash
python3 -m venv venv
venv/bin/pip install -r requirements.txt
venv/bin/pip install -e .
```

Julius and Quake tasks are C. Their code is built per task inside the sandbox
(`harness/julius_sandbox.py`, `harness/c_sandbox.py`), so a machine running them needs
CMake, SDL2 and a compiler with AddressSanitizer.

## Tests

```bash
venv/bin/pytest tests/
```

`pyproject.toml` sets `testpaths = ["tests"]`. Task validation checks one task folder per
call, so loop over all 225:

```bash
for task in tasks/*/*/*/; do
  venv/bin/python scripts/validate_task.py "$task" --quiet
done
```

The README's loop, `tasks/*/*/*/*/`, is one level too deep and validates nothing. Today
99 of the 225 tasks fail validation (`BUG.md` Bug 1).

## Task layout

Each task is `tasks/<engine>/<category>/<task-id>/`:

- `task.json`: metadata, checked against `schemas/`
- `prompt.md`: what the model is asked to do
- `game/`: the broken or incomplete code the model starts from (Pygame and Quake tasks)
- `buggy.patch`: in place of `game/` in Julius tasks, the change that breaks the Julius
  source the sandbox builds
- `solution/`: the reference solution (`solution/fix.patch` in Julius tasks)
- `tests/`: the tests that score the model's change

Engines and categories:

- `pygame`: `bug-fix`, `feature`, `optimization`, `mini-game`
- `julius`: `memory-safety`, `crash-fix`, `game-logic`, `visual`
- `quake`: `bug-fix`, `feature`, `optimization`

The Pygame baseline games are in `baselines/pygame/`. New tasks start from
`scripts/create_task.py`.

## Model runs

Start a run with one or more `-m <provider:model>` arguments:

```bash
venv/bin/python scripts/run_benchmark.py -m anthropic:<model-id>
venv/bin/python scripts/run_benchmark.py -m mock:solution --engine julius
```

- Filters: `--engine`, `--category`, `--tier`. Results go to `results/runs/<run id>/`.
- Providers: `anthropic`, `openai` (API keys from `ANTHROPIC_API_KEY` and
  `OPENAI_API_KEY`), `claude` (the Claude Code CLI), `ollama`, `llama`, and `mock`.
- After a run, log it and regenerate the leaderboard:

  ```bash
  venv/bin/python scripts/log_run.py results/runs/<run id>
  venv/bin/python scripts/update_leaderboard.py
  ```

  This appends to `results/run-log.json` and rewrites `results/leaderboard.json` and
  `LEADERBOARD.md`.

Paid model runs cost API money and start only when Philip asks for one
(`docs/decisions/2026-10-28-runs-on-demand-on-main-machine.md`). A session's own runs use
the `mock` providers: `mock:solution` answers with each task's reference patch, and
`mock:pass` answers with a fixed reply that holds no patch.
