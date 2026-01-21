#!/usr/bin/env python3
"""Validate task structure and schema compliance."""

import json
import sys
from pathlib import Path

import click
import jsonschema
from jsonschema import validate, ValidationError


def get_schema_path() -> Path:
    """Get the path to the task schema file."""
    return Path(__file__).parent.parent / "schemas" / "task_schema.json"


def load_schema() -> dict:
    """Load the task JSON schema."""
    schema_path = get_schema_path()
    if not schema_path.exists():
        raise FileNotFoundError(f"Schema file not found: {schema_path}")
    with open(schema_path) as f:
        return json.load(f)


def validate_task_json(task_dir: Path) -> list[str]:
    """Validate task.json against the schema.

    Returns a list of validation errors (empty if valid).
    """
    errors = []
    task_json_path = task_dir / "task.json"

    if not task_json_path.exists():
        errors.append(f"Missing required file: task.json")
        return errors

    try:
        with open(task_json_path) as f:
            task_data = json.load(f)
    except json.JSONDecodeError as e:
        errors.append(f"Invalid JSON in task.json: {e}")
        return errors

    try:
        schema = load_schema()
        validate(instance=task_data, schema=schema)
    except ValidationError as e:
        errors.append(f"Schema validation error: {e.message}")
        if e.path:
            errors.append(f"  at path: {'.'.join(str(p) for p in e.path)}")
    except FileNotFoundError as e:
        errors.append(str(e))

    return errors


def validate_task_structure(task_dir: Path) -> list[str]:
    """Validate the task directory structure.

    Returns a list of validation errors (empty if valid).
    """
    errors = []

    # Required files
    required_files = ["task.json", "prompt.md"]
    for filename in required_files:
        if not (task_dir / filename).exists():
            errors.append(f"Missing required file: {filename}")

    # Required directories
    required_dirs = ["game"]
    for dirname in required_dirs:
        dir_path = task_dir / dirname
        if not dir_path.exists():
            errors.append(f"Missing required directory: {dirname}/")
        elif not dir_path.is_dir():
            errors.append(f"Expected directory but found file: {dirname}")

    # Check game directory has Python files
    game_dir = task_dir / "game"
    if game_dir.exists() and game_dir.is_dir():
        py_files = list(game_dir.glob("*.py"))
        if not py_files:
            errors.append("game/ directory contains no Python files")

    # Optional but recommended directories
    recommended_dirs = ["solution", "tests"]
    warnings = []
    for dirname in recommended_dirs:
        if not (task_dir / dirname).exists():
            warnings.append(f"Recommended directory missing: {dirname}/")

    return errors, warnings


def validate_prompt(task_dir: Path) -> list[str]:
    """Validate the prompt.md file content.

    Returns a list of validation errors (empty if valid).
    """
    errors = []
    prompt_path = task_dir / "prompt.md"

    if not prompt_path.exists():
        return errors  # Already caught in structure validation

    content = prompt_path.read_text()

    if len(content.strip()) < 50:
        errors.append("prompt.md is too short (< 50 characters)")

    # Check for required sections
    required_sections = ["# ", "## "]
    has_heading = any(section in content for section in required_sections)
    if not has_heading:
        errors.append("prompt.md should have at least one heading")

    return errors


def validate_solution(task_dir: Path) -> list[str]:
    """Validate the solution directory if present.

    Returns a list of validation errors (empty if valid).
    """
    errors = []
    solution_dir = task_dir / "solution"

    if not solution_dir.exists():
        return errors  # Optional directory

    if not solution_dir.is_dir():
        errors.append("solution should be a directory")
        return errors

    # Check solution has Python files
    py_files = list(solution_dir.glob("*.py"))
    if not py_files:
        errors.append("solution/ directory contains no Python files")

    return errors


def validate_tests(task_dir: Path) -> list[str]:
    """Validate the tests directory if present.

    Returns a list of validation errors (empty if valid).
    """
    errors = []
    tests_dir = task_dir / "tests"

    if not tests_dir.exists():
        return errors  # Optional directory

    if not tests_dir.is_dir():
        errors.append("tests should be a directory")
        return errors

    # Check for test files
    test_files = list(tests_dir.glob("test_*.py"))
    if not test_files:
        errors.append("tests/ directory contains no test_*.py files")

    return errors


@click.command()
@click.argument("task_path", type=click.Path(exists=True, file_okay=False, path_type=Path))
@click.option("--strict", is_flag=True, help="Treat warnings as errors")
@click.option("--quiet", is_flag=True, help="Only output errors")
def main(task_path: Path, strict: bool, quiet: bool):
    """Validate a benchmark task directory.

    TASK_PATH is the path to the task directory to validate.
    """
    all_errors = []
    all_warnings = []

    if not quiet:
        click.echo(f"Validating task: {task_path}")
        click.echo("-" * 50)

    # Validate JSON schema
    if not quiet:
        click.echo("Checking task.json schema...")
    errors = validate_task_json(task_path)
    all_errors.extend(errors)

    # Validate directory structure
    if not quiet:
        click.echo("Checking directory structure...")
    errors, warnings = validate_task_structure(task_path)
    all_errors.extend(errors)
    all_warnings.extend(warnings)

    # Validate prompt
    if not quiet:
        click.echo("Checking prompt.md...")
    errors = validate_prompt(task_path)
    all_errors.extend(errors)

    # Validate solution
    if not quiet:
        click.echo("Checking solution/...")
    errors = validate_solution(task_path)
    all_errors.extend(errors)

    # Validate tests
    if not quiet:
        click.echo("Checking tests/...")
    errors = validate_tests(task_path)
    all_errors.extend(errors)

    # Output results
    if not quiet:
        click.echo("-" * 50)

    if all_warnings and not quiet:
        click.echo(click.style("Warnings:", fg="yellow"))
        for warning in all_warnings:
            click.echo(f"  - {warning}")

    if all_errors:
        click.echo(click.style("Errors:", fg="red"))
        for error in all_errors:
            click.echo(f"  - {error}")
        click.echo(click.style(f"\nValidation FAILED with {len(all_errors)} error(s)", fg="red"))
        sys.exit(1)
    elif strict and all_warnings:
        click.echo(click.style(f"\nValidation FAILED (strict mode) with {len(all_warnings)} warning(s)", fg="red"))
        sys.exit(1)
    else:
        if not quiet:
            click.echo(click.style("\nValidation PASSED", fg="green"))
        sys.exit(0)


if __name__ == "__main__":
    main()
