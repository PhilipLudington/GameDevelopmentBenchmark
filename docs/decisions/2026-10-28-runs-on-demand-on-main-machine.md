# Benchmark runs happen on demand on the main Mac mini (2026-10-28)

**Decided by:** Philip, 2026-10-28 · **Status:** Decided
**Led to:** PLAN.md Phases 5, 7 and 8; replaces the old plan's "Local Mac Mini CI" phase (`docs/plan-archive-2026-02.md`)

## What prompted it
The old plan's last phase set up a dedicated Mac mini for benchmark runs: scheduled weekly runs, the Caesar III game files installed, and a notice when a run finished. That Mac mini is being retired. A new Mac mini replaces the laptop as Philip's main machine, so there is no longer a spare machine for benchmark runs.

## What we weighed
- **On demand, on the new main machine** — runs start by hand when the machine is free. No scheduler and no completion notices to build. Won.
- **Scheduled overnight runs on the main machine** — keeps the old phase's automation, but runs would compete with daily work and cost API money unattended.
- **An agent keeps running benchmarks on its own schedule** — the Feb runs were made this way on the old mini; it adds a moving part without a dedicated machine to put it on.

## What we decided, and why
Benchmark runs are started by hand on the new Mac mini. The old phase's scheduled runs and completion notices are dropped; the runner script, results archival, commit-time task validation and leaderboard hosting stay (PLAN.md Phase 8). The Caesar III game files will be installed on the new Mac mini, so the Julius full-coverage work (Phase 7) goes ahead.

## What it does not settle
- Which current models join the leaderboard (Phase 6).
- Whether scheduled runs come back if a dedicated machine appears again.
