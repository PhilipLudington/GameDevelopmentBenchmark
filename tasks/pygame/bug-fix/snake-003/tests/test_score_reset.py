"""Tests for the score reset bug fix."""

import os
import sys
import pytest

# Set up headless mode
os.environ["SDL_VIDEODRIVER"] = "dummy"
os.environ["SDL_AUDIODRIVER"] = "dummy"

# Add game directory to path
game_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(game_dir, "game"))

from main import Game, GameState, INITIAL_SNAKE_LENGTH


class TestScoreResetFix:
    """Tests to verify score resets on new game."""

    def test_score_reset_on_new_game(self):
        """Score should be reset to 0 when starting a new game."""
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)

        # Simulate scoring some points
        game.score = 100

        # Reset game
        game.reset_game()

        assert game.score == 0, f"Score should be 0 after reset, got {game.score}"

    def test_high_score_preserved_on_reset(self):
        """High score should be preserved when resetting the game."""
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)

        # Set a high score
        game.high_score = 500
        game.score = 100

        # Reset game
        game.reset_game()

        assert game.high_score == 500, \
            f"High score should be preserved (500), got {game.high_score}"
        assert game.score == 0, \
            f"Score should be reset to 0, got {game.score}"

    def test_snake_reset_on_new_game(self):
        """Snake should be reset to initial length on new game."""
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)

        # Grow the snake
        game.snake.grow(10)
        for _ in range(10):
            game.snake.move()

        assert len(game.snake.body) > INITIAL_SNAKE_LENGTH

        # Reset game
        game.reset_game()

        assert len(game.snake.body) == INITIAL_SNAKE_LENGTH, \
            f"Snake should have {INITIAL_SNAKE_LENGTH} segments, got {len(game.snake.body)}"

    def test_multiple_resets(self):
        """Score should be correct after multiple game resets."""
        game = Game(headless=True)

        for i in range(3):
            game.set_state(GameState.PLAYING)
            game.score = (i + 1) * 50

            game.reset_game()

            assert game.score == 0, \
                f"Score should be 0 after reset {i+1}, got {game.score}"


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
        assert game.score == 0
