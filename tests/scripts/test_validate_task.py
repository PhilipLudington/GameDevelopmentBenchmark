"""Tests for scripts/validate_task.py (BUG.md Bug 1, DESIGN.md § Task Validation Rules).

Each broken-task case here must fail validation, so a gate that passes every task
cannot go unnoticed; the valid cases and the shipped-task sweep must pass.
"""

import json
from pathlib import Path

import pytest
from click.testing import CliRunner

from scripts import create_task
from scripts.validate_task import main as validate_main

REPO_ROOT = Path(__file__).resolve().parents[2]
SHIPPED_TASKS = sorted(p.parent for p in REPO_ROOT.glob("tasks/*/*/*/task.json"))

PROMPT = "# Fix the thing\n\n## Details\n\nA prompt long enough to pass the length check.\n"


def _write_json(path: Path, data: dict) -> None:
    path.write_text(json.dumps(data, indent=2))


def make_pygame_task(root: Path, task_id: str = "pong-901", **overrides) -> Path:
    task_dir = root / task_id
    (task_dir / "game").mkdir(parents=True)
    (task_dir / "game" / "main.py").write_text("print('game')\n")
    (task_dir / "solution").mkdir()
    (task_dir / "solution" / "main.py").write_text("print('fixed')\n")
    (task_dir / "tests").mkdir()
    (task_dir / "tests" / "test_game.py").write_text("def test_x():\n    pass\n")
    (task_dir / "prompt.md").write_text(PROMPT)
    data = {
        "id": task_id,
        "name": "A pygame task",
        "category": "bug-fix",
        "tier": 1,
        "engine": "pygame",
        "description": "Fix a bug in the pygame game",
        "evaluation": ["unit-test"],
        "tags": ["pong"],
    }
    data.update(overrides)
    _write_json(task_dir / "task.json", data)
    return task_dir


def make_julius_task(root: Path, task_id: str = "julius-901", **overrides) -> Path:
    task_dir = root / task_id
    (task_dir / "solution").mkdir(parents=True)
    (task_dir / "solution" / "fix.patch").write_text("--- a/x.c\n+++ b/x.c\n")
    (task_dir / "buggy.patch").write_text("--- a/x.c\n+++ b/x.c\n")
    (task_dir / "tests").mkdir()
    (task_dir / "tests" / "test_x.c").write_text("int main(void) { return 0; }\n")
    (task_dir / "tests" / "Makefile").write_text("all:\n")
    (task_dir / "prompt.md").write_text(PROMPT)
    data = {
        "id": task_id,
        "name": "A julius task",
        "category": "crash-fix",
        "tier": 3,
        "engine": "julius",
        "language": "c",
        "description": "Fix a crash in the julius source",
        "evaluation": ["unit-test"],
        "commit": "synthetic",
    }
    data.update(overrides)
    _write_json(task_dir / "task.json", data)
    return task_dir


def run_validate(task_dir: Path, *extra: str):
    return CliRunner().invoke(validate_main, [str(task_dir), "--quiet", *extra])


# --- valid tasks pass -------------------------------------------------------


def test_valid_pygame_task_passes(tmp_path):
    result = run_validate(make_pygame_task(tmp_path))
    assert result.exit_code == 0, result.output


def test_valid_julius_task_passes(tmp_path):
    result = run_validate(make_julius_task(tmp_path))
    assert result.exit_code == 0, result.output


@pytest.mark.parametrize("category", ["crash-fix", "game-logic", "visual", "memory-safety"])
def test_julius_categories_are_accepted(tmp_path, category):
    result = run_validate(make_julius_task(tmp_path, category=category))
    assert result.exit_code == 0, result.output


@pytest.mark.parametrize("commit", ["synthetic", "a1b2c3d4"])
def test_commit_accepts_hex_or_synthetic(tmp_path, commit):
    result = run_validate(make_julius_task(tmp_path, commit=commit))
    assert result.exit_code == 0, result.output


# --- broken tasks fail ------------------------------------------------------


def test_pygame_task_without_game_dir_fails(tmp_path):
    task_dir = make_pygame_task(tmp_path)
    (task_dir / "game" / "main.py").unlink()
    (task_dir / "game").rmdir()
    result = run_validate(task_dir)
    assert result.exit_code == 1
    assert "game/" in result.output


@pytest.mark.parametrize("patch", ["buggy.patch", "solution/fix.patch"])
def test_julius_task_missing_a_patch_fails(tmp_path, patch):
    task_dir = make_julius_task(tmp_path)
    (task_dir / patch).unlink()
    result = run_validate(task_dir)
    assert result.exit_code == 1
    assert patch in result.output


def test_julius_task_with_game_dir_fails(tmp_path):
    task_dir = make_julius_task(tmp_path)
    (task_dir / "game").mkdir()
    (task_dir / "game" / "x.c").write_text("int x;\n")
    result = run_validate(task_dir)
    assert result.exit_code == 1
    assert "game/" in result.output


def test_id_differing_from_folder_fails(tmp_path):
    task_dir = make_pygame_task(tmp_path, task_id="snake-901", id="snake-feature-901")
    result = run_validate(task_dir)
    assert result.exit_code == 1
    assert "folder" in result.output


def test_underscore_id_fails(tmp_path):
    result = run_validate(make_pygame_task(tmp_path, task_id="space_invaders-901"))
    assert result.exit_code == 1


def test_underscore_tag_fails(tmp_path):
    result = run_validate(make_pygame_task(tmp_path, tags=["space_invaders"]))
    assert result.exit_code == 1


def test_null_expected_asan_error_fails(tmp_path):
    result = run_validate(make_julius_task(tmp_path, expected_asan_error=None))
    assert result.exit_code == 1


def test_commit_typo_fails(tmp_path):
    result = run_validate(make_julius_task(tmp_path, commit="synthetiq"))
    assert result.exit_code == 1


def test_every_schema_error_is_reported(tmp_path):
    task_dir = make_julius_task(tmp_path, commit="synthetiq", expected_asan_error=None)
    result = run_validate(task_dir)
    assert result.exit_code == 1
    assert "at path: commit" in result.output
    assert "at path: expected_asan_error" in result.output


# --- the shipped tasks ------------------------------------------------------


def test_225_tasks_are_shipped():
    assert len(SHIPPED_TASKS) == 225


@pytest.mark.parametrize("task_dir", SHIPPED_TASKS, ids=lambda p: str(p.relative_to(REPO_ROOT)))
def test_every_shipped_task_validates(task_dir):
    result = run_validate(task_dir)
    assert result.exit_code == 0, result.output


def test_shipped_task_ids_are_unique():
    ids = [json.loads((d / "task.json").read_text())["id"] for d in SHIPPED_TASKS]
    assert len(ids) == len(set(ids))


# --- the task generator -----------------------------------------------------


def test_create_task_tags_a_baseline_with_hyphens(tmp_path, monkeypatch):
    monkeypatch.setattr(create_task, "get_project_root", lambda: tmp_path)
    result = CliRunner().invoke(
        create_task.main,
        ["--category", "bug-fix", "--tier", "1", "--baseline", "space_invaders",
         "--id", "space-invaders-901", "--name", "A new task"],
    )
    assert result.exit_code == 0, result.output
    task_json = tmp_path / "tasks" / "pygame" / "bug-fix" / "space-invaders-901" / "task.json"
    tags = json.loads(task_json.read_text())["tags"]
    assert "space-invaders" in tags
    assert "space_invaders" not in tags
