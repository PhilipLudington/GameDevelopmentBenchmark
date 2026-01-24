"""Evaluator for Julius benchmark tasks.

Implements the scoring logic for Julius tasks according to the evaluation protocol:
- Compiles: 1 point
- No new ASan errors: 1 point
- Test passes: 2 points
- Matches original fix structure (bonus): 1 point

Maximum score: 5 points (4 base + 1 bonus)
"""

import json
import time
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, Dict, Optional

from evaluation.asan_parser import ASanReport, parse_asan_output
from evaluation.julius_test_runner import JuliusTestRunner, JuliusTestResult
from harness.julius_sandbox import JuliusSandbox, JuliusSandboxConfig
from harness.patch_utils import (
    apply_patch,
    compare_patches,
    create_patch_from_diff,
    extract_complete_file,
    extract_model_patch,
    parse_unified_diff,
)


@dataclass
class JuliusTaskConfig:
    """Configuration loaded from Julius task.json."""

    id: str
    name: str
    category: str
    tier: int
    engine: str
    description: str
    evaluation: list[str]
    commit: str  # Git commit to checkout
    timeout: int = 300
    tags: list[str] = field(default_factory=list)
    files_to_modify: list[str] = field(default_factory=list)
    expected_asan_error: Optional[str] = None  # Expected ASan error in buggy code
    requires_assets: bool = False  # Whether task requires Caesar III assets


@dataclass
class JuliusEvaluationResult:
    """Result of evaluating a Julius task."""

    task_id: str
    model_name: str
    success: bool

    # Scoring breakdown (max 5 points)
    compiles: bool = False  # 1 point
    no_asan_errors: bool = False  # 1 point
    tests_pass: bool = False  # 2 points
    matches_fix_structure: bool = False  # 1 bonus point

    total_score: float = 0.0
    max_score: float = 5.0

    # Detailed results
    test_results: Optional[JuliusTestResult] = None
    asan_report: Optional[ASanReport] = None
    patch_similarity: float = 0.0

    # Metadata
    model_response: str = ""
    applied_patch: str = ""
    elapsed_time: float = 0.0
    error: Optional[str] = None
    metadata: Dict[str, Any] = field(default_factory=dict)

    @property
    def score_percentage(self) -> float:
        """Get score as percentage."""
        return (self.total_score / self.max_score) * 100


