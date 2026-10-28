# Project Debt

> **Written for Claude.** Precise for a later session, not easy reading. To review it, ask Claude to explain it in plain words and judge the explanation.

## [ ] Debt 1: One-off run scripts from the Feb runs

**Status:** Open
**Kind:** tooling
**Where:** `run_remaining.sh`, `run_sonnet45.sh`
**Due when:** touching `scripts/run_benchmark.py` · before Phase 8's on-demand runner lands

**Description:** Two shell scripts from the Feb 2026 runs on the retired Mac mini sit in the repo root. They hardcode `~/repos/GameDevelopmentBenchmark`, a fixed results folder and a hand-made list of 141 task IDs, so they work only for that one resumed run on that machine.

**Payment:** Fold what they do (source keys, resume a partial run from a list of missing tasks) into `scripts/run_benchmark.py` or Phase 8's runner, then delete both scripts.

**Found by:** planning-doc conversion on main, 2026-10-28 — Philip's call to file it.

---

## [ ] Debt 2: README and DESIGN describe GitHub Actions CI that does not exist

**Status:** Open
**Kind:** documentation
**Where:** `README.md:313`, `DESIGN.md:263`
**Due when:** touching `README.md` · touching `DESIGN.md` · before the leaderboard is hosted (Phase 8)

**Description:** README's CI/CD section names `ci.yml` and `benchmark.yml` workflows, and DESIGN.md plans on the GitHub Actions free tier. There is no `.github/` folder: CI moved to a local Mac mini (commit ca2684e), and that mini has since been retired in favour of on-demand runs on the main machine (`docs/decisions/2026-10-28-runs-on-demand-on-main-machine.md`).

**Payment:** Rewrite both sections to describe on-demand runs on the main machine and the gate wrappers from Phase 5.

**Found by:** planning-doc conversion on main, 2026-10-28.
