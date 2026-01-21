#!/usr/bin/env python3
"""Create a new benchmark task from a baseline game."""

import json
import shutil
from pathlib import Path

import click


PROMPT_TEMPLATE = """# {name}

## Problem Description

{description}

## Task

Your goal is to fix the issue described above. The game code is located in the `game/` directory.

## Files

The main game file is `game/main.py`. You may need to examine and modify other files as well.

## Expected Behavior

After your fix:
{expected_behavior}

## Hints

{hints}

## Testing

Run the game to verify your fix works correctly:
```bash
python game/main.py
```

Run the automated tests:
```bash
pytest tests/ -v
```
"""


def get_project_root() -> Path:
    """Get the project root directory."""
    return Path(__file__).parent.parent


def get_baseline_path(engine: str, baseline: str) -> Path:
    """Get the path to a baseline game."""
    return get_project_root() / "baselines" / engine / baseline


def get_task_path(engine: str, category: str, task_id: str) -> Path:
    """Get the path for a new task."""
    return get_project_root() / "tasks" / engine / category / task_id


def create_task_json(
    task_id: str,
    name: str,
    category: str,
    tier: int,
    engine: str,
    description: str,
    baseline: str | None,
    tags: list[str],
    evaluation: list[str],
) -> dict:
    """Create the task.json content."""
    task_data = {
        "id": task_id,
        "name": name,
        "category": category,
        "tier": tier,
        "engine": engine,
        "description": description,
        "evaluation": evaluation,
        "tags": tags,
    }
    if baseline:
        task_data["baseline"] = baseline
    return task_data


def create_prompt_md(
    name: str,
    description: str,
    expected_behavior: str = "- The bug is fixed\n- All tests pass",
    hints: str = "- Look at the relevant function(s)\n- Check edge cases",
) -> str:
    """Create the prompt.md content."""
    return PROMPT_TEMPLATE.format(
        name=name,
        description=description,
        expected_behavior=expected_behavior,
        hints=hints,
    )


def copy_baseline(baseline_path: Path, task_path: Path) -> None:
    """Copy baseline game to task's game directory."""
    game_dir = task_path / "game"
    game_dir.mkdir(parents=True, exist_ok=True)

    # Copy all Python files from baseline
    for py_file in baseline_path.glob("*.py"):
        shutil.copy(py_file, game_dir / py_file.name)

    # Copy assets if they exist
    assets_dir = baseline_path / "assets"
    if assets_dir.exists():
        shutil.copytree(assets_dir, game_dir / "assets")


def create_test_template(task_id: str) -> str:
    """Create a basic test template."""
    return f'''"""Tests for task {task_id}."""

import pytest
import sys
from pathlib import Path

# Add game directory to path
game_dir = Path(__file__).parent.parent / "game"
sys.path.insert(0, str(game_dir))


class TestBasic:
    """Basic tests to verify the fix."""

    def test_game_imports(self):
        """Test that the game module can be imported."""
        import main
        assert main is not None

    def test_game_initializes(self):
        """Test that the game can initialize."""
        # TODO: Add initialization test
        pass


class TestBugFix:
    """Tests specific to the bug fix."""

    def test_fix_applied(self):
        """Test that the fix resolves the issue."""
        # TODO: Add specific test for the bug fix
        pass
'''


@click.command()
@click.option("--category", "-c", required=True,
              type=click.Choice(["bug-fix", "feature", "optimization", "mini-game"]),
              help="Task category")
@click.option("--tier", "-t", required=True, type=click.IntRange(1, 4),
              help="Difficulty tier (1-4)")
@click.option("--baseline", "-b", default=None, help="Baseline game to derive from")
@click.option("--id", "task_id", required=True, help="Unique task identifier")
@click.option("--name", "-n", required=True, help="Human-readable task name")
@click.option("--engine", "-e", default="pygame",
              type=click.Choice(["pygame", "godot", "unity", "unreal"]),
              help="Game engine")
@click.option("--description", "-d", default="", help="Brief task description")
@click.option("--tag", "-g", multiple=True, help="Tags for categorization")
@click.option("--force", "-f", is_flag=True, help="Overwrite existing task")
def main(
    category: str,
    tier: int,
    baseline: str | None,
    task_id: str,
    name: str,
    engine: str,
    description: str,
    tag: tuple,
    force: bool,
):
    """Create a new benchmark task.

    Example:
        python create_task.py --category bug-fix --tier 2 --baseline pong \\
            --id "ball-angle-fix" --name "Ball angle always same"
    """
    task_path = get_task_path(engine, category, task_id)

    if task_path.exists() and not force:
        click.echo(click.style(f"Task already exists: {task_path}", fg="red"))
        click.echo("Use --force to overwrite")
        raise SystemExit(1)

    # Set default description if not provided
    if not description:
        description = f"{category.replace('-', ' ').title()} task: {name}"

    # Set default tags
    tags = list(tag) if tag else []
    if baseline:
        tags.append(baseline)
    tags.append(category)

    # Determine evaluation methods based on category
    if category == "bug-fix":
        evaluation = ["unit-test", "gameplay"]
    elif category == "feature":
        evaluation = ["unit-test", "integration-test", "gameplay"]
    elif category == "optimization":
        evaluation = ["performance", "unit-test"]
    else:  # mini-game
        evaluation = ["gameplay", "unit-test"]

    click.echo(f"Creating task: {task_id}")
    click.echo(f"  Category: {category}")
    click.echo(f"  Tier: {tier}")
    click.echo(f"  Engine: {engine}")
    if baseline:
        click.echo(f"  Baseline: {baseline}")

    # Create task directory structure
    task_path.mkdir(parents=True, exist_ok=True)
    (task_path / "game").mkdir(exist_ok=True)
    (task_path / "solution").mkdir(exist_ok=True)
    (task_path / "tests").mkdir(exist_ok=True)

    # Create task.json
    task_data = create_task_json(
        task_id=task_id,
        name=name,
        category=category,
        tier=tier,
        engine=engine,
        description=description,
        baseline=baseline,
        tags=tags,
        evaluation=evaluation,
    )
    with open(task_path / "task.json", "w") as f:
        json.dump(task_data, f, indent=2)
    click.echo("  Created: task.json")

    # Create prompt.md
    prompt_content = create_prompt_md(name=name, description=description)
    with open(task_path / "prompt.md", "w") as f:
        f.write(prompt_content)
    click.echo("  Created: prompt.md")

    # Copy baseline if specified
    if baseline:
        baseline_path = get_baseline_path(engine, baseline)
        if baseline_path.exists():
            copy_baseline(baseline_path, task_path)
            click.echo(f"  Copied baseline from: {baseline_path}")
        else:
            click.echo(click.style(f"  Warning: Baseline not found: {baseline_path}", fg="yellow"))

    # Create test template
    test_content = create_test_template(task_id)
    with open(task_path / "tests" / "test_task.py", "w") as f:
        f.write(test_content)
    click.echo("  Created: tests/test_task.py")

    click.echo(click.style(f"\nTask created: {task_path}", fg="green"))
    click.echo("\nNext steps:")
    click.echo("  1. Add/modify game code in game/")
    click.echo("  2. Add reference solution in solution/")
    click.echo("  3. Update tests in tests/")
    click.echo("  4. Update prompt.md with specific instructions")
    click.echo(f"  5. Validate with: python scripts/validate_task.py {task_path}")


if __name__ == "__main__":
    main()
