#!/usr/bin/env python3
"""Tests for breakout-004: Lives not resetting on new game."""

import os
import sys

os.environ["SDL_VIDEODRIVER"] = "dummy"
os.environ["SDL_AUDIODRIVER"] = "dummy"

game_dir = os.environ.get(
    "TEST_GAME_DIR",
    os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "game")
)
sys.path.insert(0, game_dir)

import pytest
from main import Game, GameState, INITIAL_LIVES


class TestLivesReset:
    """Test that lives reset properly on new game."""

    @pytest.fixture
    def game(self):
        return Game(headless=True)

    def test_lives_reset_after_game_over(self, game):
        """Lives should reset to INITIAL_LIVES when starting new game after game over."""
        # Start playing
        game.set_state(GameState.PLAYING)

        # Simulate losing all lives
        game.lives = 0
        game.set_state(GameState.GAME_OVER)

        # Start new game
        game.set_state(GameState.PLAYING)

        assert game.lives == INITIAL_LIVES

    def test_lives_reset_after_victory(self, game):
        """Lives should reset when starting new game after victory."""
        game.set_state(GameState.PLAYING)
        game.lives = 1  # Only one life left
        game.set_state(GameState.VICTORY)

        game.set_state(GameState.PLAYING)

        assert game.lives == INITIAL_LIVES

    def test_reset_game_method_resets_lives(self, game):
        """reset_game() should reset lives."""
        game.lives = 0
        game.reset_game()

        assert game.lives == INITIAL_LIVES

    def test_lives_start_at_initial_value(self, game):
        """Game should start with INITIAL_LIVES."""
        assert game.lives == INITIAL_LIVES
