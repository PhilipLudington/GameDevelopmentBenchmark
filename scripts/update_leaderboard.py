#!/usr/bin/env python3
"""Aggregate benchmark results into leaderboard files.

Reads all results from results/runs/*/report.json and individual result files,
then generates:
  - results/leaderboard.json  (machine-readable aggregate)
  - LEADERBOARD.md            (human-readable markdown for GitHub)
"""

import json
import sys
from datetime import datetime, timezone
from pathlib import Path

import click

# Pricing per million tokens (input, output) in USD
MODEL_PRICING = {
    # Anthropic
    "claude-sonnet-4-5": (3.0, 15.0),
    "claude-opus-4-5": (15.0, 75.0),
    "claude-opus-4-6": (15.0, 75.0),
    "claude-sonnet-4-20250514": (3.0, 15.0),
    "claude-haiku-3-5": (0.80, 4.0),
    "claude-haiku-4-5": (1.0, 5.0),
    # OpenAI
    "gpt-4o": (2.5, 10.0),
    "gpt-4o-mini": (0.15, 0.60),
    "gpt-4-turbo": (10.0, 30.0),
    "o1": (15.0, 60.0),
    "o1-mini": (3.0, 12.0),
    "o3-mini": (1.10, 4.40),
    "gpt-4.1": (2.0, 8.0),
    "gpt-4.1-mini": (0.40, 1.60),
    "gpt-4.1-nano": (0.10, 0.40),
}


def calculate_cost(model_id: str, input_tokens: int, output_tokens: int) -> float | None:
    """Calculate cost in USD given model and token counts."""
    # Try exact match, then prefix match
    pricing = MODEL_PRICING.get(model_id)
    if not pricing:
        for key in MODEL_PRICING:
            if model_id.startswith(key) or key in model_id:
                pricing = MODEL_PRICING[key]
                break
    if not pricing:
        return None
    input_cost, output_cost = pricing
    return (input_tokens * input_cost + output_tokens * output_cost) / 1_000_000


def load_all_results(runs_dir: Path) -> dict[str, dict]:
    """Load individual result files from all run directories.
    
    Returns dict keyed by model_name, each containing:
      - tasks: dict of task_id -> best result
      - runs: list of run timestamps
      - total_input_tokens: sum of input tokens across all tasks
      - total_output_tokens: sum of output tokens across all tasks
    """
    models: dict[str, dict] = {}
    
    for run_dir in sorted(runs_dir.iterdir()):
        if not run_dir.is_dir() or run_dir.name.startswith("."):
            continue
        
        for result_file in run_dir.glob("*.json"):
            if result_file.name in ("report.json", ".gitkeep"):
                continue
            
            try:
                with open(result_file) as f:
                    result = json.load(f)
            except (json.JSONDecodeError, IOError):
                continue
            
            model_name = result.get("model_name", "unknown")
            task_id = result.get("task_id", "unknown")
            success = result.get("success", False)
            score = result.get("score", 0.0)
            elapsed = result.get("elapsed_time", 0.0)
            usage = result.get("usage", {})
            input_tokens = usage.get("input_tokens", 0) if usage else 0
            output_tokens = usage.get("output_tokens", 0) if usage else 0
            
            version = result.get("model_version") or result.get("metadata", {}).get("model_version")

            benchmark_ver = result.get("benchmark_version")

            if model_name not in models:
                models[model_name] = {
                    "tasks": {}, 
                    "runs": set(), 
                    "run_dir": run_dir.name, 
                    "versions": set(), 
                    "benchmark_versions": set(),
                    "total_input_tokens": 0,
                    "total_output_tokens": 0,
                }
            
            models[model_name]["runs"].add(run_dir.name)
            if version:
                models[model_name]["versions"].add(version)
            if benchmark_ver:
                models[model_name]["benchmark_versions"].add(benchmark_ver)
            
            # Keep best result per task (by success, then score)
            existing = models[model_name]["tasks"].get(task_id)
            if existing is None or (success and not existing["success"]) or (success == existing["success"] and score > existing["score"]):
                # Subtract old token counts if replacing
                if existing:
                    models[model_name]["total_input_tokens"] -= existing.get("input_tokens", 0)
                    models[model_name]["total_output_tokens"] -= existing.get("output_tokens", 0)
                
                models[model_name]["tasks"][task_id] = {
                    "task_id": task_id,
                    "success": success,
                    "score": score,
                    "elapsed_time": elapsed,
                    "run": run_dir.name,
                    "input_tokens": input_tokens,
                    "output_tokens": output_tokens,
                }
                models[model_name]["total_input_tokens"] += input_tokens
                models[model_name]["total_output_tokens"] += output_tokens
    
    return models


