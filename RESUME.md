# Julius MJ2 Baseline Complete

## Status: COMPLETE

Claude Haiku baseline has been run on all 50 Julius tasks.

## Results Summary

| Metric | Value |
|--------|-------|
| **Total Pass Rate** | **36/50 (72%)** |
| Average Score | 0.86 |
| Run ID | 20260125_004623 |

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

## Comparison with MJ1

| Metric | MJ1 (10 tasks) | MJ2 (50 tasks) |
|--------|----------------|----------------|
| Pass Rate | 80% | 72% |

The 8% decrease reflects the harder synthetic tasks in MJ2.

## Files

- Detailed results: `results/baselines/julius_haiku_baseline.md`
- Raw JSON: `results/runs/20260125_004623/report.json`
- Individual task results: `results/runs/20260125_004623/julius-*_claude_haiku.json`

## Overall Benchmark Status

| Engine | Passed | Total | Rate |
|--------|--------|-------|------|
| Pygame | 18 | 50 | 36% |
| Julius | 36 | 50 | 72% |
| Quake | 4 | 10 | 40% |
| **Total** | **58** | **110** | **53%** |

*See [BASELINE.md](BASELINE.md) for complete per-task breakdown.*

## Next Steps

1. Set up CI/CD when ready
2. Expand to Pygame M2 (200+ tasks)
3. Run additional model baselines (Sonnet, GPT-4, etc.)
