#!/usr/bin/env python3
"""Tests for breakout-005: Ball escapes through screen corners."""

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
from main import Game, GameState, SCREEN_WIDTH, SCREEN_HEIGHT, BALL_RADIUS


class TestCornerBounce:
    """Test that ball correctly handles corner collisions."""

    @pytest.fixture
    def game(self):
        g = Game(headless=True)
        g.set_state(GameState.PLAYING)
        return g

    def test_ball_stays_in_bounds_top_left(self, game):
        """Ball hitting top-left corner should stay in bounds."""
        game.ball.position.x = BALL_RADIUS + 1
        game.ball.position.y = BALL_RADIUS + 1
        game.ball.velocity.x = -10
        game.ball.velocity.y = -10

        for _ in range(10):
            game.update()
            assert game.ball.position.x >= BALL_RADIUS
            assert game.ball.position.y >= BALL_RADIUS

    def test_ball_stays_in_bounds_top_right(self, game):
        """Ball hitting top-right corner should stay in bounds."""
        game.ball.position.x = SCREEN_WIDTH - BALL_RADIUS - 1
        game.ball.position.y = BALL_RADIUS + 1
        game.ball.velocity.x = 10
        game.ball.velocity.y = -10

        for _ in range(10):
            game.update()
            assert game.ball.position.x <= SCREEN_WIDTH - BALL_RADIUS
            assert game.ball.position.y >= BALL_RADIUS

    def test_both_axes_corrected_simultaneously(self, game):
        """Ball at corner should have both axes corrected."""
        # Place ball in corner violation
        game.ball.position.x = -5
        game.ball.position.y = -5
        game.ball.velocity.x = -10
        game.ball.velocity.y = -10

        game.update()

        # Both should be corrected
        assert game.ball.position.x >= BALL_RADIUS
        assert game.ball.position.y >= BALL_RADIUS
        # Both velocities should be reversed
        assert game.ball.velocity.x > 0
        assert game.ball.velocity.y > 0