def categorize_task(task_id: str, tasks_dir: Path) -> dict:
    """Get category, engine, and tier for a task."""
    # Search for task.json
    for engine in ("pygame", "julius", "quake"):
        for cat_dir in (tasks_dir / engine).iterdir() if (tasks_dir / engine).exists() else []:
            task_dir = cat_dir / task_id
            task_json = task_dir / "task.json"
            if task_json.exists():
                try:
                    with open(task_json) as f:
                        data = json.load(f)
                    return {
                        "engine": engine,
                        "category": data.get("category", cat_dir.name),
                        "tier": data.get("tier", 0),
                    }
                except (json.JSONDecodeError, IOError):
                    pass
    return {"engine": "unknown", "category": "unknown", "tier": 0}


def build_leaderboard(models: dict, tasks_dir: Path) -> dict:
    """Build the leaderboard data structure."""
    entries = []
    
    for model_name, data in models.items():
        tasks = data["tasks"]
        total = len(tasks)
        passed = sum(1 for t in tasks.values() if t["success"])
        failed = total - passed
        pass_rate = (passed / total * 100) if total > 0 else 0
        avg_score = sum(t["score"] for t in tasks.values()) / total if total > 0 else 0
        avg_time = sum(t["elapsed_time"] for t in tasks.values()) / total if total > 0 else 0
        
        # Token usage and cost
        total_input_tokens = data.get("total_input_tokens", 0)
        total_output_tokens = data.get("total_output_tokens", 0)
        total_cost = calculate_cost(model_name, total_input_tokens, total_output_tokens)
        cost_per_task = total_cost / total if total_cost and total > 0 else None
        
        # Breakdown by engine
        by_engine: dict[str, dict] = {}
        by_category: dict[str, dict] = {}
        by_tier: dict[int, dict] = {}
        
        for task_id, result in tasks.items():
            info = categorize_task(task_id, tasks_dir)
            engine = info["engine"]
            category = info["category"]
            tier = info["tier"]
            
            for grouping, key in [(by_engine, engine), (by_category, category), (by_tier, tier)]:
                if key not in grouping:
                    grouping[key] = {"total": 0, "passed": 0}
                grouping[key]["total"] += 1
                if result["success"]:
                    grouping[key]["passed"] += 1
        
        # Calculate rates
        for group in (by_engine, by_category, by_tier):
            for k, v in group.items():
                v["pass_rate"] = round(v["passed"] / v["total"] * 100, 1) if v["total"] > 0 else 0
        
        versions = sorted(data.get("versions", set()))
        bench_versions = sorted(data.get("benchmark_versions", set()))

        entries.append({
            "model": model_name,
            "model_version": versions[0] if len(versions) == 1 else versions if versions else model_name,
            "benchmark_version": bench_versions[-1] if bench_versions else "unknown",
            "total_tasks": total,
            "passed": passed,
            "failed": failed,
            "pass_rate": round(pass_rate, 1),
            "avg_score": round(avg_score * 100, 1),
            "avg_time_seconds": round(avg_time, 2),
            "total_input_tokens": total_input_tokens,
            "total_output_tokens": total_output_tokens,
            "total_cost_usd": round(total_cost, 4) if total_cost else None,
            "cost_per_task_usd": round(cost_per_task, 4) if cost_per_task else None,
            "num_runs": len(data["runs"]),
            "latest_run": max(data["runs"]),
            "by_engine": by_engine,
            "by_category": by_category,
            "by_tier": {str(k): v for k, v in sorted(by_tier.items())},
        })
    
    # Sort by pass rate descending, then avg_score
    entries.sort(key=lambda e: (e["pass_rate"], e["avg_score"]), reverse=True)
    
    # Get latest benchmark version from entries
    bench_vers = set()
    for e in entries:
        bv = e.get("benchmark_version")
        if bv and bv != "unknown":
            bench_vers.add(bv)

    return {
        "generated_at": datetime.now(timezone.utc).isoformat(),
        "benchmark_version": sorted(bench_vers)[-1] if bench_vers else "unknown",
        "total_models": len(entries),
        "total_tasks_available": 225,
        "leaderboard": entries,
    }


