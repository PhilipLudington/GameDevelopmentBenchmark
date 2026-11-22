# Game Development Benchmark — Implementation Plan

> **Written for Claude.** Precise for a later session, not easy reading. To review it, ask Claude to explain it in plain words and judge the explanation.

## Overview

A SWE-Bench-style benchmark of game-development tasks for AI models (see `DESIGN.md`).
225 tasks ship today: Pygame 165 (eight baseline games), Julius / Caesar III 50, Quake 10.
Four models have full runs on the leaderboard (`LEADERBOARD.md`, Feb 2026).

This plan was converted to the pipeline format on 2026-10-28. The pre-pipeline plan,
with its per-task tables, the Julius bug catalogue and the evaluation protocol, is kept
verbatim in `docs/plan-archive-2026-02.md`; Phases 0–4 below summarise it.

Benchmark runs happen on demand on Philip's main machine, the new Mac mini; there is no
dedicated benchmark machine and no scheduled run
(`docs/decisions/2026-10-28-runs-on-demand-on-main-machine.md`).

Current status: Phase 5 in progress.

**Blocks when:** a defect changes a score published on the leaderboard, or stops a full
benchmark run from completing on the main machine.

**Found issues before the milestone:** Phase 6

## Next Up

- [ ] Task validation — Bug 1: `scripts/validate_task.py` rejects 99 of 225 shipped tasks (Julius patch-only layout and schema nulls, underscore ids, empty Pong solutions) (blocks Phase 5: `run-tests.sh`, whose test gate runs task validation) (found: write CLAUDE.md 2026-11-18, Bug 1)

---

## Phase 0: Core Infrastructure ✅
**Status:** Complete (2026-01)

**Goal:** A harness that loads a task, asks a model for a fix, applies it in a sandbox and scores it.
**Estimated Effort:** 3 weeks (done)

### Deliverables
- Task format and JSON schema (`schemas/`), `scripts/create_task.py`, `scripts/validate_task.py`
- Model interface: API (OpenAI, Anthropic) and CLI (Claude Code, Ollama, llama.cpp) in `models/`
- Evaluation harness: `evaluation/runner.py`, `evaluation/test_runner.py`, `harness/sandbox.py`
- Headless Pygame support and gameplay bots (`harness/pygame_headless.py`, `evaluation/gameplay_bot.py`)

### Tasks
- [x] Project setup, task schema, model interface, evaluation harness, headless Pygame (completed 2026-01)
- [x] Gameplay bot framework: RandomBot, ScriptedBot, PongBot (completed 2026-01)

### Testing Strategy
`pytest tests/` passes; a task runs end to end with `mock:solution`.

---

## Phase 1: Pygame Track ✅
**Status:** Complete (2026-01-25)

**Goal:** 165 Pygame tasks across bug-fix, feature, optimization and mini-game, on eight baseline games.
**Estimated Effort:** 4 weeks (done)

### Deliverables
- Baselines in `baselines/pygame/`: Pong, Snake, Breakout, Space Invaders, Asteroids, Tetris, Platformer, Tower Defense
- 165 tasks in `tasks/pygame/`

### Tasks
- [x] M1: 50 tasks on Pong, Snake, Breakout, Space Invaders (completed 2026-01)
- [x] M2: four more baselines and 115 more tasks (completed 2026-01-25)

### Testing Strategy
Every task validates with `mock:solution` (reference solution passes, broken code fails).

---

## Phase 2: Quake Track ✅
**Status:** Complete (2026-01)

**Goal:** 10 expert (tier 4–5) C tasks on the GPL Quake 1 engine.
**Estimated Effort:** 1 week (done)

### Deliverables
- 10 tasks in `tasks/quake/` (4 bug-fix, 3 feature, 3 optimization)

### Tasks
- [x] Quake bug-fix, feature and optimization tasks with mocked-subsystem test harnesses (completed 2026-01)

### Testing Strategy
Every task validates with `mock:solution`.

---

## Phase 3: Julius Track MJ1–MJ2 ✅
**Status:** Complete (2026-01)

**Goal:** 50 C tasks on Julius (open-source Caesar III), from real historical fixes and synthetic bugs, all runnable without game files.
**Estimated Effort:** 2 weeks (done)

### Deliverables
- `harness/julius_sandbox.py`, `harness/patch_utils.py`, `evaluation/julius_test_runner.py`, `evaluation/asan_parser.py`, `evaluation/julius_evaluator.py`
- 50 tasks in `tasks/julius/` (memory-safety 19, crash-fix 10, game-logic 14, visual 7)

### Tasks
- [x] MJ1: build harness, ASan evaluation, 10 tasks (completed 2026-01)
- [x] MJ2: 50 tasks, synthetic evaluation without game files (completed 2026-01)

### Testing Strategy
Every task validates with `mock:solution`; no task needs Caesar III files.

---

## Phase 4: First Model Runs and Leaderboard ✅
**Status:** Complete (2026-02-14)

**Goal:** Full 225-task runs for the first models, with versions, cost and a generated leaderboard.
**Estimated Effort:** 1 week (done)

