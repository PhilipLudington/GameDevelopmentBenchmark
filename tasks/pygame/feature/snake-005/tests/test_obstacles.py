"""Tests for the obstacles feature."""

import os
import sys
import pytest

# Set up headless mode
os.environ["SDL_VIDEODRIVER"] = "dummy"
os.environ["SDL_AUDIODRIVER"] = "dummy"

# Add game directory to path
game_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(game_dir, "game"))

from main import Game, GameState, Position, GRID_WIDTH, GRID_HEIGHT


class TestObstaclesFeature:
    """Tests for obstacle functionality."""

    def test_game_has_obstacles_attribute(self):
        """Game should have obstacles attribute."""
        game = Game(headless=True)
        assert hasattr(game, 'obstacles'), "Game should have obstacles attribute"

    def test_obstacles_is_list(self):
        """Obstacles should be a list."""
        game = Game(headless=True)
        assert isinstance(game.obstacles, list), "obstacles should be a list"

    def test_game_has_generate_obstacles_method(self):
        """Game should have generate_obstacles method."""
        game = Game(headless=True)
        assert hasattr(game, 'generate_obstacles'), \
            "Game should have generate_obstacles method"

    def test_generate_obstacles_creates_obstacles(self):
        """generate_obstacles should create obstacles."""
        game = Game(headless=True)
        game.generate_obstacles()
        assert len(game.obstacles) > 0, "Should generate at least one obstacle"

    def test_obstacles_not_on_snake(self):
        """Obstacles should not spawn on the snake."""
        game = Game(headless=True)
        game.generate_obstacles()

        snake_positions = set(p.to_tuple() for p in game.snake.body)
        for obs in game.obstacles:
            assert obs.to_tuple() not in snake_positions, \
                f"Obstacle at {obs} should not be on snake"

    def test_obstacles_within_grid(self):
        """Obstacles should be within grid boundaries."""
        game = Game(headless=True)
        game.generate_obstacles()

        for obs in game.obstacles:
            assert 0 <= obs.x < GRID_WIDTH, f"Obstacle x={obs.x} outside grid"
            assert 0 <= obs.y < GRID_HEIGHT, f"Obstacle y={obs.y} outside grid"

    def test_obstacle_collision_ends_game(self):
        """Hitting an obstacle should end the game."""
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)

        # Place obstacle directly in front of snake
        game.obstacles = [Position(game.snake.head.x + 1, game.snake.head.y)]

        # Move snake into obstacle
        game.update()

        assert game.state == GameState.GAME_OVER, \
            "Game should end when snake hits obstacle"

    def test_food_not_on_obstacles(self):
        """Food should not spawn on obstacles."""
        game = Game(headless=True)
        game.obstacles = [Position(5, 5), Position(10, 10), Position(15, 15)]

        for _ in range(50):
            game.food.spawn(game.snake)
            obstacle_positions = set(o.to_tuple() for o in game.obstacles)
            assert game.food.position.to_tuple() not in obstacle_positions, \
                "Food should not spawn on obstacles"


class TestGameImports:
    """Basic tests to verify the game module works."""

    def test_game_imports(self):
        """Test that the game module can be imported."""
        from main import Game, GameState
        assert Game is not None
