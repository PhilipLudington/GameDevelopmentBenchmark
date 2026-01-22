"""Tests for the food spawn bug fix."""

import os
import sys
import pytest

# Set up headless mode
os.environ["SDL_VIDEODRIVER"] = "dummy"
os.environ["SDL_AUDIODRIVER"] = "dummy"

# Add game directory to path
game_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(game_dir, "game"))

from main import Snake, Food, Position, GRID_WIDTH, GRID_HEIGHT


class TestFoodSpawnFix:
    """Tests to verify food never spawns on snake."""

    def test_food_does_not_spawn_on_snake_head(self):
        """Food should not spawn on the snake's head."""
        snake = Snake(Position(10, 10))
        food = Food()

        # Try many spawns to check for collisions
        for _ in range(100):
            food.spawn(snake)
            assert food.position != snake.head, \
                "Food spawned on snake head"

    def test_food_does_not_spawn_on_snake_body(self):
        """Food should not spawn on any part of the snake's body."""
        snake = Snake(Position(10, 10))
        # Grow the snake
        snake.grow(10)
        for _ in range(10):
            snake.move()

        food = Food()

        # Try many spawns
        for _ in range(100):
            food.spawn(snake)
            assert not snake.check_collision(food.position), \
                f"Food spawned on snake at {food.position}"

    def test_food_always_valid_after_spawn(self):
        """Food should always be at a valid position after spawn."""
        snake = Snake(Position(10, 10))
        food = Food()

        for _ in range(100):
            food.spawn(snake)

            # Check within grid
            assert 0 <= food.position.x < GRID_WIDTH, \
                f"Food x={food.position.x} outside grid"
            assert 0 <= food.position.y < GRID_HEIGHT, \
                f"Food y={food.position.y} outside grid"

            # Check not on snake
            assert not snake.check_collision(food.position), \
                "Food on snake"

    def test_food_spawn_with_long_snake(self):
        """Food should find valid position even with a long snake."""
        # Create a snake that covers a significant area
        snake = Snake(Position(15, 10))
        snake.grow(50)
        for _ in range(50):
            snake.move()

        food = Food()

        # Should still be able to spawn
        for _ in range(50):
            food.spawn(snake)
            assert not snake.check_collision(food.position), \
                "Food spawned on long snake"


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

    def test_food_creation(self):
        """Test food can be created."""
        food = Food()
        assert food.position is not None
