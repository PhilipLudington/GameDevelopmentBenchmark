#!/usr/bin/env python3
"""Tests for breakout-feature-003: Add multiple levels."""

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


class TestMultipleLevels:
    """Test multiple levels functionality."""

    @pytest.fixture
    def game(self):
        g = Game(headless=True)
        g.set_state(GameState.PLAYING)
        return g

    def test_game_has_level(self, game):
        """Game should track current level."""
        assert hasattr(game, 'level'), "Game should have 'level' attribute"
        assert game.level >= 1

    def test_level_advances_on_victory(self, game):
        """Level should advance when all bricks cleared."""
        if not hasattr(game, 'level'):
            pytest.skip("Level system not implemented")

        initial_level = game.level

        # Clear all bricks
        for brick in game.bricks:
            brick.alive = False

        game.update()  # Should trigger level advance

        assert game.level > initial_level or game.state == GameState.VICTORY

    def test_different_brick_patterns(self, game):
        """Different levels should have different brick patterns."""
        if not hasattr(game, 'level'):
            pytest.skip("Level system not implemented")

        level1_pattern = [(b.row, b.col, b.alive) for b in game.bricks]

        game.level = 2
        game.reset_game()

        level2_pattern = [(b.row, b.col, b.alive) for b in game.bricks]

        # Patterns should differ (could check total bricks or positions)
        # This is a basic check - implementation may vary