def generate_markdown(leaderboard: dict) -> str:
    """Generate LEADERBOARD.md content."""
    lines = [
        "# 🏆 Game Development Benchmark — Leaderboard",
        "",
        f"*Last updated: {leaderboard['generated_at'][:10]} | Benchmark v{leaderboard.get('benchmark_version', 'unknown')}*",
        "",
        "225 tasks across Pygame (165), Julius/Caesar III (50), and Quake (10).",
        "Tasks include bug fixes, features, optimizations, mini-games, memory safety, crash fixes, and more.",
        "",
        "## Overall Rankings",
        "",
        "| Rank | Model | Version | Pass Rate | Passed | Total | Avg Time | Total Cost | $/Task |",
        "|-----:|-------|---------|----------:|-------:|------:|---------:|-----------:|-------:|",
    ]
    
    entries = leaderboard["leaderboard"]
    
    if not entries:
        lines.append("| — | *No results yet* | — | — | — | — | — | — | — |")
    else:
        for i, entry in enumerate(entries, 1):
            medal = {1: "🥇", 2: "🥈", 3: "🥉"}.get(i, str(i))
            version = entry.get("model_version", "—")
            if isinstance(version, list):
                version = ", ".join(version)
            total_cost = entry.get("total_cost_usd")
            cost_per_task = entry.get("cost_per_task_usd")
            cost_str = f"${total_cost:.2f}" if total_cost else "—"
            per_task_str = f"${cost_per_task:.4f}" if cost_per_task else "—"
            lines.append(
                f"| {medal} | **{entry['model']}** "
                f"| {version} "
                f"| {entry['pass_rate']}% "
                f"| {entry['passed']} "
                f"| {entry['total_tasks']} "
                f"| {entry['avg_time_seconds']}s "
                f"| {cost_str} "
                f"| {per_task_str} |"
            )
    
    # Engine breakdown
    if entries:
        lines.extend([
            "",
            "## By Engine",
            "",
            "| Model | Pygame | Julius (C) | Quake (C) |",
            "|-------|-------:|----------:|---------:|",
        ])
        
        for entry in entries:
            engines = entry.get("by_engine", {})
            pygame_rate = f"{engines.get('pygame', {}).get('pass_rate', '—')}%"  if 'pygame' in engines else "—"
            julius_rate = f"{engines.get('julius', {}).get('pass_rate', '—')}%" if 'julius' in engines else "—"
            quake_rate = f"{engines.get('quake', {}).get('pass_rate', '—')}%" if 'quake' in engines else "—"
            lines.append(f"| **{entry['model']}** | {pygame_rate} | {julius_rate} | {quake_rate} |")
        
        # Category breakdown
        lines.extend([
            "",
            "## By Category",
            "",
            "| Model | Bug Fix | Feature | Optimization | Mini-Game | Memory Safety | Crash Fix | Game Logic | Visual |",
            "|-------|--------:|--------:|-------------:|----------:|--------------:|----------:|-----------:|-------:|",
        ])
        
        cats = ["bug-fix", "feature", "optimization", "mini-game", "memory-safety", "crash-fix", "game-logic", "visual"]
        for entry in entries:
            by_cat = entry.get("by_category", {})
            cols = []
            for cat in cats:
                if cat in by_cat:
                    cols.append(f"{by_cat[cat]['pass_rate']}%")
                else:
                    cols.append("—")
            lines.append(f"| **{entry['model']}** | {' | '.join(cols)} |")
    
    lines.extend([
        "",
        "## How to Run",
        "",
        "```bash",
        "# Run benchmark on a model",
        "python scripts/run_benchmark.py -m anthropic:claude-opus-4-6",
        "",
        "# Update this leaderboard after a run",
        "python scripts/update_leaderboard.py",
        "```",
        "",
        "See [README.md](README.md) for full setup instructions.",
        "",
        "---",
        "",
        "*Generated by [GameDevelopmentBenchmark](https://github.com/PhilipLudington/GameDevelopmentBenchmark)*",
        "",
    ])
    
    return "\n".join(lines)


@click.command()
@click.option("--tasks-dir", "-t", type=click.Path(exists=True, path_type=Path),
              default="tasks", help="Tasks directory")
@click.option("--runs-dir", "-r", type=click.Path(exists=True, path_type=Path),
              default="results/runs", help="Results runs directory")
@click.option("--output-json", type=click.Path(path_type=Path),
              default="results/leaderboard.json", help="Output JSON path")
@click.option("--output-md", type=click.Path(path_type=Path),
              default="LEADERBOARD.md", help="Output markdown path")
def main(tasks_dir: Path, runs_dir: Path, output_json: Path, output_md: Path):
    """Aggregate benchmark results into leaderboard files."""
    
    click.echo("Loading results...")
    models = load_all_results(runs_dir)
    
    if not models:
        click.echo("No results found. Run some benchmarks first!")
        click.echo("  python scripts/run_benchmark.py -m mock:pass")
        sys.exit(1)
    
    click.echo(f"Found results for {len(models)} model(s)")
    
    click.echo("Building leaderboard...")
    leaderboard = build_leaderboard(models, tasks_dir)
    
    # Write JSON
    output_json.parent.mkdir(parents=True, exist_ok=True)
    with open(output_json, "w") as f:
        json.dump(leaderboard, f, indent=2)
    click.echo(f"JSON: {output_json}")
    
    # Write Markdown
    md_content = generate_markdown(leaderboard)
    with open(output_md, "w") as f:
        f.write(md_content)
    click.echo(f"Markdown: {output_md}")
    
    # Print summary
    click.echo("\nLeaderboard:")
    for i, entry in enumerate(leaderboard["leaderboard"], 1):
        medal = {1: "🥇", 2: "🥈", 3: "🥉"}.get(i, f"#{i}")
        click.echo(f"  {medal} {entry['model']}: {entry['pass_rate']}% ({entry['passed']}/{entry['total_tasks']})")


if __name__ == "__main__":
    main()
