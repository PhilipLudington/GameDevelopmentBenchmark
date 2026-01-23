#!/usr/bin/env python3
"""Run the full benchmark suite across multiple models."""

import json
import sys
from datetime import datetime
from pathlib import Path

import click

from evaluation.runner import EvaluationRunner
from evaluation.report import ReportGenerator
from harness.sandbox import SandboxConfig
from models.base import create_model, ModelError


def discover_tasks(
    tasks_dir: Path,
    category: str | None = None,
    tier: int | None = None,
    engine: str | None = None,
) -> list[Path]:
    """Discover all tasks in the tasks directory.

    Args:
        tasks_dir: Root tasks directory
        category: Optional category filter
        tier: Optional tier filter
        engine: Optional engine filter (e.g., "pygame", "quake")

    Returns:
        List of task directory paths
    """
    tasks = []

    # Define engines to search
    engines = [engine] if engine else ["pygame", "quake"]
    categories = [category] if category else ["bug-fix", "feature", "optimization", "mini-game"]

    for eng in engines:
        engine_dir = tasks_dir / eng
        if not engine_dir.exists():
            continue

        for cat in categories:
            cat_dir = engine_dir / cat
            if not cat_dir.exists():
                continue

            for task_dir in cat_dir.iterdir():
                if not task_dir.is_dir():
                    continue

                task_json = task_dir / "task.json"
                if not task_json.exists():
                    continue

                # Apply tier filter
                if tier is not None:
                    try:
                        with open(task_json) as f:
                            task_data = json.load(f)
                        if task_data.get("tier") != tier:
                            continue
                    except (json.JSONDecodeError, IOError):
                        continue

                tasks.append(task_dir)

    return sorted(tasks)


def run_benchmark_for_model(
    model_string: str,
    tasks: list[Path],
    output_dir: Path,
    timeout: int,
    verbose: bool,
) -> tuple[int, int]:
    """Run benchmark for a single model.

    Args:
        model_string: Model specification (e.g., "openai:gpt-4")
        tasks: List of task directories
        output_dir: Directory for results
        timeout: Execution timeout
        verbose: Enable verbose output

    Returns:
        Tuple of (passed, failed) counts
    """
    try:
        model = create_model(model_string)
    except (ValueError, ModelError) as e:
        click.echo(click.style(f"Failed to create model {model_string}: {e}", fg="red"))
        return 0, 0

    if not model.is_available():
        click.echo(click.style(f"Model not available: {model_string}", fg="yellow"))
        return 0, 0

    passed = 0
    failed = 0
    sandbox_config = SandboxConfig(timeout=timeout, headless=True)

    for i, task_dir in enumerate(tasks, 1):
        # Read task ID from task.json
        task_json = task_dir / "task.json"
        try:
            with open(task_json) as f:
                task_data = json.load(f)
            task_id = task_data.get("id", task_dir.name)
        except (json.JSONDecodeError, IOError):
            task_id = task_dir.name
        click.echo(f"  [{i}/{len(tasks)}] {task_id}...", nl=False)

        runner = EvaluationRunner(
            task_dir=task_dir,
            model=model,
            sandbox_config=sandbox_config,
            verbose=verbose,
        )

        result = runner.run()

        # Save result
        result_file = output_dir / f"{task_id}_{model_string.replace(':', '_')}.json"
        result_dict = {
            "task_id": result.task_id,
            "model_name": result.model_name,
            "success": result.success,
            "score": result.score,
            "elapsed_time": result.elapsed_time,
            "error": result.error,
            "metadata": result.metadata,
        }
        with open(result_file, "w") as f:
            json.dump(result_dict, f, indent=2)

        if result.success:
            click.echo(click.style(" PASSED", fg="green"))
            passed += 1
        else:
            click.echo(click.style(" FAILED", fg="red"))
            failed += 1

    return passed, failed


@click.command()
@click.option("--model", "-m", multiple=True, required=True,
              help="Model to evaluate (can specify multiple)")
@click.option("--tasks-dir", "-t", type=click.Path(exists=True, path_type=Path),
              default="tasks", help="Tasks directory")
@click.option("--output", "-o", type=click.Path(path_type=Path),
              default="results/runs", help="Output directory")
@click.option("--category", "-c",
              type=click.Choice(["bug-fix", "feature", "optimization", "mini-game"]),
              help="Filter by category")
@click.option("--tier", type=click.IntRange(1, 5), help="Filter by tier (1-5)")
@click.option("--engine", "-e",
              type=click.Choice(["pygame", "quake"]),
              help="Filter by engine type")
@click.option("--timeout", default=120, type=int, help="Default execution timeout per task (overridden by task-specific timeouts)")
@click.option("--verbose", "-v", is_flag=True, help="Verbose output")
@click.option("--report/--no-report", default=True, help="Generate HTML report")
def main(
    model: tuple[str, ...],
    tasks_dir: Path,
    output: Path,
    category: str | None,
    tier: int | None,
    engine: str | None,
    timeout: int,
    verbose: bool,
    report: bool,
):
    """Run the benchmark suite.

    Example:
        python run_benchmark.py -m openai:gpt-4 -m anthropic:claude-3-opus
        python run_benchmark.py -m anthropic:claude-3-opus --engine quake
    """
    # Create output directory
    run_id = datetime.now().strftime("%Y%m%d_%H%M%S")
    output_dir = output / run_id
    output_dir.mkdir(parents=True, exist_ok=True)

    # Discover tasks
    tasks = discover_tasks(tasks_dir, category, tier, engine)

    if not tasks:
        click.echo(click.style("No tasks found", fg="red"))
        sys.exit(1)

    click.echo(f"Found {len(tasks)} tasks")
    click.echo(f"Models: {', '.join(model)}")
    if engine:
        click.echo(f"Engine: {engine}")
    else:
        click.echo("Engines: pygame, quake")
    click.echo(f"Output: {output_dir}")
    click.echo("=" * 60)

    # Run benchmark for each model
    all_results = {}
    for model_string in model:
        click.echo(f"\nModel: {model_string}")
        click.echo("-" * 40)

        passed, failed = run_benchmark_for_model(
            model_string=model_string,
            tasks=tasks,
            output_dir=output_dir,
            timeout=timeout,
            verbose=verbose,
        )

        all_results[model_string] = {"passed": passed, "failed": failed}
        total = passed + failed
        if total > 0:
            click.echo(f"  Results: {passed}/{total} passed ({100*passed/total:.1f}%)")

    # Generate report
    click.echo("\n" + "=" * 60)

    if report and all_results:
        click.echo("Generating report...")
        generator = ReportGenerator(output_dir)
        bench_report = generator.generate_report(run_id)

        # Save JSON report
        json_path = output_dir / "report.json"
        generator.save_report(bench_report, json_path)
        click.echo(f"JSON report: {json_path}")

        # Save HTML report
        html_path = output_dir / "report.html"
        generator.save_html(bench_report, html_path)
        click.echo(f"HTML report: {html_path}")

    # Summary
    click.echo("\nSummary:")
    for model_string, results in all_results.items():
        passed = results["passed"]
        failed = results["failed"]
        total = passed + failed
        if total > 0:
            rate = 100 * passed / total
            status = click.style(f"{rate:.1f}%", fg="green" if rate >= 80 else "yellow" if rate >= 50 else "red")
            click.echo(f"  {model_string}: {passed}/{total} ({status})")

    click.echo(f"\nResults saved to: {output_dir}")


if __name__ == "__main__":
    main()
