#!/usr/bin/env python3
"""Main evaluation harness for running benchmark tasks."""

import json
import re
import time
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any

import click

from evaluation.test_runner import TestRunner, TestResult
from harness.sandbox import Sandbox, SandboxConfig, SandboxResult
from models.base import ModelInterface, GenerationResult, create_model


@dataclass
class TaskConfig:
    """Configuration loaded from task.json."""

    id: str
    name: str
    category: str
    tier: int
    engine: str
    description: str
    evaluation: list[str]
    tags: list[str] = field(default_factory=list)
    baseline: str | None = None
    timeout: int = 60
    files_to_modify: list[str] = field(default_factory=list)
    hints: list[str] = field(default_factory=list)


@dataclass
class EvaluationResult:
    """Result of evaluating a task."""

    task_id: str
    model_name: str
    success: bool
    score: float  # 0.0 to 1.0
    test_results: TestResult | None = None
    gameplay_results: dict[str, Any] | None = None
    performance_results: dict[str, Any] | None = None
    model_response: str = ""
    applied_changes: dict[str, str] = field(default_factory=dict)
    elapsed_time: float = 0.0
    error: str | None = None
    metadata: dict[str, Any] = field(default_factory=dict)


class EvaluationRunner:
    """Orchestrates the evaluation of benchmark tasks.

    The evaluation process:
    1. Load task configuration and prompt
    2. Set up sandbox with broken game code
    3. Invoke model with prompt
    4. Parse and apply model's code changes
    5. Run evaluation phases (tests, gameplay, performance)
    6. Aggregate and return results
    """

    def __init__(
        self,
        task_dir: Path,
        model: ModelInterface,
        sandbox_config: SandboxConfig | None = None,
        verbose: bool = False,
    ):
        """Initialize the evaluation runner.

        Args:
            task_dir: Path to the task directory
            model: Model interface to use
            sandbox_config: Optional sandbox configuration
            verbose: Enable verbose output
        """
        self.task_dir = Path(task_dir)
        self.model = model
        self.sandbox_config = sandbox_config or SandboxConfig()
        self.verbose = verbose
        self.task_config: TaskConfig | None = None

    def log(self, message: str) -> None:
        """Log a message if verbose mode is enabled."""
        if self.verbose:
            click.echo(message)

    def load_task(self) -> TaskConfig:
        """Load the task configuration.

        Returns:
            TaskConfig from task.json

        Raises:
            FileNotFoundError: If task.json doesn't exist
            ValueError: If task.json is invalid
        """
        task_json_path = self.task_dir / "task.json"
        if not task_json_path.exists():
            raise FileNotFoundError(f"task.json not found in {self.task_dir}")

        with open(task_json_path) as f:
            data = json.load(f)

        self.task_config = TaskConfig(
            id=data["id"],
            name=data["name"],
            category=data["category"],
            tier=data["tier"],
            engine=data["engine"],
            description=data["description"],
            evaluation=data["evaluation"],
            tags=data.get("tags", []),
            baseline=data.get("baseline"),
            timeout=data.get("timeout", 60),
            files_to_modify=data.get("files_to_modify", []),
            hints=data.get("hints", []),
        )

        return self.task_config

    def load_prompt(self) -> str:
        """Load the task prompt.

        Returns:
            Contents of prompt.md

        Raises:
            FileNotFoundError: If prompt.md doesn't exist
        """
        prompt_path = self.task_dir / "prompt.md"
        if not prompt_path.exists():
            raise FileNotFoundError(f"prompt.md not found in {self.task_dir}")

        return prompt_path.read_text()

    def build_context(self, sandbox: Sandbox) -> dict[str, Any]:
        """Build context for the model including file contents.

        Args:
            sandbox: Sandbox with game files

        Returns:
            Context dictionary for model generation
        """
        context: dict[str, Any] = {
            "system": (
                "You are an expert game developer fixing bugs in pygame games.\n\n"
                "CRITICAL: You must output the COMPLETE modified file, not just the changed function. "
                "The code block must contain the entire file from start to finish, including all imports, "
                "classes, and functions - with your fix applied.\n\n"
                "Format your response as:\n"
                "```python main.py\n"
                "<complete file contents here>\n"
                "```"
            ),
            "files": {},
        }

        # Read game files from sandbox
        game_dir = sandbox.game_dir
        if game_dir and game_dir.exists():
            for py_file in game_dir.rglob("*.py"):
                rel_path = py_file.relative_to(game_dir)
                context["files"][str(rel_path)] = py_file.read_text()

        return context

    def parse_code_blocks(self, response: str) -> dict[str, str]:
        """Parse code blocks from model response.

        Extracts Python code blocks and attempts to determine which file
        they belong to.

        Args:
            response: Model's response text

        Returns:
            Dictionary mapping filenames to code contents
        """
        changes: dict[str, str] = {}

        # Pattern for fenced code blocks with optional filename
        # Matches: ```python filename.py or ```python or ```
        pattern = r"```(?:python)?\s*(?:([^\n`]+\.py))?\n(.*?)```"
        matches = re.findall(pattern, response, re.DOTALL | re.IGNORECASE)

        for filename, code in matches:
            filename = filename.strip() if filename else ""

            # Try to extract filename from code comments
            if not filename:
                first_line = code.strip().split("\n")[0] if code else ""
                if first_line.startswith("#") and ".py" in first_line:
                    # Extract filename from comment like "# main.py" or "# File: main.py"
                    match = re.search(r"(?:file:?\s*)?(\w+\.py)", first_line, re.IGNORECASE)
                    if match:
                        filename = match.group(1)

            # Default to main.py if no filename found
            if not filename:
                filename = "main.py"

            changes[filename] = code.strip()

        return changes

    def run_evaluation_phases(self, sandbox: Sandbox) -> dict[str, Any]:
        """Run all configured evaluation phases.

        Args:
            sandbox: Sandbox with modified game code

        Returns:
            Dictionary with results from each phase
        """
        results: dict[str, Any] = {
            "test": None,
            "gameplay": None,
            "performance": None,
        }

        if not self.task_config:
            return results

        evaluation_methods = self.task_config.evaluation

        # Run unit/integration tests
        if "unit-test" in evaluation_methods or "integration-test" in evaluation_methods:
            self.log("Running tests...")
            test_runner = TestRunner(sandbox)

            # Copy test files to sandbox
            tests_dir = self.task_dir / "tests"
            if tests_dir.exists():
                import shutil
                sandbox_tests = sandbox.working_dir / "tests" if sandbox.working_dir else None
                if sandbox_tests:
                    if sandbox_tests.exists():
                        shutil.rmtree(sandbox_tests)
                    shutil.copytree(tests_dir, sandbox_tests)

            test_result = test_runner.run()
            results["test"] = {
                "passed": test_result.passed,
                "failed": test_result.failed,
                "errors": test_result.errors,
                "skipped": test_result.skipped,
                "total": test_result.total,
                "success": test_result.success,
                "output": test_result.output,
            }

        # Run gameplay evaluation
        if "gameplay" in evaluation_methods:
            self.log("Running gameplay evaluation...")
            # TODO: Implement gameplay bot evaluation
            results["gameplay"] = {"status": "not_implemented"}

        # Run performance evaluation
        if "performance" in evaluation_methods:
            self.log("Running performance evaluation...")
            # TODO: Implement performance benchmarking
            results["performance"] = {"status": "not_implemented"}

        return results

    def calculate_score(self, eval_results: dict[str, Any]) -> float:
        """Calculate overall score from evaluation results.

        Args:
            eval_results: Results from evaluation phases

        Returns:
            Score from 0.0 to 1.0
        """
        scores = []

        # Test score
        test_results = eval_results.get("test")
        if test_results:
            total = test_results.get("total", 0)
            passed = test_results.get("passed", 0)
            if total > 0:
                scores.append(passed / total)

        # Gameplay score (placeholder)
        gameplay = eval_results.get("gameplay")
        if gameplay and gameplay.get("status") != "not_implemented":
            if gameplay.get("success"):
                scores.append(1.0)
            else:
                scores.append(0.0)

        # Performance score (placeholder)
        performance = eval_results.get("performance")
        if performance and performance.get("status") != "not_implemented":
            if performance.get("meets_target"):
                scores.append(1.0)
            else:
                scores.append(0.0)

        if not scores:
            return 0.0

        return sum(scores) / len(scores)

    def run(self) -> EvaluationResult:
        """Run the full evaluation pipeline.

        Returns:
            EvaluationResult with all evaluation data
        """
        start_time = time.time()

        try:
            # Load task configuration
            self.log(f"Loading task from {self.task_dir}")
            task_config = self.load_task()
            prompt = self.load_prompt()

            # Set up sandbox
            self.log("Setting up sandbox...")
            game_dir = self.task_dir / "game"
            with Sandbox(self.sandbox_config) as sandbox:
                sandbox.setup(game_dir)

                # Build context with file contents
                context = self.build_context(sandbox)

                # Update model timeout to use task-specific timeout
                if task_config.timeout:
                    self.model.config.timeout = task_config.timeout

                # Generate response from model
                self.log(f"Invoking model: {self.model.get_name()} (timeout: {self.model.config.timeout}s)")
                model_result = self.model.generate(prompt, context)

                # Parse code changes from response
                self.log("Parsing model response...")
                changes = self.parse_code_blocks(model_result.content)

                if not changes:
                    self.log("Warning: No code blocks found in response")
                else:
                    for filename in changes:
                        self.log(f"  - {filename} ({len(changes[filename])} chars)")

                # Apply changes to sandbox
                self.log(f"Applying {len(changes)} file changes...")
                sandbox.apply_changes(changes)

                # Run evaluation phases
                eval_results = self.run_evaluation_phases(sandbox)

                # Calculate score
                score = self.calculate_score(eval_results)

                elapsed = time.time() - start_time

                # Determine success based on score threshold
                success = score >= 0.95  # Require near-perfect score

                return EvaluationResult(
                    task_id=task_config.id,
                    model_name=self.model.get_name(),
                    success=success,
                    score=score,
                    test_results=TestResult(**eval_results["test"]) if eval_results.get("test") else None,
                    gameplay_results=eval_results.get("gameplay"),
                    performance_results=eval_results.get("performance"),
                    model_response=model_result.content,
                    applied_changes=changes,
                    elapsed_time=elapsed,
                    metadata={
                        "model_config": self.model.get_config(),
                        "task_tier": task_config.tier,
                        "task_category": task_config.category,
                    },
                )

        except Exception as e:
            elapsed = time.time() - start_time
            # Include metadata even on error so reports can categorize failures
            metadata = {}
            if self.task_config:
                metadata = {
                    "model_config": self.model.get_config(),
                    "task_tier": self.task_config.tier,
                    "task_category": self.task_config.category,
                }
            return EvaluationResult(
                task_id=self.task_config.id if self.task_config else "unknown",
                model_name=self.model.get_name(),
                success=False,
                score=0.0,
                elapsed_time=elapsed,
                error=str(e),
                metadata=metadata,
            )


