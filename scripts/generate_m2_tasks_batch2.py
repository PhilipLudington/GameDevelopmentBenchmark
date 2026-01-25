#!/usr/bin/env python3
"""
Generate additional task structure for Pygame M2 expansion - Batch 2.
"""

import json
import os
import shutil
from pathlib import Path

BASE_DIR = Path(__file__).parent.parent
TASKS_DIR = BASE_DIR / "tasks" / "pygame"
BASELINES_DIR = BASE_DIR / "baselines" / "pygame"

# Additional tasks to reach 160+ total
TASKS = [
    # ============= MORE ASTEROIDS TASKS =============
    {"id": "asteroids-bug-001", "name": "Fix screen wrap collision at edges", "category": "bug-fix", "tier": 3, "baseline": "asteroids",
     "description": "Asteroids near screen edges can pass through ship due to wrap logic", "tags": ["bug", "collision", "wrap"],
     "prompt": "# Fix Screen Wrap Collision\n\nAsteroids wrapping at screen edges can skip collision detection."},
    {"id": "asteroids-bug-002", "name": "Fix bullet-asteroid collision near wrap point", "category": "bug-fix", "tier": 3, "baseline": "asteroids",
     "description": "Bullets miss asteroids at screen wrap boundaries", "tags": ["bug", "collision", "wrap"],
     "prompt": "# Fix Bullet Wrap Collision\n\nFix collision detection for bullets near screen edges."},
    {"id": "asteroids-feature-005", "name": "Add multi-shot power-up", "category": "feature", "tier": 3, "baseline": "asteroids",
     "description": "Power-up that lets ship fire 3 bullets in spread pattern", "tags": ["power-up", "shooting"],
     "prompt": "# Multi-Shot Power-Up\n\nAdd spread shot power-up that fires 3 bullets."},
    {"id": "asteroids-feature-006", "name": "Implement screen shake effect", "category": "feature", "tier": 3, "baseline": "asteroids",
     "description": "Add screen shake when asteroids are destroyed", "tags": ["effects", "camera"],
     "prompt": "# Screen Shake Effect\n\nAdd camera shake when explosions occur."},
    {"id": "asteroids-feature-007", "name": "Add smart bomb ability", "category": "feature", "tier": 3, "baseline": "asteroids",
     "description": "Limited-use bomb that destroys all on-screen asteroids", "tags": ["ability", "bomb"],
     "prompt": "# Smart Bomb Ability\n\nImplement a limited-use bomb that clears all asteroids."},
    {"id": "asteroids-optimization-002", "name": "Implement object pooling", "category": "optimization", "tier": 3, "baseline": "asteroids",
     "description": "Pool bullets and particles to reduce garbage collection", "tags": ["performance", "memory", "pooling"],
     "prompt": "# Object Pooling\n\nCreate object pools for bullets and particles."},

    # ============= MORE TETRIS TASKS =============
    {"id": "tetris-bug-002", "name": "Fix lock delay reset on failed rotation", "category": "bug-fix", "tier": 3, "baseline": "tetris",
     "description": "Lock timer resets even when rotation fails", "tags": ["bug", "lock-delay", "rotation"],
     "prompt": "# Fix Lock Delay Reset\n\nLock timer should only reset on successful moves."},
    {"id": "tetris-bug-003", "name": "Fix DAS not working after hold", "category": "bug-fix", "tier": 3, "baseline": "tetris",
     "description": "DAS state resets incorrectly after holding a piece", "tags": ["bug", "das", "hold"],
     "prompt": "# Fix DAS After Hold\n\nDAS should maintain state after piece hold."},
    {"id": "tetris-feature-004", "name": "Add 20G instant drop mode", "category": "feature", "tier": 3, "baseline": "tetris",
     "description": "Optional mode where pieces instantly drop to bottom", "tags": ["mode", "speed", "difficulty"],
     "prompt": "# 20G Mode\n\nAdd optional instant gravity mode."},
    {"id": "tetris-feature-005", "name": "Implement garbage line system", "category": "feature", "tier": 3, "baseline": "tetris",
     "description": "Add garbage lines from bottom like competitive Tetris", "tags": ["garbage", "competitive"],
     "prompt": "# Garbage Lines\n\nAdd garbage line system for competitive play."},
    {"id": "tetris-feature-006", "name": "Add perfect clear bonus", "category": "feature", "tier": 3, "baseline": "tetris",
     "description": "Detect and reward clearing entire board", "tags": ["scoring", "perfect-clear"],
     "prompt": "# Perfect Clear\n\nDetect when board is completely cleared."},
    {"id": "tetris-optimization-001", "name": "Optimize line clear check", "category": "optimization", "tier": 3, "baseline": "tetris",
     "description": "Only check rows affected by placed piece", "tags": ["performance", "algorithm"],
     "prompt": "# Optimize Line Clear\n\nOnly check rows where piece landed."},
    {"id": "tetris-mini-001", "name": "Create battle mode for 2 players", "category": "mini-game", "tier": 4, "baseline": "tetris",
     "description": "Split-screen competitive mode with garbage trading", "tags": ["multiplayer", "competitive", "battle"],
     "prompt": "# Battle Mode\n\nCreate 2-player competitive mode."},

    # ============= MORE PLATFORMER TASKS =============
    {"id": "platformer-bug-001", "name": "Fix corner collision getting stuck", "category": "bug-fix", "tier": 3, "baseline": "platformer",
     "description": "Player gets stuck on tile corners", "tags": ["bug", "collision", "physics"],
     "prompt": "# Fix Corner Collision\n\nPlayer should slide off corners, not get stuck."},
    {"id": "platformer-bug-002", "name": "Fix one-way platform from side", "category": "bug-fix", "tier": 3, "baseline": "platformer",
     "description": "Player can clip through one-way platforms from sides", "tags": ["bug", "platform", "collision"],
     "prompt": "# Fix One-Way Platform\n\nFix collision from sides of one-way platforms."},
    {"id": "platformer-feature-005", "name": "Add ladder climbing mechanic", "category": "feature", "tier": 3, "baseline": "platformer",
     "description": "Implement ladders the player can climb", "tags": ["movement", "ladder", "tiles"],
     "prompt": "# Ladder Climbing\n\nAdd climbable ladder tiles."},
    {"id": "platformer-feature-006", "name": "Implement coyote time and jump buffering", "category": "feature", "tier": 3, "baseline": "platformer",
     "description": "Add forgiveness frames for better jump feel", "tags": ["movement", "feel", "controls"],
     "prompt": "# Coyote Time\n\nAdd coyote time and jump buffer frames."},
    {"id": "platformer-feature-007", "name": "Add checkpoint system", "category": "feature", "tier": 3, "baseline": "platformer",
     "description": "Respawn at checkpoints instead of level start", "tags": ["checkpoint", "respawn"],
     "prompt": "# Checkpoints\n\nImplement checkpoint flags for respawning."},
    {"id": "platformer-feature-008", "name": "Implement slope physics", "category": "feature", "tier": 4, "baseline": "platformer",
     "description": "Add sloped tiles the player can walk on", "tags": ["physics", "slopes", "tiles"],
     "prompt": "# Slope Physics\n\nAdd 45-degree slope tiles with proper collision."},
    {"id": "platformer-optimization-001", "name": "Implement dirty rect rendering", "category": "optimization", "tier": 4, "baseline": "platformer",
     "description": "Only redraw changed portions of screen", "tags": ["performance", "rendering"],
     "prompt": "# Dirty Rect Rendering\n\nOptimize rendering with dirty rectangles."},

    # ============= MORE TOWER DEFENSE TASKS =============
    {"id": "tower_defense-bug-001", "name": "Fix path recalculation blocking enemies", "category": "bug-fix", "tier": 3, "baseline": "tower_defense",
     "description": "Enemies freeze when path is recalculated", "tags": ["bug", "pathfinding"],
     "prompt": "# Fix Path Recalculation\n\nEnemies should smoothly transition to new path."},
    {"id": "tower_defense-bug-002", "name": "Fix tower range display with zoom", "category": "bug-fix", "tier": 3, "baseline": "tower_defense",
     "description": "Tower range circle doesn't scale with camera zoom", "tags": ["bug", "display", "ui"],
     "prompt": "# Fix Range Display\n\nTower range should match actual targeting range."},
    {"id": "tower_defense-feature-005", "name": "Add slow tower type", "category": "feature", "tier": 3, "baseline": "tower_defense",
     "description": "Tower that slows enemies instead of damaging", "tags": ["tower", "slow", "status-effect"],
     "prompt": "# Slow Tower\n\nCreate tower type that applies slow debuff."},
    {"id": "tower_defense-feature-006", "name": "Implement tower synergy bonuses", "category": "feature", "tier": 3, "baseline": "tower_defense",
     "description": "Adjacent towers of same type get damage bonus", "tags": ["synergy", "strategy"],
     "prompt": "# Tower Synergy\n\nGive bonuses for placing towers together."},
    {"id": "tower_defense-feature-007", "name": "Add enemy armor and armor-piercing", "category": "feature", "tier": 3, "baseline": "tower_defense",
     "description": "Some enemies have armor that reduces damage", "tags": ["damage-types", "armor"],
     "prompt": "# Armor System\n\nAdd armor to enemies and armor-piercing to towers."},
    {"id": "tower_defense-feature-008", "name": "Implement infinite mode", "category": "feature", "tier": 4, "baseline": "tower_defense",
     "description": "Endless waves with scaling difficulty", "tags": ["mode", "endless", "scaling"],
     "prompt": "# Infinite Mode\n\nAdd endless mode with progressively harder waves."},
    {"id": "tower_defense-mini-001", "name": "Create tower placement puzzle mode", "category": "mini-game", "tier": 4, "baseline": "tower_defense",
     "description": "Limited towers, must place optimally to win", "tags": ["puzzle", "optimization"],
     "prompt": "# Puzzle Mode\n\nCreate levels with fixed tower count."},

    # ============= MORE SNAKE TASKS =============
    {"id": "snake-bug-007", "name": "Fix food spawning on snake", "category": "bug-fix", "tier": 3, "baseline": "snake",
     "description": "Food can spawn overlapping with snake body", "tags": ["bug", "food", "spawn"],
     "prompt": "# Fix Food Spawn\n\nFood should never spawn on snake body."},
    {"id": "snake-feature-010", "name": "Add portal tiles", "category": "feature", "tier": 3, "baseline": "snake",
     "description": "Paired portals that teleport the snake", "tags": ["teleport", "tiles", "mechanics"],
     "prompt": "# Portal Tiles\n\nAdd portal pairs that teleport snake."},
    {"id": "snake-feature-011", "name": "Implement time attack mode", "category": "feature", "tier": 3, "baseline": "snake",
     "description": "Eat food against timer with time bonuses", "tags": ["mode", "timer", "speed"],
     "prompt": "# Time Attack\n\nAdd time-limited mode with bonuses."},
    {"id": "snake-feature-012", "name": "Add obstacles that appear over time", "category": "feature", "tier": 3, "baseline": "snake",
     "description": "Walls spawn as game progresses", "tags": ["obstacles", "difficulty"],
     "prompt": "# Dynamic Obstacles\n\nAdd walls that spawn during gameplay."},
    {"id": "snake-optimization-002", "name": "Implement grid-based collision", "category": "optimization", "tier": 3, "baseline": "snake",
     "description": "Use 2D grid lookup instead of list iteration", "tags": ["performance", "collision", "grid"],
     "prompt": "# Grid Collision\n\nUse grid data structure for fast collision."},
    {"id": "snake-mini-003", "name": "Create snake battle royale", "category": "mini-game", "tier": 5, "baseline": "snake",
     "description": "Multiple AI snakes competing", "tags": ["ai", "multiplayer", "battle"],
     "prompt": "# Snake Battle Royale\n\nMultiple snakes compete on one field."},

    # ============= MORE SPACE INVADERS TASKS =============
    {"id": "space_invaders-bug-005", "name": "Fix shield regeneration timing", "category": "bug-fix", "tier": 3, "baseline": "space_invaders",
     "description": "Shield blocks regenerate at wrong times", "tags": ["bug", "shield", "timing"],
     "prompt": "# Fix Shield Regen\n\nShields should only regenerate between waves."},
    {"id": "space_invaders-feature-007", "name": "Add weapon upgrade system", "category": "feature", "tier": 3, "baseline": "space_invaders",
     "description": "Collect power-ups to upgrade weapon", "tags": ["power-up", "weapon", "upgrade"],
     "prompt": "# Weapon Upgrades\n\nAdd progressive weapon upgrades."},
    {"id": "space_invaders-feature-008", "name": "Implement bullet patterns", "category": "feature", "tier": 3, "baseline": "space_invaders",
     "description": "Enemies fire in various bullet patterns", "tags": ["patterns", "bullets", "enemy"],
     "prompt": "# Bullet Patterns\n\nCreate varied enemy attack patterns."},
    {"id": "space_invaders-feature-009", "name": "Add score multiplier combo", "category": "feature", "tier": 3, "baseline": "space_invaders",
     "description": "Consecutive kills increase score multiplier", "tags": ["scoring", "combo"],
     "prompt": "# Score Combo\n\nAdd kill streak multiplier."},
    {"id": "space_invaders-feature-010", "name": "Implement threat assessment AI", "category": "feature", "tier": 4, "baseline": "space_invaders",
     "description": "Enemies prioritize dangerous targets", "tags": ["ai", "targeting", "threat"],
     "prompt": "# Threat Assessment\n\nEnemies target player intelligently."},
    {"id": "space_invaders-optimization-003", "name": "Batch sprite rendering", "category": "optimization", "tier": 4, "baseline": "space_invaders",
     "description": "Combine draw calls for performance", "tags": ["performance", "rendering", "batch"],
     "prompt": "# Batch Rendering\n\nCombine similar sprites into batches."},

    # ============= MORE BREAKOUT TASKS =============
    {"id": "breakout-bug-007", "name": "Fix ball stuck in paddle", "category": "bug-fix", "tier": 3, "baseline": "breakout",
     "description": "Ball can get stuck inside paddle", "tags": ["bug", "collision", "physics"],
     "prompt": "# Fix Ball Stuck\n\nBall should never get inside paddle."},
    {"id": "breakout-feature-010", "name": "Add multi-ball power-up", "category": "feature", "tier": 3, "baseline": "breakout",
     "description": "Power-up that splits ball into 3", "tags": ["power-up", "multiball"],
     "prompt": "# Multi-Ball\n\nAdd power-up that creates multiple balls."},
    {"id": "breakout-feature-011", "name": "Implement brick health levels", "category": "feature", "tier": 3, "baseline": "breakout",
     "description": "Bricks require multiple hits based on color", "tags": ["brick", "health", "variety"],
     "prompt": "# Brick Health\n\nDifferent colored bricks need more hits."},
    {"id": "breakout-feature-012", "name": "Add paddle magnet power-up", "category": "feature", "tier": 3, "baseline": "breakout",
     "description": "Ball sticks to paddle, aim and release", "tags": ["power-up", "control"],
     "prompt": "# Magnet Paddle\n\nBall sticks to paddle temporarily."},
    {"id": "breakout-feature-013", "name": "Implement level editor", "category": "feature", "tier": 4, "baseline": "breakout",
     "description": "Create custom brick layouts", "tags": ["editor", "levels", "tools"],
     "prompt": "# Level Editor\n\nCreate and save custom levels."},
    {"id": "breakout-optimization-002", "name": "Optimize brick collision grid", "category": "optimization", "tier": 3, "baseline": "breakout",
     "description": "Use spatial grid for brick collisions", "tags": ["performance", "collision", "grid"],
     "prompt": "# Collision Grid\n\nUse grid for faster brick collision."},

    # ============= MORE PONG TASKS =============
    {"id": "pong-bug-006", "name": "Fix ball speed increase overflow", "category": "bug-fix", "tier": 3, "baseline": "pong",
     "description": "Ball speed can increase infinitely", "tags": ["bug", "speed", "balance"],
     "prompt": "# Fix Speed Overflow\n\nCap ball speed to reasonable maximum."},
    {"id": "pong-feature-013", "name": "Add power-up drops", "category": "feature", "tier": 3, "baseline": "pong",
     "description": "Power-ups occasionally spawn on field", "tags": ["power-up", "items"],
     "prompt": "# Power-Ups\n\nAdd power-up items that spawn."},
    {"id": "pong-feature-014", "name": "Implement ball physics variations", "category": "feature", "tier": 3, "baseline": "pong",
     "description": "Different ball types with unique physics", "tags": ["physics", "variety", "ball"],
     "prompt": "# Ball Types\n\nAdd balls with different physics properties."},
    {"id": "pong-feature-015", "name": "Add obstacle blocks in middle", "category": "feature", "tier": 3, "baseline": "pong",
     "description": "Blocks in center that ball bounces off", "tags": ["obstacles", "level-design"],
     "prompt": "# Obstacles\n\nAdd bouncing obstacles in play area."},
    {"id": "pong-feature-016", "name": "Implement AI difficulty levels", "category": "feature", "tier": 3, "baseline": "pong",
     "description": "Selectable AI difficulty from easy to hard", "tags": ["ai", "difficulty", "settings"],
     "prompt": "# AI Difficulty\n\nAdd selectable AI skill levels."},
    {"id": "pong-optimization-001", "name": "Optimize collision detection", "category": "optimization", "tier": 3, "baseline": "pong",
     "description": "Use swept AABB for better collision", "tags": ["performance", "collision", "physics"],
     "prompt": "# Swept Collision\n\nUse swept AABB for fast ball."},
    {"id": "pong-mini-003", "name": "Create 4-player mode", "category": "mini-game", "tier": 4, "baseline": "pong",
     "description": "4 paddles on all sides of screen", "tags": ["multiplayer", "4-player"],
     "prompt": "# 4-Player Pong\n\nAdd 4 paddles, one per side."},

    # ============= CROSS-GAME ARCHITECTURE TASKS =============
    {"id": "asteroids-feature-008", "name": "Add achievements system", "category": "feature", "tier": 4, "baseline": "asteroids",
     "description": "Track and display player achievements", "tags": ["achievements", "progression", "meta"],
     "prompt": "# Achievements\n\nAdd achievement tracking system."},
    {"id": "tetris-feature-007", "name": "Implement statistics tracking", "category": "feature", "tier": 3, "baseline": "tetris",
     "description": "Track games played, lines cleared, etc", "tags": ["statistics", "persistence"],
     "prompt": "# Statistics\n\nTrack and display gameplay statistics."},
    {"id": "platformer-feature-009", "name": "Add speedrun timer", "category": "feature", "tier": 3, "baseline": "platformer",
     "description": "Precise timer for speedrunning", "tags": ["timer", "speedrun", "competitive"],
     "prompt": "# Speedrun Timer\n\nAdd millisecond-accurate game timer."},
    {"id": "tower_defense-feature-009", "name": "Implement save/load system", "category": "feature", "tier": 4, "baseline": "tower_defense",
     "description": "Save and resume game progress", "tags": ["save", "persistence", "state"],
     "prompt": "# Save System\n\nImplement game state serialization."},
]