### Deliverables
- `scripts/log_run.py` and `results/run-log.json` (persistent run log with model and benchmark versions)
- `scripts/update_leaderboard.py`, `results/leaderboard.json`, `LEADERBOARD.md`
- Runs: Claude Haiku 4.5, Claude Sonnet 4.5, GPT-5 mini, GPT-4.1 nano

### Tasks
- [x] Run logging, model versioning, cost tracking, leaderboard generation (completed 2026-02-14)
- [x] Four full model runs (completed 2026-02-14)

### Testing Strategy
`LEADERBOARD.md` regenerates from `results/run-log.json`.

---

## Phase 5: Pipeline Setup and New Machine

**Goal:** The repo works under the pipeline (CLAUDE.md, gate wrappers, ledgers) and a full benchmark run works on the new Mac mini.
**Estimated Effort:** 2–3 days, once the new Mac mini is in service (due early November 2026)

### Deliverables
- `CLAUDE.md` at the repo root
- `.airtower.json`, `run-tests.sh`, `run-build.sh` (gate wrappers, per `~/.claude/docs/airtower.md`)
- The new Mac mini can run every engine's tasks: Python venv, API keys, C toolchain for Julius and Quake (CMake, SDL2, ASan)

### Tasks
- [x] Convert the planning docs to the pipeline format: this PLAN.md, `DEBT.md`, old plan archived to `docs/plan-archive-2026-02.md` (completed 2026-10-28)
- [x] Rework the plan for the old Mac mini's retirement: drop the dedicated benchmark machine and scheduled runs; runs happen on demand on the new main Mac mini (completed 2026-10-28)
- [x] Write `CLAUDE.md`: build and test commands, task layout, how a model run is started, the public-repo rule (nothing private in commits or decision records)
  It describes the repo as it is when written: raw commands now, and the next task (wrappers) adds the AirTower "use the wrappers" section (rule: `~/.claude/docs/airtower.md` § Claude Code Integration; PLAN.md order)
  Build is the Python install, `pip install -r requirements.txt` then `pip install -e .` in `venv/`; there is no compile step, and Julius and Quake C code is built per task inside the sandbox (code: `README.md` Quick Start; `harness/julius_sandbox.py`)
  Tests are `pytest tests/` (code: `pyproject.toml` `testpaths`); task validation is `scripts/validate_task.py <task dir>`, one folder per call, looped over `tasks/*/*/*/` (225 folders; the README's `tasks/*/*/*/*/` is one level too deep) (code: `scripts/validate_task.py` `main`)
  Task layout is `tasks/<engine>/<category>/<task-id>/` with `task.json`, `prompt.md`, `game/`, `solution/`, `tests/` (code: `tasks/`); baselines in `baselines/pygame/` (code)
  A run is `python scripts/run_benchmark.py -m <provider:model>` with its `--engine`/`--category`/`--tier` filters, then `scripts/log_run.py <run dir>` and `scripts/update_leaderboard.py` (code: those scripts' `main`); keys come from `ANTHROPIC_API_KEY` / `OPENAI_API_KEY` in the environment, and no key file path is named (code: `models/api_model.py`; rule: public repo below)
  Paid model runs start only when Philip asks for one; a session's own runs use `mock:*` providers (`mock:solution`, `mock:pass`) (rule: `docs/decisions/2026-10-28-runs-on-demand-on-main-machine.md`, runs started by hand, unattended spend rejected)
  The public-repo rule covers every committed file and decision record: nothing private (money, health, other people, client terms), no local secret paths, and the Caesar III game files are never committed (rule: `~/.claude/rules/decision-format.md` § Public repos; `gh repo view` reads PUBLIC; Phase 7 deliverables); push and CI rules are left to the global `CLAUDE.md` and Debt 2 (rule: global `CLAUDE.md` § Commits and pushes)
  Verified by: each command in `CLAUDE.md` run as written succeeds (`pytest tests/` collects, a `validate_task.py` loop finds 225 folders, `run_benchmark.py --help` lists the flags named) — answer key: `pyproject.toml`, the scripts' click options and `decision-format.md` § Public repos, none written by this task
  (completed 2026-11-18)
  Reviewed: 2026-11-18 0920e97a (gaps 8, Philip 0)
- [ ] Add `.airtower.json` and the `run-tests.sh` / `run-build.sh` wrappers; the test gate runs `pytest tests/` plus task validation, so it needs no API key
- [ ] Write reference solutions for the 9 Pong tasks with none (`pong-002` to `pong-010`), so `mock:solution` can answer every task; due before the Phase 5 `mock:solution` proof (decided 2026-11-22, `docs/decisions/2026-11-22-task-rules-are-the-truth.md`)
- [ ] Set up the new Mac mini for benchmark runs: venv from `requirements.txt`, API keys for Anthropic and OpenAI, Julius and Quake build dependencies; prove it with one `mock:solution` pass over all 225 tasks

### Testing Strategy
`run-tests.sh` exits green on the new Mac mini; `scripts/run_benchmark.py -m mock:solution` passes all 225 tasks there.

---

## Phase 6: Leaderboard Credibility

**Goal:** Every score on the leaderboard can be trusted, and the leaderboard includes current strong models.
**Estimated Effort:** 1–2 weeks (API spend per full run: a few dollars for small models, more for large ones)

### Deliverables
- A written finding on why Haiku 4.5 outscores Sonnet 4.5 and why Pygame pass rates are low (8–11%), with any harness defect filed and fixed
- Cost shown for every model row, including OpenAI
- Full runs for current flagship and small models from Anthropic and OpenAI
- Regenerated `LEADERBOARD.md`

### Tasks
- [ ] Audit the Feb runs for harness-caused failures: sample failed Pygame tasks for Sonnet 4.5 and Haiku 4.5 and classify each failure (model wrong, output not parsed, patch not applied, timeout, test flake); file each harness defect in `BUG.md`
- [ ] Find why the two OpenAI rows show no cost when GPT-5 mini pricing exists, and fix the cost path in `scripts/update_leaderboard.py` or the run log
- [ ] Re-run GPT-5 mini and GPT-4.1 nano if the audit or the 2026-02-14 GPT-5 parameter fix shows their Feb scores are not valid
- [ ] Choose the current models to add (Anthropic and OpenAI, flagship and small), run each on all 225 tasks, and regenerate `LEADERBOARD.md`
- [ ] Pong gameplay bot that can win against the built-in AI (left open from the original Phase 1.2.2)

### Testing Strategy
Each harness defect found has a regression test; every leaderboard row has a version and a cost; the audit's sample shows no unexplained harness failures.

---

## Phase 7: Julius MJ3 — Full Coverage

**Goal:** 100+ Julius tasks, including visual bugs that need the real game files, run on the new Mac mini.
**Estimated Effort:** 3–4 weeks

### Deliverables
- Caesar III game files installed on the new Mac mini (never committed)
- Screenshot comparison pipeline for visual (J4) tasks
- 50+ more Julius tasks, including the game-file tasks listed in `docs/plan-archive-2026-02.md` § Bug Categories
- Julius results compared with the main track and shown on the leaderboard

### Tasks
- [ ] Install Caesar III game files on the new Mac mini and document the expected path in `CLAUDE.md`; tasks that need them skip cleanly when the files are absent
- [ ] Screenshot comparison pipeline
- [ ] Reach 100+ Julius tasks, including visual (J4) tasks
- [ ] Cross-reference Julius results with the main track metrics
- [ ] Julius breakdown on the public leaderboard

### Testing Strategy
Every new task validates with `mock:solution` on the new Mac mini; game-file tasks skip, not fail, on a machine without the files.

---

## Phase 8: On-Demand Runs and Leaderboard Hosting

**Goal:** One command runs the benchmark on the main machine and publishes the result to a hosted leaderboard.
**Estimated Effort:** 1 week

### Deliverables
- A runner script that takes a model list, runs the full benchmark, logs the run and regenerates the leaderboard
- Results archived under `results/` with the run log
- `leaderboard/index.html` built from `results/leaderboard.json` and hosted on GitHub Pages

### Tasks
- [ ] On-demand runner script that replaces the one-off `run_remaining.sh` / `run_sonnet45.sh` (see `DEBT.md`)
- [ ] Results collection and archival under `results/`
- [ ] Task validation on commit (pre-push hook)
- [ ] `leaderboard/index.html`: static results page (placeholder exists)
- [ ] Host the leaderboard on GitHub Pages

### Testing Strategy
A run started with one command on the new Mac mini ends with an updated `results/run-log.json`, `LEADERBOARD.md` and hosted page.

---

## Phase 9: Hardening & Debt

**Goal:** Clear every deferred bug and every open debt entry.
**Estimated Effort:** depends on the ledgers

### Deliverables
- `BUG.md` with no entry marked Deferred
- `DEBT.md` with no open entry

### Tasks
- [x] Fix every `BUG.md` entry marked Deferred
- [ ] Pay every open `DEBT.md` entry

### Testing Strategy
Both ledgers have no open entries; `run-tests.sh` is green.

---

## Risk Register

| Risk | Impact | Likelihood | Mitigation |
|------|--------|------------|------------|
| Benchmark runs slow down Philip's main machine | Medium | High | Runs are on demand, started when the machine is free |
| Harness defects make published scores wrong | High | Medium | Phase 6 audit before any new model is added |
| API cost of full runs on flagship models | Medium | Medium | Price a run from `results/run-log.json` before starting it |
| Caesar III files unavailable or not redistributable | Medium | Low | Game-file tasks skip cleanly; files never committed |
| Older C toolchain needs (Quake, Julius) break on the new macOS | Medium | Medium | Phase 5 proves all 225 tasks with `mock:solution` first |

## Timeline

Phase 5 → Phase 6 → Phase 7 and Phase 8 (either order) → Phase 9.
Phase 5's machine tasks wait for the new Mac mini.
Beyond this plan, the README Roadmap names Godot and Unity/Unreal tracks; neither is planned yet.