@click.command()
@click.option("--task", "-t", required=True, type=click.Path(exists=True, path_type=Path),
              help="Path to task directory")
@click.option("--model", "-m", required=True, help="Model to use (e.g., openai:gpt-4)")
@click.option("--output", "-o", type=click.Path(path_type=Path), help="Output directory for results")
@click.option("--verbose", "-v", is_flag=True, help="Enable verbose output")
@click.option("--timeout", default=60, type=int, help="Execution timeout in seconds")
def main(task: Path, model: str, output: Path | None, verbose: bool, timeout: int):
    """Run evaluation on a single task.

    Example:
        python runner.py --task tasks/pygame/bug-fix/pong-001/ --model openai:gpt-4
    """
    # Create model
    model_interface = create_model(model)

    # Check model availability
    if not model_interface.is_available():
        click.echo(click.style(f"Model not available: {model}", fg="red"))
        raise SystemExit(1)

    # Create sandbox config
    sandbox_config = SandboxConfig(timeout=timeout, headless=True)

    # Create runner
    runner = EvaluationRunner(
        task_dir=task,
        model=model_interface,
        sandbox_config=sandbox_config,
        verbose=verbose,
    )

    # Run evaluation
    click.echo(f"Evaluating task: {task.name}")
    click.echo(f"Model: {model}")
    click.echo("-" * 50)

    result = runner.run()

    # Output results
    click.echo("-" * 50)
    if result.success:
        click.echo(click.style(f"Result: PASSED", fg="green"))
    else:
        click.echo(click.style(f"Result: FAILED", fg="red"))

    click.echo(f"Score: {result.score:.2%}")
    click.echo(f"Time: {result.elapsed_time:.2f}s")

    if result.error:
        click.echo(click.style(f"Error: {result.error}", fg="red"))

    if result.test_results:
        tr = result.test_results
        click.echo(f"Tests: {tr.passed}/{tr.total} passed")
        # Show test output if verbose and there were failures or no tests found
        if verbose and (tr.failed > 0 or tr.errors > 0 or tr.total == 0):
            click.echo("\nTest output:")
            click.echo(tr.output[:2000] if len(tr.output) > 2000 else tr.output)

    # Save results if output specified
    if output:
        output.mkdir(parents=True, exist_ok=True)
        result_file = output / f"{result.task_id}_{model.replace(':', '_')}.json"

        result_dict = {
            "task_id": result.task_id,
            "model_name": result.model_name,
            "success": result.success,
            "score": result.score,
            "elapsed_time": result.elapsed_time,
            "error": result.error,
            "metadata": result.metadata,
        }

        if result.test_results:
            result_dict["test_results"] = {
                "passed": result.test_results.passed,
                "failed": result.test_results.failed,
                "total": result.test_results.total,
            }

        with open(result_file, "w") as f:
            json.dump(result_dict, f, indent=2)

        click.echo(f"\nResults saved to: {result_file}")


if __name__ == "__main__":
    main()
