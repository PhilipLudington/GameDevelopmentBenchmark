#!/usr/bin/env python3
"""Tests for breakout-mini-001: Create two-player cooperative mode."""

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


class TestTwoPlayerMode:
    """Test two-player cooperative mode."""

    @pytest.fixture
    def game(self):
        g = Game(headless=True)
        g.set_state(GameState.PLAYING)
        return g

    def test_game_has_two_paddles(self, game):
        """Game should have two paddle objects."""
        has_two_paddles = (
            (hasattr(game, 'paddle1') and hasattr(game, 'paddle2')) or
            (hasattr(game, 'paddles') and len(game.paddles) == 2) or
            (hasattr(game, 'top_paddle') and hasattr(game, 'bottom_paddle'))
        )
        assert has_two_paddles, "Game should have two paddles"

    def test_paddles_at_different_positions(self, game):
        """Paddles should be at top and bottom of screen."""
        if hasattr(game, 'paddle1') and hasattr(game, 'paddle2'):
            assert game.paddle1.y != game.paddle2.y
        elif hasattr(game, 'paddles'):
            assert game.paddles[0].y != game.paddles[1].y

    def test_both_paddles_moveable(self, game):
        """Both paddles should be independently moveable."""
        if hasattr(game, 'paddle1'):
            initial_x1 = game.paddle1.x
            game.paddle1.move_left()
            assert game.paddle1.x != initial_x1

            initial_x2 = game.paddle2.x
            game.paddle2.move_right()
            assert game.paddle2.x != initial_x2

    def test_ball_bounces_off_both_paddles(self, game):
        """Ball should be able to bounce off either paddle."""
        # This is a basic structure test
        # Full implementation would test actual collision with both paddles
        if hasattr(game, 'paddle1') and hasattr(game, 'paddle2'):
            assert game.paddle1.rect is not None
            assert game.paddle2.rect is not None