def create_task(task_def):
    """Create all files for a task."""
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

    prompt_content = task_def["prompt"].strip() + "\n\n## Testing\n\nRun: `pytest tests/ -v`\n"
    with open(task_dir / "prompt.md", "w") as f:
        f.write(prompt_content)

    baseline_file = BASELINES_DIR / baseline / "main.py"
    if baseline_file.exists():
        shutil.copy(baseline_file, task_dir / "game" / "main.py")
        shutil.copy(baseline_file, task_dir / "solution" / "main.py")

    with open(task_dir / "tests" / "__init__.py", "w") as f:
        f.write(f"# {task_id} tests\n")

    test_file = task_dir / "tests" / f"test_{task_id.replace('-', '_')}.py"
    test_content = f'''"""Tests for {task_def["name"]}."""
import os, sys
os.environ["SDL_VIDEODRIVER"] = "dummy"
os.environ["SDL_AUDIODRIVER"] = "dummy"
game_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(game_dir, "game"))
from main import Game, GameState
import pytest

class TestBasicImports:
    def test_game_imports(self):
        assert Game is not None

if __name__ == "__main__":
    pytest.main([__file__, "-v"])
'''
    with open(test_file, "w") as f:
        f.write(test_content)

    print(f"Created: {task_id}")


def main():
    print(f"Generating {len(TASKS)} additional tasks...")
    for task_def in TASKS:
        create_task(task_def)

    tier_counts = {}
    for task in TASKS:
        tier = task["tier"]
        tier_counts[tier] = tier_counts.get(tier, 0) + 1

    print(f"\nGenerated {len(TASKS)} tasks")
    print("By tier:", tier_counts)


if __name__ == "__main__":
    main()
