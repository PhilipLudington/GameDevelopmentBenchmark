"""Tests for the wrap-around walls feature."""

import os
import sys
import pytest

# Set up headless mode
os.environ["SDL_VIDEODRIVER"] = "dummy"
os.environ["SDL_AUDIODRIVER"] = "dummy"

# Add game directory to path
game_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(game_dir, "game"))

from main import Game, GameState, Snake, Position, Direction, GRID_WIDTH, GRID_HEIGHT


class TestWrapAroundFeature:
    """Tests for wrap-around walls mode."""

    def test_game_has_wrap_around_attribute(self):
        """Game should have wrap_around attribute."""
        game = Game(headless=True)
        assert hasattr(game, 'wrap_around'), "Game should have wrap_around attribute"

    def test_wrap_around_default_false(self):
        """Wrap around should be disabled by default."""
        game = Game(headless=True)
        assert game.wrap_around is False, "wrap_around should default to False"

    def test_snake_has_wrap_position_method(self):
        """Snake should have wrap_position method."""
        snake = Snake(Position(10, 10))
        assert hasattr(snake, 'wrap_position'), "Snake should have wrap_position method"

    def test_wrap_position_wraps_negative_x(self):
        """Position with negative x should wrap to right side."""
        snake = Snake(Position(10, 10))
        wrapped = snake.wrap_position(Position(-1, 5))
        assert wrapped.x == GRID_WIDTH - 1, \
            f"x=-1 should wrap to {GRID_WIDTH-1}, got {wrapped.x}"

    def test_wrap_position_wraps_overflow_x(self):
        """Position with x >= GRID_WIDTH should wrap to left side."""
        snake = Snake(Position(10, 10))
        wrapped = snake.wrap_position(Position(GRID_WIDTH, 5))
        assert wrapped.x == 0, f"x={GRID_WIDTH} should wrap to 0, got {wrapped.x}"

    def test_wrap_position_wraps_negative_y(self):
        """Position with negative y should wrap to bottom."""
        snake = Snake(Position(10, 10))
        wrapped = snake.wrap_position(Position(5, -1))
        assert wrapped.y == GRID_HEIGHT - 1, \
            f"y=-1 should wrap to {GRID_HEIGHT-1}, got {wrapped.y}"

    def test_wrap_position_wraps_overflow_y(self):
        """Position with y >= GRID_HEIGHT should wrap to top."""
        snake = Snake(Position(10, 10))
        wrapped = snake.wrap_position(Position(5, GRID_HEIGHT))
        assert wrapped.y == 0, f"y={GRID_HEIGHT} should wrap to 0, got {wrapped.y}"

    def test_no_wall_collision_when_wrap_enabled(self):
        """Wall collision should not trigger when wrap_around is enabled."""
        game = Game(headless=True)
        game.wrap_around = True
        game.set_state(GameState.PLAYING)

        # Move snake to edge
        game.snake.body[0] = Position(GRID_WIDTH - 1, 5)
        game.snake.direction = Direction.RIGHT
        game.snake.next_direction = Direction.RIGHT

        # Update should wrap, not die
        game.update()

        assert game.state == GameState.PLAYING, \
            "Game should continue when wrap_around is enabled"


class TestGameImports:
    """Basic tests to verify the game module works."""

    def test_game_imports(self):
        """Test that the game module can be imported."""
        from main import Game, GameState, Snake
        assert Game is not None
        assert Snake is not None
