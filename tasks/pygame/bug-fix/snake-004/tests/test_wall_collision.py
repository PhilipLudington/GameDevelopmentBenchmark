"""Tests for the wall collision boundary fix."""

import os
import sys
import pytest

# Set up headless mode
os.environ["SDL_VIDEODRIVER"] = "dummy"
os.environ["SDL_AUDIODRIVER"] = "dummy"

# Add game directory to path
game_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(game_dir, "game"))

from main import Snake, Position, GRID_WIDTH, GRID_HEIGHT


class TestWallCollisionFix:
    """Tests to verify wall collision boundaries are correct."""

    def test_no_collision_at_origin(self):
        """Snake at (0,0) should NOT be in collision - it's still in bounds."""
        snake = Snake(Position(5, 5))
        snake.body[0] = Position(0, 0)

        assert not snake.check_wall_collision(), \
            "Position (0,0) should be valid - no collision"

    def test_no_collision_at_max_bounds(self):
        """Snake at (GRID_WIDTH-1, GRID_HEIGHT-1) should NOT be in collision."""
        snake = Snake(Position(5, 5))
        snake.body[0] = Position(GRID_WIDTH - 1, GRID_HEIGHT - 1)

        assert not snake.check_wall_collision(), \
            f"Position ({GRID_WIDTH-1},{GRID_HEIGHT-1}) should be valid - no collision"

    def test_collision_at_negative_x(self):
        """Snake at x=-1 should be in collision."""
        snake = Snake(Position(5, 5))
        snake.body[0] = Position(-1, 5)

        assert snake.check_wall_collision(), \
            "Position (-1,5) should trigger wall collision"

    def test_collision_at_negative_y(self):
        """Snake at y=-1 should be in collision."""
        snake = Snake(Position(5, 5))
        snake.body[0] = Position(5, -1)

        assert snake.check_wall_collision(), \
            "Position (5,-1) should trigger wall collision"

    def test_collision_at_grid_width(self):
        """Snake at x=GRID_WIDTH should be in collision (outside grid)."""
        snake = Snake(Position(5, 5))
        snake.body[0] = Position(GRID_WIDTH, 5)

        assert snake.check_wall_collision(), \
            f"Position ({GRID_WIDTH},5) should trigger wall collision"

    def test_collision_at_grid_height(self):
        """Snake at y=GRID_HEIGHT should be in collision (outside grid)."""
        snake = Snake(Position(5, 5))
        snake.body[0] = Position(5, GRID_HEIGHT)

        assert snake.check_wall_collision(), \
            f"Position (5,{GRID_HEIGHT}) should trigger wall collision"

    def test_no_collision_at_edge_positions(self):
        """Snake at all edge positions within grid should NOT collide."""
        snake = Snake(Position(5, 5))

        # Test left edge (x=0)
        snake.body[0] = Position(0, 5)
        assert not snake.check_wall_collision(), "Left edge (0,5) should be valid"

        # Test right edge (x=GRID_WIDTH-1)
        snake.body[0] = Position(GRID_WIDTH - 1, 5)
        assert not snake.check_wall_collision(), f"Right edge ({GRID_WIDTH-1},5) should be valid"

        # Test top edge (y=0)
        snake.body[0] = Position(5, 0)
        assert not snake.check_wall_collision(), "Top edge (5,0) should be valid"

        # Test bottom edge (y=GRID_HEIGHT-1)
        snake.body[0] = Position(5, GRID_HEIGHT - 1)
        assert not snake.check_wall_collision(), f"Bottom edge (5,{GRID_HEIGHT-1}) should be valid"

    def test_collision_just_outside_all_edges(self):
        """Snake just outside all edges should collide."""
        snake = Snake(Position(5, 5))

        # Test left wall (x=-1)
        snake.body[0] = Position(-1, 5)
        assert snake.check_wall_collision(), "Just outside left (-1,5) should collide"

        # Test right wall (x=GRID_WIDTH)
        snake.body[0] = Position(GRID_WIDTH, 5)
        assert snake.check_wall_collision(), f"Just outside right ({GRID_WIDTH},5) should collide"

        # Test top wall (y=-1)
        snake.body[0] = Position(5, -1)
        assert snake.check_wall_collision(), "Just outside top (5,-1) should collide"

        # Test bottom wall (y=GRID_HEIGHT)
        snake.body[0] = Position(5, GRID_HEIGHT)
        assert snake.check_wall_collision(), f"Just outside bottom (5,{GRID_HEIGHT}) should collide"


class TestGameImports:
    """Basic tests to verify the game module works."""

    def test_game_imports(self):
        """Test that the game module can be imported."""
        from main import Game, GameState, Snake
        assert Game is not None
        assert GameState is not None

    def test_grid_constants(self):
        """Test grid constants are defined."""
        assert GRID_WIDTH > 0
        assert GRID_HEIGHT > 0
