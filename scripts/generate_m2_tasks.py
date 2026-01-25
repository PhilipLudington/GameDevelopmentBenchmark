#!/usr/bin/env python3
"""
Generate task structure for Pygame M2 expansion.
Creates directories, task.json, and prompt.md for all new tasks.
"""

import json
import os
import shutil
from pathlib import Path

BASE_DIR = Path(__file__).parent.parent
TASKS_DIR = BASE_DIR / "tasks" / "pygame"
BASELINES_DIR = BASE_DIR / "baselines" / "pygame"

# Task definitions organized by game and category
TASKS = [
    # ============= ASTEROIDS TASKS =============
    # Tier 3 Feature
    {
        "id": "asteroids-feature-002",
        "name": "Implement momentum-based thrust physics",
        "category": "feature",
        "tier": 3,
        "baseline": "asteroids",
        "description": "Add realistic momentum physics where ship velocity persists and thrust adds to current momentum",
        "tags": ["physics", "momentum", "ship-control"],
        "prompt": """# Implement Momentum-Based Thrust Physics

## Feature Description
Enhance the ship physics to use realistic momentum-based thrust. The ship should maintain velocity when not thrusting and thrust should add to the current velocity vector rather than replace it.

## Requirements
1. Remove instant velocity changes - ship maintains momentum
2. Thrust adds acceleration in facing direction
3. Add configurable mass and thrust power
4. Implement proper deceleration (friction/drag)
5. Add momentum indicator to HUD"""
    },
    {
        "id": "asteroids-feature-003",
        "name": "Add hyperspace teleport ability",
        "category": "feature",
        "tier": 3,
        "baseline": "asteroids",
        "description": "Implement a hyperspace ability that teleports the ship to a random location with risk",
        "tags": ["ability", "teleport", "risk-reward"],
        "prompt": """# Add Hyperspace Teleport Ability

## Feature Description
Add a classic hyperspace ability that teleports the ship to a random location. There should be a cooldown and a small chance of the ship exploding.

## Requirements
1. Press H key to activate hyperspace
2. Ship teleports to random safe location
3. 10% chance of ship exploding on teleport
4. 3-second cooldown between uses
5. Visual effect during teleport"""
    },
    # Tier 4 Optimization
    {
        "id": "asteroids-optimization-001",
        "name": "Implement spatial hashing for collision detection",
        "category": "optimization",
        "tier": 4,
        "baseline": "asteroids",
        "description": "Replace O(n²) collision detection with spatial hashing for O(n) performance",
        "tags": ["performance", "collision", "spatial-hash", "algorithm"],
        "prompt": """# Implement Spatial Hashing for Collision Detection

## Feature Description
The current collision detection checks every entity against every other entity (O(n²)). Implement spatial hashing to reduce this to O(n) average case.

## Requirements
1. Create SpatialHash class with configurable cell size
2. Insert all collidable entities into hash each frame
3. Only check collisions between entities in same/adjacent cells
4. Maintain correctness for screen wrapping
5. Performance should scale linearly with entity count"""
    },
    # Tier 4 Feature
    {
        "id": "asteroids-feature-004",
        "name": "Implement particle-based explosion system",
        "category": "feature",
        "tier": 4,
        "baseline": "asteroids",
        "description": "Create an advanced particle system for explosions with physics simulation",
        "tags": ["particles", "effects", "physics"],
        "prompt": """# Implement Particle-Based Explosion System

## Feature Description
Create an advanced particle system that simulates realistic explosions with debris, sparks, and shockwaves.

## Requirements
1. Create ParticleSystem class with object pooling
2. Different particle types: debris, sparks, smoke
3. Particles affected by gravity and drag
4. Shockwave effect that pushes nearby entities
5. Particle lifetime and fade effects
6. Performance: support 500+ active particles"""
    },
    # Tier 5 Architecture
    {
        "id": "asteroids-mini-001",
        "name": "Implement networked multiplayer",
        "category": "mini-game",
        "tier": 5,
        "baseline": "asteroids",
        "description": "Add local network multiplayer support for 2 players",
        "tags": ["networking", "multiplayer", "architecture"],
        "prompt": """# Implement Networked Multiplayer

## Feature Description
Add local network multiplayer support allowing two players to play cooperatively or competitively over LAN.

## Requirements
1. Host/join game modes
2. Synchronize ship positions, bullets, asteroids
3. Handle network latency with prediction
4. Support both cooperative and versus modes
5. Graceful disconnect handling"""
    },

    # ============= TETRIS TASKS =============
    # Tier 3 Bug Fix
    {
        "id": "tetris-bug-001",
        "name": "Fix T-spin detection edge cases",
        "category": "bug-fix",
        "tier": 3,
        "baseline": "tetris",
        "description": "The T-spin detection incorrectly identifies some rotations as T-spins",
        "tags": ["bug", "scoring", "rotation"],
        "prompt": """# Fix T-Spin Detection Edge Cases

## Bug Description
The T-spin detection has edge cases where regular rotations are incorrectly identified as T-spins, and some valid T-spins are missed.

## Requirements
1. Implement proper 3-corner rule for T-spin detection
2. Detect mini T-spins vs full T-spins
3. Track last action (rotation vs movement)
4. Award correct bonus points for T-spins"""
    },
    # Tier 3 Feature
    {
        "id": "tetris-feature-001",
        "name": "Add combo and back-to-back scoring",
        "category": "feature",
        "tier": 3,
        "baseline": "tetris",
        "description": "Implement combo chains and back-to-back Tetris/T-spin bonuses",
        "tags": ["scoring", "combo", "game-mechanics"],
        "prompt": """# Add Combo and Back-to-Back Scoring

## Feature Description
Implement the guideline combo and back-to-back bonus systems.

## Requirements
1. Track combo counter (consecutive line clears)
2. Combo bonus: 50 * combo * level points
3. Track back-to-back (consecutive Tetrises or T-spins)
4. B2B bonus: 1.5x multiplier
5. Display combo and B2B status in HUD"""
    },
    # Tier 4 AI
    {
        "id": "tetris-feature-002",
        "name": "Implement AI player with look-ahead",
        "category": "feature",
        "tier": 4,
        "baseline": "tetris",
        "description": "Create an AI player that can play Tetris using piece evaluation and look-ahead",
        "tags": ["ai", "algorithm", "game-playing"],
        "prompt": """# Implement AI Player with Look-Ahead

## Feature Description
Create an AI that can play Tetris automatically, evaluating positions and planning moves.

## Requirements
1. Evaluate board states (holes, height, roughness)
2. Consider all possible placements for current piece
3. Look ahead to next piece in queue
4. Implement configurable AI speed
5. AI should be able to survive 100+ lines"""
    },
    # Tier 4 Feature
    {
        "id": "tetris-feature-003",
        "name": "Add replay system with deterministic playback",
        "category": "feature",
        "tier": 4,
        "baseline": "tetris",
        "description": "Record and replay games with frame-perfect accuracy",
        "tags": ["replay", "recording", "determinism"],
        "prompt": """# Add Replay System with Deterministic Playback

## Feature Description
Implement a replay system that records inputs and random seed for perfect playback.

## Requirements
1. Record all inputs with frame timestamps
2. Save random seed for deterministic piece sequence
3. Save/load replays to file
4. Playback at variable speed (0.5x, 1x, 2x, 4x)
5. Seek to specific frame"""
    },

    # ============= PLATFORMER TASKS =============
    # Tier 3 Feature
    {
        "id": "platformer-feature-001",
        "name": "Add wall jump and wall slide mechanics",
        "category": "feature",
        "tier": 3,
        "baseline": "platformer",
        "description": "Implement wall sliding and wall jumping for enhanced movement",
        "tags": ["movement", "physics", "player-control"],
        "prompt": """# Add Wall Jump and Wall Slide Mechanics

## Feature Description
Enhance player movement with wall slide and wall jump abilities.

## Requirements
1. Detect when player is against wall while airborne
2. Slow fall speed when wall sliding
3. Wall jump pushes away from wall
4. Configurable wall slide speed and jump force
5. Wall jump resets double jump"""
    },
    # Tier 3 Feature
    {
        "id": "platformer-feature-002",
        "name": "Implement dash ability with cooldown",
        "category": "feature",
        "tier": 3,
        "baseline": "platformer",
        "description": "Add a horizontal dash ability for fast movement",
        "tags": ["movement", "ability", "cooldown"],
        "prompt": """# Implement Dash Ability with Cooldown

## Feature Description
Add a dash ability that gives the player a burst of horizontal speed.

## Requirements
1. Press Shift to dash in facing direction
2. Dash travels fixed distance (100 pixels)
3. Invulnerable during dash
4. 1.5 second cooldown
5. Can dash in air (once per jump)"""
    },
    # Tier 4 Algorithm
    {
        "id": "platformer-feature-003",
        "name": "Add procedural level generation",
        "category": "feature",
        "tier": 4,
        "baseline": "platformer",
        "description": "Generate playable platformer levels procedurally",
        "tags": ["procedural", "level-design", "algorithm"],
        "prompt": """# Add Procedural Level Generation

## Feature Description
Create a procedural level generator that creates playable platformer levels.

## Requirements
1. Generate solvable levels (player can reach goal)
2. Control difficulty via parameters
3. Place platforms with appropriate gaps
4. Spawn enemies and coins procedurally
5. Validate level is completable"""
    },
    # Tier 4 AI
    {
        "id": "platformer-feature-004",
        "name": "Implement behavior tree enemy AI",
        "category": "feature",
        "tier": 4,
        "baseline": "platformer",
        "description": "Replace simple FSM with behavior tree AI for enemies",
        "tags": ["ai", "behavior-tree", "enemy"],
        "prompt": """# Implement Behavior Tree Enemy AI

## Feature Description
Create a behavior tree framework and implement complex enemy behaviors.

## Requirements
1. Create BehaviorTree, Selector, Sequence, and Leaf nodes
2. Implement patrol, chase, attack, flee behaviors
3. Enemies use line-of-sight for detection
4. Support for different enemy types with different trees
5. Debug visualization of active nodes"""
    },
    # Tier 5 Architecture
    {
        "id": "platformer-mini-001",
        "name": "Create in-game level editor",
        "category": "mini-game",
        "tier": 5,
        "baseline": "platformer",
        "description": "Build a full-featured in-game level editor with save/load",
        "tags": ["editor", "tools", "serialization"],
        "prompt": """# Create In-Game Level Editor

## Feature Description
Build a comprehensive level editor that allows creating, editing, and playing custom levels.

## Requirements
1. Tile palette with all tile types
2. Click-drag tile placement
3. Entity placement (enemies, coins, player start)
4. Save/load levels to JSON
5. Test play from editor
6. Undo/redo support"""
    },

    # ============= TOWER DEFENSE TASKS =============
    # Tier 3 Feature
    {
        "id": "tower_defense-feature-001",
        "name": "Add tower targeting priorities",
        "category": "feature",
        "tier": 3,
        "baseline": "tower_defense",
        "description": "Allow players to set tower targeting modes",
        "tags": ["targeting", "strategy", "tower"],
        "prompt": """# Add Tower Targeting Priorities

## Feature Description
Implement selectable targeting modes for towers.

## Requirements
1. Support modes: First, Last, Nearest, Strongest, Weakest
2. Click tower to select, then choose mode
3. Display current mode on tower
4. Save mode preference per tower
5. Default mode is First (furthest along path)"""
    },
    # Tier 3 Feature
    {
        "id": "tower_defense-feature-002",
        "name": "Implement tower upgrade system",
        "category": "feature",
        "tier": 3,
        "baseline": "tower_defense",
        "description": "Allow towers to be upgraded for increased stats",
        "tags": ["upgrade", "economy", "tower"],
        "prompt": """# Implement Tower Upgrade System

## Feature Description
Add an upgrade system that allows improving tower stats.

## Requirements
1. Each tower can be upgraded 3 times
2. Upgrades cost increasing gold
3. Each upgrade improves damage, range, or fire rate
4. Visual indicator of upgrade level
5. Show upgrade cost in tower info panel"""
    },
    # Tier 4 Algorithm
    {
        "id": "tower_defense-feature-003",
        "name": "Add flying enemies that ignore path",
        "category": "feature",
        "tier": 4,
        "baseline": "tower_defense",
        "description": "Implement flying enemies that travel directly to goal",
        "tags": ["enemy-type", "pathfinding", "variety"],
        "prompt": """# Add Flying Enemies That Ignore Path

## Feature Description
Create flying enemies that travel directly toward the goal, ignoring the path.

## Requirements
1. Flying enemies move in straight line to goal
2. Slower than ground enemies
3. Only targetable by certain tower types
4. Spawn in mixed waves with ground enemies
5. Different visual appearance (wings, shadow)"""
    },
    # Tier 4 Optimization
    {
        "id": "tower_defense-optimization-001",
        "name": "Optimize pathfinding with flow fields",
        "category": "optimization",
        "tier": 4,
        "baseline": "tower_defense",
        "description": "Replace individual A* with flow field pathfinding",
        "tags": ["performance", "pathfinding", "flow-field"],
        "prompt": """# Optimize Pathfinding with Flow Fields

## Feature Description
Replace per-enemy A* pathfinding with a single flow field that all enemies follow.

## Requirements
1. Generate flow field from goal to all cells
2. Enemies follow flow field gradient
3. Recalculate only when towers placed/removed
4. Support 1000+ enemies without slowdown
5. Handle multiple paths to goal"""
    },
    # Tier 5 AI
    {
        "id": "tower_defense-feature-004",
        "name": "Implement adaptive wave difficulty",
        "category": "feature",
        "tier": 5,
        "baseline": "tower_defense",
        "description": "Create dynamic wave generation that adapts to player performance",
        "tags": ["ai", "difficulty", "adaptive"],
        "prompt": """# Implement Adaptive Wave Difficulty

## Feature Description
Create a dynamic difficulty system that adjusts waves based on player performance.

## Requirements
1. Track player metrics (lives lost, gold earned, time per wave)
2. Adjust enemy count and types based on performance
3. Scale between easy/medium/hard difficulty bands
4. Prevent death spirals (ease up if struggling)
5. Reward skilled play with bonus waves"""
    },

    # ============= SNAKE TASKS (extending existing) =============
    # Tier 3 Algorithm
    {
        "id": "snake-feature-008",
        "name": "Implement flood fill for enclosed area detection",
        "category": "feature",
        "tier": 3,
        "baseline": "snake",
        "description": "Detect when snake encloses an area and award bonus points",
        "tags": ["algorithm", "flood-fill", "scoring"],
        "prompt": """# Implement Flood Fill for Enclosed Area Detection

## Feature Description
Award bonus points when the snake body encloses an area of the grid.

## Requirements
1. Detect enclosed areas using flood fill
2. Calculate enclosed cell count
3. Award points based on area size
4. Visual highlight of enclosed area
5. Handle multiple enclosed regions"""
    },
    # Tier 4 AI
    {
        "id": "snake-feature-009",
        "name": "Implement A* pathfinding AI snake",
        "category": "feature",
        "tier": 4,
        "baseline": "snake",
        "description": "Create AI that uses A* to find optimal path to food",
        "tags": ["ai", "pathfinding", "a-star"],
        "prompt": """# Implement A* Pathfinding AI Snake

## Feature Description
Create an AI player that uses A* pathfinding to navigate to food while avoiding its own body.

## Requirements
1. A* pathfinding from head to food
2. Consider snake body as obstacles
3. Predict future body positions
4. Fall back to survival mode if no path
5. AI should achieve high scores consistently"""
    },
    # Tier 5 Architecture
    {
        "id": "snake-mini-002",
        "name": "Create plugin/mod system",
        "category": "mini-game",
        "tier": 5,
        "baseline": "snake",
        "description": "Implement a plugin system for game modifications",
        "tags": ["architecture", "plugins", "modding"],
        "prompt": """# Create Plugin/Mod System

## Feature Description
Implement a modular plugin system that allows extending game functionality.

## Requirements
1. Plugin discovery and loading
2. Hook system for game events
3. Plugin can modify game rules
4. Plugin isolation (errors don't crash game)
5. Example plugins: speed mod, obstacles, power-ups"""
    },

    # ============= SPACE INVADERS TASKS (extending existing) =============
    # Tier 3 Feature
    {
        "id": "space_invaders-feature-005",
        "name": "Add boss enemy with attack patterns",
        "category": "feature",
        "tier": 3,
        "baseline": "space_invaders",
        "description": "Create a boss enemy that appears every 5 waves",
        "tags": ["boss", "patterns", "enemy"],
        "prompt": """# Add Boss Enemy with Attack Patterns

## Feature Description
Create a boss enemy with multiple attack patterns that challenges the player.

## Requirements
1. Boss appears every 5 waves
2. Multiple attack patterns (spread shot, beam, missiles)
3. Boss has health bar displayed
4. Different phases based on remaining health
5. Special rewards for defeating boss"""
    },
    # Tier 4 Algorithm
    {
        "id": "space_invaders-feature-006",
        "name": "Implement flocking behavior for enemies",
        "category": "feature",
        "tier": 4,
        "baseline": "space_invaders",
        "description": "Add flocking behavior where enemies move as a coordinated swarm",
        "tags": ["ai", "flocking", "behavior"],
        "prompt": """# Implement Flocking Behavior for Enemies

## Feature Description
Replace grid-based movement with flocking behavior for more organic enemy movement.

## Requirements
1. Implement separation, alignment, cohesion
2. Enemies flock toward player
3. Maintain minimum spacing
4. Smooth transitions between behaviors
5. Support 50+ flocking enemies"""
    },
    # Tier 4 Optimization
    {
        "id": "space_invaders-optimization-002",
        "name": "Implement quadtree spatial partitioning",
        "category": "optimization",
        "tier": 4,
        "baseline": "space_invaders",
        "description": "Use quadtree for efficient collision queries",
        "tags": ["performance", "collision", "quadtree"],
        "prompt": """# Implement Quadtree Spatial Partitioning

## Feature Description
Implement a quadtree data structure for efficient spatial queries and collision detection.

## Requirements
1. Create Quadtree class with insert/query
2. Automatically subdivide when capacity exceeded
3. Rebuild tree each frame
4. Support point and range queries
5. Collision detection should scale to 500+ entities"""
    },
    # Tier 5 Architecture
    {
        "id": "space_invaders-mini-002",
        "name": "Refactor to Entity Component System",
        "category": "mini-game",
        "tier": 5,
        "baseline": "space_invaders",
        "description": "Restructure game using ECS architecture",
        "tags": ["architecture", "ecs", "refactor"],
        "prompt": """# Refactor to Entity Component System

## Feature Description
Refactor the game architecture to use an Entity Component System pattern.

## Requirements
1. Create Entity, Component, System base classes
2. Components: Transform, Sprite, Collider, Health, AI
3. Systems: Movement, Rendering, Collision, AI
4. Remove inheritance-based entity types
5. Maintain all existing functionality"""
    },

    # ============= BREAKOUT TASKS (extending existing) =============
    # Tier 3 Feature
    {
        "id": "breakout-feature-008",
        "name": "Add chain reaction brick destruction",
        "category": "feature",
        "tier": 3,
        "baseline": "breakout",
        "description": "Create explosive bricks that destroy adjacent bricks",
        "tags": ["mechanics", "chain-reaction", "brick-types"],
        "prompt": """# Add Chain Reaction Brick Destruction

## Feature Description
Add explosive brick type that destroys adjacent bricks when hit.

## Requirements
1. New explosive brick type (different color)
2. Destroys all adjacent bricks (8 directions)
3. Chain reactions if adjacent explosives
4. Explosion animation effect
5. Bonus points for chain reactions"""
    },
    # Tier 4 Physics
    {
        "id": "breakout-feature-009",
        "name": "Implement verlet integration rope physics",
        "category": "feature",
        "tier": 4,
        "baseline": "breakout",
        "description": "Add a rope/chain powerup using verlet integration",
        "tags": ["physics", "verlet", "power-up"],
        "prompt": """# Implement Verlet Integration Rope Physics

## Feature Description
Create a power-up where the ball trails a rope that can wrap around and destroy bricks.

## Requirements
1. Implement verlet integration for rope
2. Rope connects paddle to ball
3. Rope collides with bricks
4. Rope wraps around obstacles
5. Configurable rope length and stiffness"""
    },
    # Tier 5 Physics
    {
        "id": "breakout-mini-002",
        "name": "Implement destructible brick environment",
        "category": "mini-game",
        "tier": 5,
        "baseline": "breakout",
        "description": "Create fully destructible environment with physics debris",
        "tags": ["physics", "destruction", "particles"],
        "prompt": """# Implement Destructible Brick Environment

## Feature Description
Create a physics-based destruction system where bricks shatter into debris.

## Requirements
1. Bricks break into physics-simulated pieces
2. Debris collides with ball and paddle
3. Debris fades and disappears over time
4. Different materials (glass, stone, metal)
5. Support 100+ debris pieces"""
    },

    # ============= PONG TASKS (extending existing) =============
    # Tier 3 Physics
    {
        "id": "pong-feature-011",
        "name": "Implement curved ball trajectories",
        "category": "feature",
        "tier": 3,
        "baseline": "pong",
        "description": "Add spin mechanics that curve the ball's path",
        "tags": ["physics", "spin", "ball-mechanics"],
        "prompt": """# Implement Curved Ball Trajectories

## Feature Description
Add spin mechanics where paddle movement when hitting adds curve to the ball.

## Requirements
1. Track paddle velocity at hit time
2. Apply spin based on paddle velocity
3. Ball curves in flight
4. Spin diminishes over time
5. Visual spin indicator on ball"""
    },
    # Tier 4 AI
    {
        "id": "pong-feature-012",
        "name": "Implement minimax AI opponent",
        "category": "feature",
        "tier": 4,
        "baseline": "pong",
        "description": "Create an AI opponent using minimax algorithm",
        "tags": ["ai", "minimax", "game-theory"],
        "prompt": """# Implement Minimax AI Opponent

## Feature Description
Create an AI that uses minimax algorithm to make optimal decisions.

## Requirements
1. Predict ball trajectory
2. Evaluate positions using minimax
3. Configurable search depth
4. Alpha-beta pruning for performance
5. Difficulty levels via search depth"""
    },
    # Tier 5 Multiplayer
    {
        "id": "pong-mini-002",
        "name": "Add local network multiplayer",
        "category": "mini-game",
        "tier": 5,
        "baseline": "pong",
        "description": "Implement network play between two computers",
        "tags": ["networking", "multiplayer", "synchronization"],
        "prompt": """# Add Local Network Multiplayer

## Feature Description
Allow two players to play over a local network connection.

## Requirements
1. Host/join lobby system
2. Synchronize ball and paddle positions
3. Handle latency with prediction
4. Clean disconnect handling
5. In-game chat"""
    },
]


