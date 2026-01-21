"""Results aggregation and report generation."""

import json
from dataclasses import dataclass, field, asdict
from datetime import datetime
from pathlib import Path
from typing import Any

from jinja2 import Template


@dataclass
class ModelScore:
    """Aggregate scores for a model."""

    model_name: str
    total_tasks: int
    passed: int
    failed: int
    pass_rate: float
    avg_score: float
    by_category: dict[str, dict[str, float]] = field(default_factory=dict)
    by_tier: dict[int, dict[str, float]] = field(default_factory=dict)


@dataclass
class BenchmarkReport:
    """Complete benchmark run report."""

    run_id: str
    timestamp: str
    total_tasks: int
    models: list[ModelScore]
    task_results: list[dict[str, Any]]
    metadata: dict[str, Any] = field(default_factory=dict)


class ReportGenerator:
    """Generates reports from evaluation results."""

    def __init__(self, results_dir: Path):
        """Initialize the report generator.

        Args:
            results_dir: Directory containing result JSON files
        """
        self.results_dir = Path(results_dir)

    def load_results(self) -> list[dict[str, Any]]:
        """Load all result files from the results directory.

        Returns:
            List of result dictionaries
        """
        results = []
        for result_file in self.results_dir.glob("*.json"):
            if result_file.name.startswith("."):
                continue
            try:
                with open(result_file) as f:
                    results.append(json.load(f))
            except (json.JSONDecodeError, IOError):
                continue
        return results

    def aggregate_by_model(self, results: list[dict[str, Any]]) -> dict[str, list[dict]]:
        """Group results by model.

        Args:
            results: List of result dictionaries

        Returns:
            Dictionary mapping model names to their results
        """
        by_model: dict[str, list[dict]] = {}
        for result in results:
            model = result.get("model_name", "unknown")
            if model not in by_model:
                by_model[model] = []
            by_model[model].append(result)
        return by_model

    def calculate_model_score(self, model_name: str, results: list[dict]) -> ModelScore:
        """Calculate aggregate scores for a model.

        Args:
            model_name: Name of the model
            results: Results for this model

        Returns:
            ModelScore with aggregated statistics
        """
        total = len(results)
        passed = sum(1 for r in results if r.get("success", False))
        failed = total - passed

        scores = [r.get("score", 0.0) for r in results]
        avg_score = sum(scores) / len(scores) if scores else 0.0

        # Aggregate by category
        by_category: dict[str, dict[str, Any]] = {}
        for result in results:
            category = result.get("metadata", {}).get("task_category", "unknown")
            if category not in by_category:
                by_category[category] = {"total": 0, "passed": 0, "scores": []}
            by_category[category]["total"] += 1
            if result.get("success"):
                by_category[category]["passed"] += 1
            by_category[category]["scores"].append(result.get("score", 0.0))

        category_stats = {}
        for cat, data in by_category.items():
            category_stats[cat] = {
                "total": data["total"],
                "passed": data["passed"],
                "pass_rate": data["passed"] / data["total"] if data["total"] > 0 else 0,
                "avg_score": sum(data["scores"]) / len(data["scores"]) if data["scores"] else 0,
            }

        # Aggregate by tier
        by_tier: dict[int, dict[str, Any]] = {}
        for result in results:
            tier = result.get("metadata", {}).get("task_tier", 0)
            if tier not in by_tier:
                by_tier[tier] = {"total": 0, "passed": 0, "scores": []}
            by_tier[tier]["total"] += 1
            if result.get("success"):
                by_tier[tier]["passed"] += 1
            by_tier[tier]["scores"].append(result.get("score", 0.0))

        tier_stats = {}
        for tier, data in by_tier.items():
            tier_stats[tier] = {
                "total": data["total"],
                "passed": data["passed"],
                "pass_rate": data["passed"] / data["total"] if data["total"] > 0 else 0,
                "avg_score": sum(data["scores"]) / len(data["scores"]) if data["scores"] else 0,
            }

        return ModelScore(
            model_name=model_name,
            total_tasks=total,
            passed=passed,
            failed=failed,
            pass_rate=passed / total if total > 0 else 0,
            avg_score=avg_score,
            by_category=category_stats,
            by_tier=tier_stats,
        )

    def generate_report(self, run_id: str | None = None) -> BenchmarkReport:
        """Generate a complete benchmark report.

        Args:
            run_id: Optional run identifier

        Returns:
            BenchmarkReport with all aggregated data
        """
        results = self.load_results()
        by_model = self.aggregate_by_model(results)

        model_scores = [
            self.calculate_model_score(model, model_results)
            for model, model_results in by_model.items()
        ]

        # Sort by pass rate, then by average score
        model_scores.sort(key=lambda x: (x.pass_rate, x.avg_score), reverse=True)

        return BenchmarkReport(
            run_id=run_id or datetime.now().strftime("%Y%m%d_%H%M%S"),
            timestamp=datetime.now().isoformat(),
            total_tasks=len(results),
            models=model_scores,
            task_results=results,
            metadata={
                "results_dir": str(self.results_dir),
                "num_models": len(model_scores),
            },
        )

    def save_report(self, report: BenchmarkReport, output_path: Path) -> None:
        """Save report to JSON file.

        Args:
            report: Report to save
            output_path: Output file path
        """
        report_dict = {
            "run_id": report.run_id,
            "timestamp": report.timestamp,
            "total_tasks": report.total_tasks,
            "models": [asdict(m) for m in report.models],
            "metadata": report.metadata,
        }

        with open(output_path, "w") as f:
            json.dump(report_dict, f, indent=2)

    def generate_html(self, report: BenchmarkReport) -> str:
        """Generate HTML report.

        Args:
            report: Report to render

        Returns:
            HTML string
        """
        template = Template(HTML_TEMPLATE)
        return template.render(report=report)

    def save_html(self, report: BenchmarkReport, output_path: Path) -> None:
        """Save HTML report.

        Args:
            report: Report to render
            output_path: Output file path
        """
        html = self.generate_html(report)
        with open(output_path, "w") as f:
            f.write(html)


