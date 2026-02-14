#!/usr/bin/env python3
"""Append a benchmark run to the persistent run log.

Called after each benchmark run to record results in results/run-log.json.
Each entry captures the full context: model, date, pass rate, breakdowns,
and a pointer to the raw results directory.

The run log is append-only — never overwritten, never pruned.
"""

import json
import sys
from datetime import datetime, timezone
from pathlib import Path

import click


def load_run_results(run_dir: Path) -> dict:
    """Load all individual result files from a run directory."""
    results = []
    for f in sorted(run_dir.glob("*.json")):
        if f.name in ("report.json", ".gitkeep"):
            continue
        try:
            with open(f) as fh:
                results.append(json.load(fh))
        except (json.JSONDecodeError, IOError):
            continue
    return results


def summarize_run(run_dir: Path) -> dict | None:
    """Build a run summary from result files."""
    results = load_run_results(run_dir)
    if not results:
        return None

    # Group by model
    by_model: dict[str, list] = {}
    for r in results:
        model = r.get("model_name", "unknown")
        by_model.setdefault(model, []).append(r)
    
    # Collect resolved versions per model
    model_versions: dict[str, set] = {}
    for r in results:
        model = r.get("model_name", "unknown")
        version = r.get("model_version") or r.get("metadata", {}).get("model_version")
        if version:
            model_versions.setdefault(model, set()).add(version)

    model_summaries = []
    for model_name, model_results in by_model.items():
        total = len(model_results)
        passed = sum(1 for r in model_results if r.get("success"))
        scores = [r.get("score", 0) for r in model_results]
        times = [r.get("elapsed_time", 0) for r in model_results]

        # Per-task results (compact)
        task_results = []
        for r in model_results:
            task_results.append({
                "task_id": r.get("task_id"),
                "success": r.get("success", False),
                "score": round(r.get("score", 0), 4),
                "time": round(r.get("elapsed_time", 0), 2),
            })

        # Get resolved version(s) for this model
        versions = sorted(model_versions.get(model_name, set()))

        model_summaries.append({
            "model": model_name,
            "model_version": versions[0] if len(versions) == 1 else versions if versions else model_name,
            "total": total,
            "passed": passed,
            "failed": total - passed,
            "pass_rate": round(passed / total * 100, 1) if total > 0 else 0,
            "avg_score": round(sum(scores) / total * 100, 1) if total > 0 else 0,
            "avg_time": round(sum(times) / total, 2) if total > 0 else 0,
            "total_time": round(sum(times), 2),
            "tasks": task_results,
        })

    return {
        "run_id": run_dir.name,
        "timestamp": datetime.now(timezone.utc).isoformat(),
        "run_dir": str(run_dir),
        "models": model_summaries,
    }


@click.command()
@click.argument("run_dir", type=click.Path(exists=True, path_type=Path))
@click.option("--log-file", type=click.Path(path_type=Path),
              default="results/run-log.json", help="Path to run log file")
@click.option("--note", "-n", default=None, help="Optional note for this run")
def main(run_dir: Path, log_file: Path, note: str | None):
    """Append a benchmark run to the persistent run log.
    
    RUN_DIR is the results directory for the run (e.g., results/runs/20260213_185236)
    """
    summary = summarize_run(run_dir)
    if not summary:
        click.echo("No results found in run directory.")
        sys.exit(1)

    if note:
        summary["note"] = note

    # Load existing log
    log_file.parent.mkdir(parents=True, exist_ok=True)
    log: list[dict] = []
    if log_file.exists():
        try:
            with open(log_file) as f:
                log = json.load(f)
        except (json.JSONDecodeError, IOError):
            log = []

    # Append
    log.append(summary)

    # Write
    with open(log_file, "w") as f:
        json.dump(log, f, indent=2)

    # Print summary
    click.echo(f"Logged run {summary['run_id']} to {log_file}")
    for m in summary["models"]:
        click.echo(f"  {m['model']}: {m['passed']}/{m['total']} ({m['pass_rate']}%) in {m['total_time']}s")
    if note:
        click.echo(f"  Note: {note}")


if __name__ == "__main__":
    main()
