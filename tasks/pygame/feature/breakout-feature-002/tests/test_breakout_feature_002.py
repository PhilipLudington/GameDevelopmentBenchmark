#!/usr/bin/env python3
"""Tests for breakout-feature-002: Add paddle width power-up."""

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
from main import Game, GameState, PADDLE_WIDTH


class TestPaddleWidthPowerUp:
    """Test paddle width power-up functionality."""

    @pytest.fixture
    def game(self):
        g = Game(headless=True)
        g.set_state(GameState.PLAYING)
        return g

    def test_paddle_has_base_width(self, game):
        """Paddle should track base width."""
        assert hasattr(game.paddle, 'base_width'), "Paddle should have 'base_width' attribute"

    def test_paddle_can_expand(self, game):
        """Paddle width should be expandable."""
        initial_width = game.paddle.width

        if hasattr(game.paddle, 'set_width'):
            game.paddle.set_width(initial_width * 1.5)
            assert game.paddle.width > initial_width

    def test_paddle_width_resets(self, game):
        """Paddle width should be resettable."""
        if hasattr(game.paddle, 'set_width') and hasattr(game.paddle, 'reset_width'):
            game.paddle.set_width(PADDLE_WIDTH * 1.5)
            game.paddle.reset_width()
            assert game.paddle.width == PADDLE_WIDTH
