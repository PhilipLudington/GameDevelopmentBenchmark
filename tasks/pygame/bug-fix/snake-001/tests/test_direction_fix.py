"""Tests for the direction reversal bug fix."""

import os
import sys
import pytest

# Set up headless mode
os.environ["SDL_VIDEODRIVER"] = "dummy"
os.environ["SDL_AUDIODRIVER"] = "dummy"

# Add game directory to path
game_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(game_dir, "game"))

from main import Snake, Direction, Position


class TestDirectionFix:
    """Tests to verify the direction reversal bug is fixed."""

    def test_rapid_up_then_down_blocked(self):
        """Rapid UP then DOWN should block DOWN (opposite of pending UP)."""
        snake = Snake(Position(10, 10))
        assert snake.direction == Direction.RIGHT
        assert snake.next_direction == Direction.RIGHT

        # Change to UP
        snake.change_direction(Direction.UP)
        assert snake.next_direction == Direction.UP

        # Try to immediately reverse with DOWN
        # With the bug: DOWN would be allowed because it checks against
        # direction (RIGHT), and DOWN is not opposite of RIGHT
        # With the fix: DOWN should be blocked because it's opposite of
        # next_direction (UP)
        snake.change_direction(Direction.DOWN)

        # With the fix, DOWN should be blocked
        assert snake.next_direction == Direction.UP, \
            "DOWN should be blocked as it's opposite of pending direction UP"

    def test_cannot_reverse_right_to_left(self):
        """Snake moving right should not be able to reverse to left."""
        snake = Snake(Position(10, 10))
        assert snake.direction == Direction.RIGHT

        # Direct reversal should be blocked
        snake.change_direction(Direction.LEFT)
        assert snake.next_direction == Direction.RIGHT, "Direct reversal should be blocked"

    def test_rapid_left_then_right_blocked(self):
        """Rapid LEFT then RIGHT should block RIGHT (opposite of pending LEFT)."""
        snake = Snake(Position(10, 10))

        # First, change to UP (valid from RIGHT)
        snake.change_direction(Direction.UP)
        snake.move()
        assert snake.direction == Direction.UP

        # Now change to LEFT (valid from UP)
        snake.change_direction(Direction.LEFT)
        assert snake.next_direction == Direction.LEFT

        # Try to immediately reverse with RIGHT
        # With the bug: RIGHT would be allowed (not opposite of direction UP)
        # With the fix: RIGHT should be blocked (opposite of next_direction LEFT)
        snake.change_direction(Direction.RIGHT)

        assert snake.next_direction == Direction.LEFT, \
            "RIGHT should be blocked as it's opposite of pending direction LEFT"

    def test_valid_turns_still_work(self):
        """Valid 90-degree turns should still work normally."""
        snake = Snake(Position(10, 10))
        assert snake.direction == Direction.RIGHT

        # Turn up - valid
        snake.change_direction(Direction.UP)
        assert snake.next_direction == Direction.UP

        snake.move()
        assert snake.direction == Direction.UP

        # Turn left - valid (from UP)
        snake.change_direction(Direction.LEFT)
        assert snake.next_direction == Direction.LEFT

        snake.move()
        assert snake.direction == Direction.LEFT

        # Turn down - valid (from LEFT)
        snake.change_direction(Direction.DOWN)
        assert snake.next_direction == Direction.DOWN

        snake.move()
        assert snake.direction == Direction.DOWN

    def test_direct_reverse_blocked(self):
        """Direct reversal should be blocked."""
        snake = Snake(Position(10, 10))

        # Moving right, try to go left
        snake.change_direction(Direction.LEFT)
        assert snake.next_direction == Direction.RIGHT, "LEFT should be blocked"

        # Move and turn up
        snake.change_direction(Direction.UP)
        snake.move()

        # Moving up, try to go down
        snake.change_direction(Direction.DOWN)
        assert snake.next_direction == Direction.UP, "DOWN should be blocked"

        # Turn left
        snake.change_direction(Direction.LEFT)
        snake.move()

        # Moving left, try to go right
        snake.change_direction(Direction.RIGHT)
        assert snake.next_direction == Direction.LEFT, "RIGHT should be blocked"

    def test_rapid_inputs_against_pending_direction(self):
        """Multiple rapid inputs should be validated against pending direction."""
        snake = Snake(Position(10, 10))
        assert snake.direction == Direction.RIGHT

        # Change to UP
        snake.change_direction(Direction.UP)
        assert snake.next_direction == Direction.UP

        # Try DOWN (opposite of pending UP) - should be blocked with the fix
        snake.change_direction(Direction.DOWN)

        # With the bug, DOWN would be allowed because it checks against
        # direction (RIGHT) instead of next_direction (UP)
        # With the fix, DOWN should be blocked
        assert snake.next_direction != Direction.DOWN, \
            "DOWN should be blocked as opposite of pending UP direction"


class TestGameImports:
    """Basic tests to verify the game module works."""

    def test_game_imports(self):
        """Test that the game module can be imported."""
        from main import Game, GameState, Snake, Food
        assert Game is not None
        assert GameState is not None

    def test_snake_creation(self):
        """Test snake can be created."""
        snake = Snake(Position(10, 10))
        assert snake.head is not None
        assert len(snake.body) > 0

    def test_direction_enum(self):
        """Test Direction enum has opposite property."""
        assert Direction.UP.opposite == Direction.DOWN
        assert Direction.DOWN.opposite == Direction.UP
        assert Direction.LEFT.opposite == Direction.RIGHT
        assert Direction.RIGHT.opposite == Direction.LEFT
