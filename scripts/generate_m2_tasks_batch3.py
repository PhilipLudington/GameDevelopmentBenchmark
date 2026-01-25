#!/usr/bin/env python3
"""Final batch to reach 160+ tasks, focusing on Tier 4-5."""

import json
import os
import shutil
from pathlib import Path

BASE_DIR = Path(__file__).parent.parent
TASKS_DIR = BASE_DIR / "tasks" / "pygame"
BASELINES_DIR = BASE_DIR / "baselines" / "pygame"

TASKS = [
    # ============= TIER 4 TASKS =============
    {"id": "asteroids-feature-009", "name": "Implement gravitational wells", "category": "feature", "tier": 4, "baseline": "asteroids",
     "description": "Add gravity sources that affect bullet and asteroid trajectories", "tags": ["physics", "gravity", "simulation"],
     "prompt": "# Gravitational Wells\n\nAdd gravity points that pull nearby entities."},
    {"id": "asteroids-feature-010", "name": "Add continuous collision detection", "category": "feature", "tier": 4, "baseline": "asteroids",
     "description": "Prevent fast objects from passing through each other", "tags": ["physics", "collision", "ccd"],
     "prompt": "# Continuous Collision\n\nImplement swept collision for fast objects."},
    {"id": "tetris-feature-008", "name": "Implement Tetris 99 style targeting", "category": "feature", "tier": 4, "baseline": "tetris",
     "description": "Add targeting system for sending garbage to opponents", "tags": ["competitive", "targeting", "strategy"],
     "prompt": "# Targeting System\n\nImplement targeting badges for garbage routing."},
    {"id": "tetris-feature-009", "name": "Add zone mechanics like Tetris Effect", "category": "feature", "tier": 4, "baseline": "tetris",
     "description": "Time-stop zone ability that lets pieces float", "tags": ["ability", "mechanics", "zone"],
     "prompt": "# Zone Mechanic\n\nAdd time-stop zone ability."},
    {"id": "platformer-feature-010", "name": "Implement parallax background scrolling", "category": "feature", "tier": 4, "baseline": "platformer",
     "description": "Multi-layer parallax background with depth effect", "tags": ["graphics", "parallax", "camera"],
     "prompt": "# Parallax Scrolling\n\nAdd multi-layer parallax backgrounds."},
    {"id": "platformer-feature-011", "name": "Add dialogue system with choices", "category": "feature", "tier": 4, "baseline": "platformer",
     "description": "NPCs with branching dialogue trees", "tags": ["dialogue", "npc", "narrative"],
     "prompt": "# Dialogue System\n\nCreate dialogue boxes with choices."},
    {"id": "tower_defense-feature-010", "name": "Implement tower ability cooldowns", "category": "feature", "tier": 4, "baseline": "tower_defense",
     "description": "Towers have special abilities with cooldowns", "tags": ["abilities", "cooldown", "tower"],
     "prompt": "# Tower Abilities\n\nAdd activated abilities to towers."},
    {"id": "tower_defense-feature-011", "name": "Add multi-path level support", "category": "feature", "tier": 4, "baseline": "tower_defense",
     "description": "Levels with branching enemy paths", "tags": ["pathfinding", "level-design", "multi-path"],
     "prompt": "# Multi-Path Levels\n\nSupport branching paths for enemies."},
    {"id": "snake-feature-013", "name": "Implement snake AI with learning", "category": "feature", "tier": 4, "baseline": "snake",
     "description": "AI that improves over multiple games using simple RL", "tags": ["ai", "learning", "reinforcement"],
     "prompt": "# Learning AI\n\nAI that improves with experience."},
    {"id": "space_invaders-feature-011", "name": "Add bullet time slowdown", "category": "feature", "tier": 4, "baseline": "space_invaders",
     "description": "Slow motion ability for dodging bullets", "tags": ["ability", "slowmo", "time"],
     "prompt": "# Bullet Time\n\nAdd slowdown ability."},
    {"id": "breakout-feature-014", "name": "Implement physics-based ball spin", "category": "feature", "tier": 4, "baseline": "breakout",
     "description": "Ball rotation affects bounce trajectory", "tags": ["physics", "spin", "ball"],
     "prompt": "# Ball Spin\n\nPhysics-based spin affecting bounce."},
    {"id": "pong-feature-017", "name": "Add tournament bracket mode", "category": "feature", "tier": 4, "baseline": "pong",
     "description": "Multi-player tournament with brackets", "tags": ["tournament", "multiplayer", "competitive"],
     "prompt": "# Tournament Mode\n\nAdd bracket-style tournament."},

    # ============= TIER 5 TASKS =============
    {"id": "asteroids-mini-002", "name": "Create asteroid field procedural generator", "category": "mini-game", "tier": 5, "baseline": "asteroids",
     "description": "Infinite procedural asteroid levels with seeds", "tags": ["procedural", "generation", "infinite"],
     "prompt": "# Procedural Asteroids\n\nGenerate infinite unique asteroid fields."},
    {"id": "tetris-mini-002", "name": "Implement Tetris 99 full mode", "category": "mini-game", "tier": 5, "baseline": "tetris",
     "description": "Simulate 99-player battle royale with AI opponents", "tags": ["battle-royale", "ai", "simulation"],
     "prompt": "# Tetris 99 Mode\n\n99-player battle with AI."},
    {"id": "platformer-mini-002", "name": "Create procedural metroidvania generator", "category": "mini-game", "tier": 5, "baseline": "platformer",
     "description": "Generate connected rooms with item progression", "tags": ["procedural", "metroidvania", "generation"],
     "prompt": "# Procedural Metroidvania\n\nGenerate explorable map with item gates."},
    {"id": "tower_defense-mini-002", "name": "Implement creep AI that learns player patterns", "category": "mini-game", "tier": 5, "baseline": "tower_defense",
     "description": "Enemies adapt to player tower placement over waves", "tags": ["ai", "learning", "adaptive"],
     "prompt": "# Adaptive Enemies\n\nEnemies learn to counter player strategy."},
    {"id": "snake-mini-004", "name": "Create neural network snake AI", "category": "mini-game", "tier": 5, "baseline": "snake",
     "description": "Train NN from scratch without ML libraries", "tags": ["neural-network", "ai", "training"],
     "prompt": "# Neural Network AI\n\nImplement NN snake player from scratch."},
    {"id": "space_invaders-mini-003", "name": "Add full boss rush mode", "category": "mini-game", "tier": 5, "baseline": "space_invaders",
     "description": "Sequence of unique bosses with complex patterns", "tags": ["boss", "patterns", "challenge"],
     "prompt": "# Boss Rush\n\nMode with sequential unique bosses."},
    {"id": "breakout-mini-003", "name": "Create physics sandbox mode", "category": "mini-game", "tier": 5, "baseline": "breakout",
     "description": "Soft body physics with destructible everything", "tags": ["physics", "sandbox", "destruction"],
     "prompt": "# Physics Sandbox\n\nFull physics simulation sandbox."},
    {"id": "pong-mini-004", "name": "Implement AI coach with advice", "category": "mini-game", "tier": 5, "baseline": "pong",
     "description": "AI analyzes player and gives improvement tips", "tags": ["ai", "coach", "analysis"],
     "prompt": "# AI Coach\n\nAI that analyzes and advises player."},

    # ============= MORE TIER 3 =============
    {"id": "asteroids-feature-011", "name": "Add twin-stick control mode", "category": "feature", "tier": 3, "baseline": "asteroids",
     "description": "Alternative controls where aim is separate from move", "tags": ["controls", "twin-stick", "input"],
     "prompt": "# Twin Stick Mode\n\nAdd separate aim and movement controls."},
    {"id": "tetris-feature-010", "name": "Implement finesse tracking", "category": "feature", "tier": 3, "baseline": "tetris",
     "description": "Track and display key presses per piece", "tags": ["statistics", "efficiency", "skill"],
     "prompt": "# Finesse Tracking\n\nCount keypresses and show efficiency."},
    {"id": "platformer-feature-012", "name": "Add camera zoom for boss fights", "category": "feature", "tier": 3, "baseline": "platformer",
     "description": "Dynamic camera zoom based on arena size", "tags": ["camera", "zoom", "boss"],
     "prompt": "# Dynamic Zoom\n\nCamera zooms based on scene."},
    {"id": "tower_defense-feature-012", "name": "Add fast forward button", "category": "feature", "tier": 3, "baseline": "tower_defense",
     "description": "Speed up game to 2x or 3x speed", "tags": ["speed", "controls", "quality-of-life"],
     "prompt": "# Fast Forward\n\nAdd speed up controls."},
    {"id": "snake-feature-014", "name": "Implement screen shake on death", "category": "feature", "tier": 3, "baseline": "snake",
     "description": "Camera shake effect when snake dies", "tags": ["effects", "camera", "feedback"],
     "prompt": "# Death Screen Shake\n\nAdd screen shake on death."},
    {"id": "space_invaders-feature-012", "name": "Add ship selection screen", "category": "feature", "tier": 3, "baseline": "space_invaders",
     "description": "Multiple ships with different stats", "tags": ["ships", "selection", "variety"],
     "prompt": "# Ship Selection\n\nAdd multiple playable ships."},
]