HTML_TEMPLATE = """<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Game Development Benchmark - {{ report.run_id }}</title>
    <style>
        :root {
            --bg-color: #1a1a2e;
            --card-bg: #16213e;
            --text-color: #e4e4e4;
            --accent: #0f3460;
            --success: #4ade80;
            --failure: #f87171;
            --border: #2a2a4a;
        }
        * { box-sizing: border-box; margin: 0; padding: 0; }
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            background: var(--bg-color);
            color: var(--text-color);
            line-height: 1.6;
            padding: 2rem;
        }
        .container { max-width: 1200px; margin: 0 auto; }
        h1 { font-size: 2rem; margin-bottom: 0.5rem; }
        .subtitle { color: #888; margin-bottom: 2rem; }
        .card {
            background: var(--card-bg);
            border-radius: 8px;
            padding: 1.5rem;
            margin-bottom: 1.5rem;
            border: 1px solid var(--border);
        }
        .card h2 { font-size: 1.25rem; margin-bottom: 1rem; }
        table {
            width: 100%;
            border-collapse: collapse;
        }
        th, td {
            padding: 0.75rem;
            text-align: left;
            border-bottom: 1px solid var(--border);
        }
        th { font-weight: 600; color: #aaa; }
        .pass-rate {
            font-weight: bold;
        }
        .pass-rate.high { color: var(--success); }
        .pass-rate.medium { color: #fbbf24; }
        .pass-rate.low { color: var(--failure); }
        .badge {
            display: inline-block;
            padding: 0.25rem 0.5rem;
            border-radius: 4px;
            font-size: 0.75rem;
            font-weight: 600;
        }
        .badge-success { background: rgba(74, 222, 128, 0.2); color: var(--success); }
        .badge-failure { background: rgba(248, 113, 113, 0.2); color: var(--failure); }
    </style>
</head>
<body>
    <div class="container">
        <h1>Game Development Benchmark</h1>
        <p class="subtitle">Run ID: {{ report.run_id }} | {{ report.timestamp }}</p>

        <div class="card">
            <h2>Leaderboard</h2>
            <table>
                <thead>
                    <tr>
                        <th>Rank</th>
                        <th>Model</th>
                        <th>Pass Rate</th>
                        <th>Avg Score</th>
                        <th>Passed</th>
                        <th>Failed</th>
                    </tr>
                </thead>
                <tbody>
                    {% for model in report.models %}
                    <tr>
                        <td>{{ loop.index }}</td>
                        <td>{{ model.model_name }}</td>
                        <td class="pass-rate {% if model.pass_rate >= 0.8 %}high{% elif model.pass_rate >= 0.5 %}medium{% else %}low{% endif %}">
                            {{ "%.1f"|format(model.pass_rate * 100) }}%
                        </td>
                        <td>{{ "%.2f"|format(model.avg_score * 100) }}%</td>
                        <td><span class="badge badge-success">{{ model.passed }}</span></td>
                        <td><span class="badge badge-failure">{{ model.failed }}</span></td>
                    </tr>
                    {% endfor %}
                </tbody>
            </table>
        </div>

        {% for model in report.models %}
        <div class="card">
            <h2>{{ model.model_name }} - By Category</h2>
            <table>
                <thead>
                    <tr>
                        <th>Category</th>
                        <th>Pass Rate</th>
                        <th>Avg Score</th>
                        <th>Total</th>
                    </tr>
                </thead>
                <tbody>
                    {% for cat, stats in model.by_category.items() %}
                    <tr>
                        <td>{{ cat }}</td>
                        <td class="pass-rate {% if stats.pass_rate >= 0.8 %}high{% elif stats.pass_rate >= 0.5 %}medium{% else %}low{% endif %}">
                            {{ "%.1f"|format(stats.pass_rate * 100) }}%
                        </td>
                        <td>{{ "%.2f"|format(stats.avg_score * 100) }}%</td>
                        <td>{{ stats.total }}</td>
                    </tr>
                    {% endfor %}
                </tbody>
            </table>
        </div>
        {% endfor %}

        <footer style="text-align: center; color: #666; margin-top: 2rem;">
            Generated by Game Development Benchmark
        </footer>
    </div>
</body>
</html>
"""