def create_task(task_def):
    """Create all files for a task."""
    task_id = task_def["id"]
    category = task_def["category"]
    baseline = task_def["baseline"]

    # Create task directory
    task_dir = TASKS_DIR / category / task_id
    task_dir.mkdir(parents=True, exist_ok=True)

    # Create subdirectories
    (task_dir / "game").mkdir(exist_ok=True)
    (task_dir / "solution").mkdir(exist_ok=True)
    (task_dir / "tests").mkdir(exist_ok=True)

    # Create task.json
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

    # Create prompt.md
    prompt_content = task_def["prompt"].strip() + """

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```
"""

    with open(task_dir / "prompt.md", "w") as f:
        f.write(prompt_content)

    # Copy baseline game
    baseline_file = BASELINES_DIR / baseline / "main.py"
    if baseline_file.exists():
        shutil.copy(baseline_file, task_dir / "game" / "main.py")
        shutil.copy(baseline_file, task_dir / "solution" / "main.py")

    # Create empty test file
    tests_init = task_dir / "tests" / "__init__.py"
    with open(tests_init, "w") as f:
        f.write(f"# {task_id} tests\n")

    test_file = task_dir / "tests" / f"test_{task_id.replace('-', '_')}.py"
    test_content = f'''"""
Tests for {task_def["name"]}.
"""

import os
import sys
import pytest

# Setup headless mode
os.environ["SDL_VIDEODRIVER"] = "dummy"
os.environ["SDL_AUDIODRIVER"] = "dummy"

# Add game directory to path
game_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(game_dir, "game"))

from main import Game, GameState


class TestBasicImports:
    """Test that game can be imported."""

    def test_game_imports(self):
        assert Game is not None

    def test_game_creates(self):
        game = Game(headless=True)
        assert game is not None


# TODO: Add specific tests for {task_def["name"]}
# Tests should verify the requirements in prompt.md


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
'''

    with open(test_file, "w") as f:
        f.write(test_content)

    print(f"Created task: {task_id}")


def main():
    """Generate all M2 tasks."""
    print(f"Generating {len(TASKS)} tasks...")

    for task_def in TASKS:
        create_task(task_def)

    print(f"\nGenerated {len(TASKS)} tasks")

    # Print summary by tier
    tier_counts = {}
    for task in TASKS:
        tier = task["tier"]
        tier_counts[tier] = tier_counts.get(tier, 0) + 1

    print("\nTasks by tier:")
    for tier in sorted(tier_counts.keys()):
        print(f"  Tier {tier}: {tier_counts[tier]} tasks")


if __name__ == "__main__":
    main()