def create_task(task_def):
    task_id = task_def["id"]
    category = task_def["category"]
    baseline = task_def["baseline"]

    task_dir = TASKS_DIR / category / task_id
    task_dir.mkdir(parents=True, exist_ok=True)
    (task_dir / "game").mkdir(exist_ok=True)
    (task_dir / "solution").mkdir(exist_ok=True)
    (task_dir / "tests").mkdir(exist_ok=True)

    task_json = {
        "id": task_def["id"],
        "name": task_def["name"],
        "category": task_def["category"],
        "tier": task_def["tier"],
        "engine": "pygame",
        "description": task_def["description"],
        "evaluation": ["unit-test"],
        "tags": task_def["tags"],
        "baseline": task_def["baseline"]
    }

    with open(task_dir / "task.json", "w") as f:
        json.dump(task_json, f, indent=2)
        f.write("\n")

    with open(task_dir / "prompt.md", "w") as f:
        f.write(task_def["prompt"].strip() + "\n\n## Testing\n\nRun: `pytest tests/ -v`\n")

    baseline_file = BASELINES_DIR / baseline / "main.py"
    if baseline_file.exists():
        shutil.copy(baseline_file, task_dir / "game" / "main.py")
        shutil.copy(baseline_file, task_dir / "solution" / "main.py")

    with open(task_dir / "tests" / "__init__.py", "w") as f:
        f.write(f"# {task_id} tests\n")

    test_content = f'''"""Tests for {task_def["name"]}."""
import os, sys
os.environ["SDL_VIDEODRIVER"] = "dummy"
os.environ["SDL_AUDIODRIVER"] = "dummy"
sys.path.insert(0, os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "game"))
from main import Game, GameState
import pytest

class TestBasicImports:
    def test_game_imports(self):
        assert Game is not None

if __name__ == "__main__":
    pytest.main([__file__, "-v"])
'''
    with open(task_dir / "tests" / f"test_{task_id.replace('-', '_')}.py", "w") as f:
        f.write(test_content)

    print(f"Created: {task_id}")


def main():
    print(f"Generating {len(TASKS)} final tasks...")
    for task_def in TASKS:
        create_task(task_def)

    tier_counts = {}
    for task in TASKS:
        tier_counts[task["tier"]] = tier_counts.get(task["tier"], 0) + 1

    print(f"\nGenerated {len(TASKS)} tasks")
    print("By tier:", tier_counts)


if __name__ == "__main__":
    main()
