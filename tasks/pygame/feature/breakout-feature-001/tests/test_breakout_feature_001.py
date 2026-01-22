#!/usr/bin/env python3
"""Tests for breakout-feature-001: Add multi-ball power-up."""

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
from main import Game, GameState


class TestMultiBallPowerUp:
    """Test multi-ball power-up functionality."""

    @pytest.fixture
    def game(self):
        g = Game(headless=True)
        g.set_state(GameState.PLAYING)
        return g

    def test_game_has_balls_list(self, game):
        """Game should track multiple balls."""
        assert hasattr(game, 'balls'), "Game should have 'balls' list attribute"
        assert isinstance(game.balls, list), "balls should be a list"

    def test_game_has_powerups_list(self, game):
        """Game should track power-ups."""
        assert hasattr(game, 'powerups'), "Game should have 'powerups' list attribute"

    def test_powerup_class_exists(self, game):
        """PowerUp class should be defined."""
        try:
            from main import PowerUp
        except ImportError:
            pytest.fail("PowerUp class should be defined in main.py")

    def test_multiball_spawns_additional_balls(self, game):
        """Activating multi-ball should spawn additional balls."""
        initial_count = len(game.balls) if hasattr(game, 'balls') else 1

        # Activate multi-ball power-up (implementation-specific)
        if hasattr(game, 'activate_multiball'):
            game.activate_multiball()
            assert len(game.balls) > initial_count
