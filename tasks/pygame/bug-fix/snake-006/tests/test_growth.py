"""Tests for the snake growth direction fix."""

import os
import sys
import pytest

# Set up headless mode
os.environ["SDL_VIDEODRIVER"] = "dummy"
os.environ["SDL_AUDIODRIVER"] = "dummy"

# Add game directory to path
game_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(game_dir, "game"))

from main import Snake, Position, INITIAL_SNAKE_LENGTH


class TestGrowthFix:
    """Tests for snake growth direction."""

    def test_growth_extends_from_tail(self):
        """Snake should grow from the tail end."""
        snake = Snake(Position(10, 10))
        initial_tail = Position(snake.body[-1].x, snake.body[-1].y)

        snake.grow(1)
        snake.move()

        # The old tail position should now be second-to-last
        assert snake.body[-2] == initial_tail or snake.body[-1] == initial_tail, \
            "Growth should extend from tail"

    def test_head_moves_correctly_when_growing(self):
        """Head should still move in correct direction when growing."""
        snake = Snake(Position(10, 10))
        initial_head_x = snake.head.x

        snake.grow(1)
        snake.move()

        # Head should have moved right (default direction)
        assert snake.head.x == initial_head_x + 1, \
            "Head should move in direction of travel"

    def test_length_increases_by_one_per_grow(self):
        """Snake length should increase by exactly 1 per grow call."""
        snake = Snake(Position(10, 10))
        initial_length = len(snake.body)

        snake.grow(1)
        snake.move()

        assert len(snake.body) == initial_length + 1, \
            f"Length should be {initial_length + 1}, got {len(snake.body)}"

    def test_multiple_grows_extend_correctly(self):
        """Multiple grow calls should all extend from tail."""
        snake = Snake(Position(10, 10))
        initial_length = len(snake.body)

        snake.grow(3)
        for _ in range(3):
            snake.move()

        assert len(snake.body) == initial_length + 3, \
            f"Length should be {initial_length + 3}, got {len(snake.body)}"

    def test_new_segment_at_tail_position(self):
        """New segment should appear at tail, not head."""
        snake = Snake(Position(10, 10))

        # Record original body positions
        original_positions = [Position(p.x, p.y) for p in snake.body]

        snake.grow(1)
        snake.move()

        # Head should be at a new position
        assert snake.body[0] not in original_positions, \
            "Head should be at a new position"

        # Tail area should contain original tail
        assert any(p == original_positions[-1] for p in snake.body[-2:]), \
            "Growth should preserve tail area"


class TestGameImports:
    """Basic tests to verify the game module works."""

    def test_game_imports(self):
        """Test that the game module can be imported."""
        from main import Snake, Position
        assert Snake is not None

    def test_snake_has_grow_method(self):
        """Snake should have grow method."""
        snake = Snake(Position(10, 10))
        assert hasattr(snake, 'grow')
