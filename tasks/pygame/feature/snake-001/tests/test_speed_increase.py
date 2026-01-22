"""Tests for the speed increase feature."""

import os
import sys
import pytest

# Set up headless mode
os.environ["SDL_VIDEODRIVER"] = "dummy"
os.environ["SDL_AUDIODRIVER"] = "dummy"

# Add game directory to path
game_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(game_dir, "game"))

from main import Game, GameState, Position, INITIAL_SNAKE_LENGTH


class TestSpeedIncrease:
    """Tests for the speed increase feature."""

    def test_get_current_fps_exists(self):
        """Game should have get_current_fps method."""
        game = Game(headless=True)
        assert hasattr(game, 'get_current_fps'), \
            "Game class should have get_current_fps method"

    def test_base_fps_is_10(self):
        """Base FPS should be 10 at initial snake length."""
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)

        fps = game.get_current_fps()
        assert fps == 10, f"Base FPS should be 10, got {fps}"

    def test_fps_increases_with_snake_length(self):
        """FPS should increase as snake grows."""
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)

        # Initial FPS
        base_fps = game.get_current_fps()
        assert base_fps == 10

        # Grow snake by 5 segments
        game.snake.grow(5)
        for _ in range(5):
            game.snake.move()

        fps_after_5 = game.get_current_fps()
        assert fps_after_5 == 11, \
            f"FPS should be 11 after 5 extra segments, got {fps_after_5}"

        # Grow snake by 5 more segments (total 10 extra)
        game.snake.grow(5)
        for _ in range(5):
            game.snake.move()

        fps_after_10 = game.get_current_fps()
        assert fps_after_10 == 12, \
            f"FPS should be 12 after 10 extra segments, got {fps_after_10}"

    def test_fps_capped_at_20(self):
        """FPS should be capped at 20."""
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)

        # Grow snake by 100 segments (way more than needed for max FPS)
        game.snake.grow(100)
        for _ in range(100):
            game.snake.move()

        fps = game.get_current_fps()
        assert fps == 20, f"FPS should be capped at 20, got {fps}"

    def test_fps_not_below_10(self):
        """FPS should never go below 10."""
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)

        fps = game.get_current_fps()
        assert fps >= 10, f"FPS should never be below 10, got {fps}"

    def test_fps_calculation_formula(self):
        """Verify the FPS calculation follows the expected formula."""
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)

        # Test various lengths
        test_cases = [
            (INITIAL_SNAKE_LENGTH + 0, 10),   # 0 extra -> 10 FPS
            (INITIAL_SNAKE_LENGTH + 4, 10),   # 4 extra -> 10 FPS (not yet 5)
            (INITIAL_SNAKE_LENGTH + 5, 11),   # 5 extra -> 11 FPS
            (INITIAL_SNAKE_LENGTH + 9, 11),   # 9 extra -> 11 FPS (not yet 10)
            (INITIAL_SNAKE_LENGTH + 10, 12),  # 10 extra -> 12 FPS
            (INITIAL_SNAKE_LENGTH + 50, 20),  # 50 extra -> 20 FPS (capped)
        ]

        for target_length, expected_fps in test_cases:
            game.reset_game()
            segments_to_add = target_length - INITIAL_SNAKE_LENGTH
            if segments_to_add > 0:
                game.snake.grow(segments_to_add)
                for _ in range(segments_to_add):
                    game.snake.move()

            actual_fps = game.get_current_fps()
            assert actual_fps == expected_fps, \
                f"With snake length {target_length}, expected FPS {expected_fps}, got {actual_fps}"


class TestGameImports:
    """Basic tests to verify the game module works."""

    def test_game_imports(self):
        """Test that the game module can be imported."""
        from main import Game, GameState, Snake, Food
        assert Game is not None
        assert GameState is not None

    def test_game_creates(self):
        """Test game can be created."""
        game = Game(headless=True)
        assert game is not None