class JuliusEvaluator:
    """Evaluates AI model performance on Julius benchmark tasks.

    Evaluation workflow:
    1. Clone Julius at the task's commit
    2. Apply buggy.patch to create broken code
    3. Verify bug exists (test fails or ASan triggers)
    4. Present code + bug report to AI model
    5. Apply model's proposed fix
    6. Run test suite with ASan
    7. Score according to evaluation protocol
    """

    def __init__(
        self,
        task_dir: Path,
        model,  # ModelInterface
        sandbox_config: Optional[JuliusSandboxConfig] = None,
        verbose: bool = False,
    ):
        """Initialize the Julius evaluator.

        Args:
            task_dir: Path to the task directory
            model: Model interface to use
            sandbox_config: Optional sandbox configuration
            verbose: Enable verbose output
        """
        self.task_dir = Path(task_dir)
        self.model = model
        self.sandbox_config = sandbox_config or JuliusSandboxConfig()
        self.verbose = verbose
        self.task_config: Optional[JuliusTaskConfig] = None

    def log(self, message: str) -> None:
        """Log a message if verbose mode is enabled."""
        if self.verbose:
            print(message)

    def load_task(self) -> JuliusTaskConfig:
        """Load the task configuration.

        Returns:
            JuliusTaskConfig from task.json

        Raises:
            FileNotFoundError: If task.json doesn't exist
            ValueError: If task.json is invalid
        """
        task_json_path = self.task_dir / "task.json"
        if not task_json_path.exists():
            raise FileNotFoundError(f"task.json not found in {self.task_dir}")

        with open(task_json_path) as f:
            data = json.load(f)

        self.task_config = JuliusTaskConfig(
            id=data["id"],
            name=data["name"],
            category=data["category"],
            tier=data["tier"],
            engine=data["engine"],
            description=data["description"],
            evaluation=data["evaluation"],
            commit=data["commit"],
            timeout=data.get("timeout", 300),
            tags=data.get("tags", []),
            files_to_modify=data.get("files_to_modify", []),
            expected_asan_error=data.get("expected_asan_error"),
            requires_assets=data.get("requires_assets", False),
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

    def load_buggy_patch(self) -> str:
        """Load the buggy patch that reverts the fix.

        Returns:
            Contents of buggy.patch

        Raises:
            FileNotFoundError: If buggy.patch doesn't exist
        """
        patch_path = self.task_dir / "buggy.patch"
        if not patch_path.exists():
            raise FileNotFoundError(f"buggy.patch not found in {self.task_dir}")

        return patch_path.read_text()

    def load_solution_patch(self) -> Optional[str]:
        """Load the reference solution patch.

        Returns:
            Contents of solution/fix.patch or None if not present
        """
        solution_path = self.task_dir / "solution" / "fix.patch"
        if solution_path.exists():
            return solution_path.read_text()
        return None

    def build_context(self, sandbox: JuliusSandbox) -> Dict[str, Any]:
        """Build context for the model including relevant file contents.

        Args:
            sandbox: JuliusSandbox with buggy code

        Returns:
            Context dictionary for model generation
        """
        context: Dict[str, Any] = {
            "system": (
                "You are an expert C developer debugging issues in Julius, "
                "an open-source reimplementation of Caesar III.\n\n"
                "CRITICAL: Respond with the COMPLETE fixed file. "
                "Output the entire modified file in a code block with the filename:\n"
                "```c src/path/to/file.c\n"
                "<complete file contents with your fix applied>\n"
                "```\n\n"
                "Important:\n"
                "- Output the ENTIRE file, not just the changed parts\n"
                "- Include all original code with your fix integrated\n"
                "- Only change what's necessary to fix the bug\n"
                "- Preserve all existing formatting and structure"
            ),
            "files": {},
        }

        # Include files that need to be modified
        if self.task_config and self.task_config.files_to_modify:
            for filepath in self.task_config.files_to_modify:
                content = sandbox.get_file_content(filepath)
                if content:
                    context["files"][filepath] = content

        return context

    def evaluate(self) -> JuliusEvaluationResult:
        """Run the full evaluation pipeline.

        Returns:
            JuliusEvaluationResult with scoring and details
        """
        start_time = time.time()

        try:
            # Load task configuration
            self.log(f"Loading task from {self.task_dir}")
            task_config = self.load_task()

            # Check if task requires assets we don't have
            if task_config.requires_assets:
                self.log("Warning: Task requires Caesar III assets")
                # Could check for assets here and skip if not available

            return self._run_evaluation(task_config, start_time)

        except Exception as e:
            elapsed = time.time() - start_time
            return JuliusEvaluationResult(
                task_id=self.task_config.id if self.task_config else "unknown",
                model_name=self.model.get_name(),
                success=False,
                elapsed_time=elapsed,
                error=str(e),
            )

    def _run_evaluation(
        self,
        task_config: JuliusTaskConfig,
        start_time: float,
    ) -> JuliusEvaluationResult:
        """Run the evaluation with the loaded task config.

        Args:
            task_config: Loaded task configuration
            start_time: Evaluation start time

        Returns:
            JuliusEvaluationResult
        """
        prompt = self.load_prompt()
        buggy_patch = self.load_buggy_patch()
        solution_patch = self.load_solution_patch()

        with JuliusSandbox(self.sandbox_config) as sandbox:
            # Clone Julius at the task's commit
            self.log(f"Cloning Julius at commit {task_config.commit}")
            clone_result = sandbox.clone(commit=task_config.commit)

            if not clone_result.success:
                elapsed = time.time() - start_time
                return JuliusEvaluationResult(
                    task_id=task_config.id,
                    model_name=self.model.get_name(),
                    success=False,
                    elapsed_time=elapsed,
                    error=f"Failed to clone: {clone_result.error}",
                )

            # Apply buggy patch to revert the fix
            self.log("Applying buggy patch to create broken code")
            patch_result = sandbox.apply_buggy_patch(self.task_dir / "buggy.patch")

            if not patch_result.success:
                elapsed = time.time() - start_time
                return JuliusEvaluationResult(
                    task_id=task_config.id,
                    model_name=self.model.get_name(),
                    success=False,
                    elapsed_time=elapsed,
                    error=f"Failed to apply buggy patch: {patch_result.error}",
                )

            # Build context for model
            context = self.build_context(sandbox)

            # Generate response from model
            self.log(f"Invoking model: {self.model.get_name()}")
            if hasattr(self.model, 'config'):
                self.model.config.timeout = task_config.timeout
                # Julius tasks need more tokens for complete file output
                self.model.config.max_tokens = 16384
            model_result = self.model.generate(prompt, context)

            # Extract complete file or patch from model response
            self.log("Extracting fix from model response")

            # First, try to extract complete file (preferred method)
            complete_files = extract_complete_file(model_result.content)
            model_patch = None
            original_contents = {}

            if complete_files:
                self.log(f"Found complete file(s): {list(complete_files.keys())}")
                # Save original contents for patch comparison
                for filepath, new_content in complete_files.items():
                    original = sandbox.get_file_content(filepath)
                    if original:
                        original_contents[filepath] = original
                        # Generate patch for comparison
                        model_patch = create_patch_from_diff(original, new_content, filepath)
                # Apply all complete files at once
                sandbox.apply_file_changes(complete_files)
                fix_result_success = True
            else:
                # Fall back to patch extraction
                self.log("No complete file found, trying patch extraction")
                model_patch = extract_model_patch(model_result.content)

                if not model_patch:
                    elapsed = time.time() - start_time
                    return JuliusEvaluationResult(
                        task_id=task_config.id,
                        model_name=self.model.get_name(),
                        success=False,
                        model_response=model_result.content,
                        elapsed_time=elapsed,
                        error="No fix found in model response (expected complete file or patch)",
                    )

                # Apply patch
                self.log("Applying model's proposed patch")
                fix_result = sandbox.apply_model_fix(model_patch)
                fix_result_success = fix_result.success

            # Score: Compiles (1 point)
            compiles = False
            if fix_result_success:
                self.log("Building with model's fix...")
                build_result = sandbox.build()
                compiles = build_result.success

            # Run tests
            self.log("Running tests...")
            test_runner = JuliusTestRunner(sandbox, timeout=task_config.timeout)
            test_dir = self.task_dir / "tests"
            test_results = test_runner.run(test_dir)

            # Score: No ASan errors (1 point)
            no_asan_errors = True
            if test_results.asan_report:
                no_asan_errors = not test_results.asan_report.has_errors

            # Score: Tests pass (2 points)
            tests_pass = test_results.success

            # Score: Matches fix structure (1 bonus point)
            matches_fix_structure = False
            patch_similarity = 0.0
            if solution_patch and model_patch:
                patch_similarity = compare_patches(solution_patch, model_patch)
                matches_fix_structure = patch_similarity >= 0.7  # 70% similarity threshold

            # Calculate total score
            total_score = 0.0
            if compiles:
                total_score += 1.0
            if no_asan_errors:
                total_score += 1.0
            if tests_pass:
                total_score += 2.0
            if matches_fix_structure:
                total_score += 1.0

            # Overall success requires: compiles + no ASan errors + tests pass
            success = compiles and no_asan_errors and tests_pass

            elapsed = time.time() - start_time

            return JuliusEvaluationResult(
                task_id=task_config.id,
                model_name=self.model.get_name(),
                success=success,
                compiles=compiles,
                no_asan_errors=no_asan_errors,
                tests_pass=tests_pass,
                matches_fix_structure=matches_fix_structure,
                total_score=total_score,
                test_results=test_results,
                asan_report=test_results.asan_report,
                patch_similarity=patch_similarity,
                model_response=model_result.content,
                applied_patch=model_patch,
                elapsed_time=elapsed,
                metadata={
                    "model_config": self.model.get_config(),
                    "task_tier": task_config.tier,
                    "task_category": task_config.category,
                    "commit": task_config.commit,
                },
            )


def evaluate_julius_task(
    task_dir: Path,
    model,
    verbose: bool = False,
) -> JuliusEvaluationResult:
    """Convenience function to evaluate a single Julius task.

    Args:
        task_dir: Path to task directory
        model: Model interface to use
        verbose: Enable verbose output

    Returns:
        JuliusEvaluationResult
    """
    evaluator = JuliusEvaluator(task_dir, model, verbose=verbose)
    return evaluator.evaluate()


def format_julius_result(result: JuliusEvaluationResult) -> str:
    """Format Julius evaluation result as human-readable text.

    Args:
        result: Evaluation result

    Returns:
        Formatted string
    """
    lines = [
        f"Task: {result.task_id}",
        f"Model: {result.model_name}",
        f"Result: {'PASSED' if result.success else 'FAILED'}",
        f"",
        f"Score Breakdown ({result.total_score}/{result.max_score}):",
        f"  [{'✓' if result.compiles else '✗'}] Compiles: {'1' if result.compiles else '0'} point",
        f"  [{'✓' if result.no_asan_errors else '✗'}] No ASan errors: {'1' if result.no_asan_errors else '0'} point",
        f"  [{'✓' if result.tests_pass else '✗'}] Tests pass: {'2' if result.tests_pass else '0'} points",
        f"  [{'✓' if result.matches_fix_structure else '✗'}] Matches fix: {'1' if result.matches_fix_structure else '0'} bonus point",
        f"",
        f"Patch similarity: {result.patch_similarity:.1%}",
        f"Time: {result.elapsed_time:.2f}s",
    ]

    if result.error:
        lines.append(f"\nError: {result.error}")

    if result.asan_report and result.asan_report.has_errors:
        lines.append(f"\nASan errors found: {result.asan_report.error_count}")
        for error in result.asan_report.errors[:3]:  # Show first 3
            lines.append(f"  - {error.error_type.value}: {error.summary}")

    return "\n".join(lines)
