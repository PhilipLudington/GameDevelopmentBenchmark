#!/usr/bin/env python3
"""Tests for breakout-003: Ball stuck in paddle after collision."""

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
from main import Game, GameState, PADDLE_Y, BALL_RADIUS


class TestPaddleCollision:
    """Test that ball doesn't get stuck in paddle."""

    @pytest.fixture
    def game(self):
        g = Game(headless=True)
        g.set_state(GameState.PLAYING)
        return g

    def test_ball_above_paddle_after_collision(self, game):
        """Ball should be above paddle after bouncing."""
        # Position ball inside paddle (simulating deep penetration)
        game.ball.position.x = game.paddle.center_x
        game.ball.position.y = PADDLE_Y + 5  # Inside paddle
        game.ball.velocity.y = 5  # Moving down

        game.update()

        # Ball should be above paddle top
        assert game.ball.position.y <= PADDLE_Y - BALL_RADIUS

    def test_ball_bounces_up_from_paddle(self, game):
        """Ball should be moving upward after paddle collision."""
        game.ball.position.x = game.paddle.center_x
        game.ball.position.y = PADDLE_Y - BALL_RADIUS - 1
        game.ball.velocity.y = 5  # Moving down

        game.update()

        assert game.ball.velocity.y < 0  # Now moving up

    def test_no_multiple_bounces(self, game):
        """Ball shouldn't bounce multiple times in quick succession."""
        game.ball.position.x = game.paddle.center_x
        game.ball.position.y = PADDLE_Y - BALL_RADIUS - 1
        game.ball.velocity.y = 5

        game.update()
        initial_vel_y = game.ball.velocity.y

        # Run several more frames
        for _ in range(5):
            game.update()

        # Ball should maintain upward velocity (not flip-flop)
        # Either still moving up or hit top wall
        assert game.ball.velocity.y <= 0 or game.ball.position.y <= BALL_RADIUS
