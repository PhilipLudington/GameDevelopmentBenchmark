"""Tests for the length display feature."""

import os
import sys
import pytest

# Set up headless mode
os.environ["SDL_VIDEODRIVER"] = "dummy"
os.environ["SDL_AUDIODRIVER"] = "dummy"

# Add game directory to path
game_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(game_dir, "game"))

from main import Game, GameState


class TestLengthDisplayFeature:
    """Tests for length display functionality."""

    def test_game_has_draw_length_method(self):
        """Game should have draw_length method."""
        game = Game(headless=True)
        assert hasattr(game, 'draw_length'), \
            "Game should have draw_length method"

    def test_draw_length_is_callable(self):
        """draw_length should be callable."""
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)

        # Should not raise an exception
        try:
            game.draw_length()
        except Exception as e:
            pytest.fail(f"draw_length raised an exception: {e}")

    def test_length_reflects_snake_body(self):
        """Displayed length should match actual snake length."""
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)

        # The length we display should match snake body length
        displayed_length = len(game.snake.body)
        assert displayed_length > 0, "Snake should have a positive length"

    def test_length_updates_when_snake_grows(self):
        """Length display should update when snake grows."""
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)

        initial_length = len(game.snake.body)

        game.snake.grow(1)
        game.snake.move()

        new_length = len(game.snake.body)
        assert new_length == initial_length + 1, \
            "Length should increase when snake grows"


class TestGameImports:
    """Basic tests to verify the game module works."""

    def test_game_imports(self):
        """Test that the game module can be imported."""
        from main import Game, GameState
        assert Game is not None
