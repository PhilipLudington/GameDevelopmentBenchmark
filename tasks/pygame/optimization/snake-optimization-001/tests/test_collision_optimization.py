"""Tests for the collision detection optimization."""

import os
import sys
import pytest

# Set up headless mode
os.environ["SDL_VIDEODRIVER"] = "dummy"
os.environ["SDL_AUDIODRIVER"] = "dummy"

# Add game directory to path
game_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(game_dir, "game"))

from main import Snake, Position


class TestCollisionOptimization:
    """Tests for optimized collision detection."""

    def test_snake_has_body_set(self):
        """Snake should have body_set attribute for O(1) lookup."""
        snake = Snake(Position(10, 10))
        assert hasattr(snake, 'body_set'), "Snake should have body_set attribute"

    def test_body_set_is_set_type(self):
        """body_set should be a set."""
        snake = Snake(Position(10, 10))
        assert isinstance(snake.body_set, set), "body_set should be a set"

    def test_body_set_matches_body(self):
        """body_set should contain same positions as body."""
        snake = Snake(Position(10, 10))
        snake.grow(5)
        for _ in range(5):
            snake.move()

        body_tuples = set(p.to_tuple() for p in snake.body)
        assert snake.body_set == body_tuples, "body_set should match body positions"

    def test_body_set_updated_after_move(self):
        """body_set should be updated after snake moves."""
        snake = Snake(Position(10, 10))
        initial_set = snake.body_set.copy()

        snake.move()

        assert snake.body_set != initial_set, "body_set should change after move"

    def test_check_collision_uses_set(self):
        """check_collision should work with optimized set lookup."""
        snake = Snake(Position(10, 10))

        # Position in snake body
        assert snake.check_collision(Position(10, 10)), \
            "Should detect collision with head"
        assert snake.check_collision(Position(9, 10)), \
            "Should detect collision with body"

        # Position not in snake
        assert not snake.check_collision(Position(20, 20)), \
            "Should not detect collision outside snake"

    def test_check_self_collision_still_works(self):
        """Self collision detection should still work correctly."""
        snake = Snake(Position(10, 10))

        # Initially no self collision
        assert not snake.check_self_collision()

        # Create self collision scenario
        snake.grow(5)
        for _ in range(5):
            snake.move()

        # Make snake collide with itself
        snake.change_direction(Direction.UP)
        snake.move()
        snake.change_direction(Direction.LEFT)
        snake.move()
        snake.change_direction(Direction.DOWN)
        snake.move()

        from main import Direction
        assert snake.check_self_collision(), "Should detect self collision"

    def test_collision_performance(self):
        """Collision check should complete quickly even with long snake."""
        snake = Snake(Position(15, 12))

        # Grow snake to significant length
        snake.grow(100)
        for _ in range(100):
            snake.move()

        # These checks should be fast (O(1))
        import time
        start = time.time()
        for _ in range(1000):
            snake.check_collision(Position(0, 0))
        elapsed = time.time() - start

        # Should complete 1000 checks in under 0.1 seconds
        assert elapsed < 0.1, f"Collision checks took too long: {elapsed}s"


class TestGameImports:
    """Basic tests to verify the game module works."""

    def test_game_imports(self):
        """Test that the game module can be imported."""
        from main import Snake, Position
        assert Snake is not None
