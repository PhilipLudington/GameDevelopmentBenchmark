"""Tests for the task."""

import os
import sys
import pytest

os.environ["SDL_VIDEODRIVER"] = "dummy"
os.environ["SDL_AUDIODRIVER"] = "dummy"

game_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(game_dir, "game"))

from main import Game, GameState, Ball, Paddle


class TestBasic:
    """Basic tests to verify the game works."""

    def test_game_imports(self):
        from main import Game, GameState
        assert Game is not None

    def test_game_initializes(self):
        game = Game(headless=True)
        assert game.state == GameState.MENU
