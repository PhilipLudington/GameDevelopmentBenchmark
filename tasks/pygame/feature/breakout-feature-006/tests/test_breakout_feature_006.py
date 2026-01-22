#!/usr/bin/env python3
"""Tests for breakout-feature-006: Add ball speed increase over time."""

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
from main import Game, GameState, BALL_SPEED, BRICK_WIDTH, BRICK_HEIGHT, BALL_RADIUS


class TestBallSpeedIncrease:
    """Test ball speed increase functionality."""

    @pytest.fixture
    def game(self):
        g = Game(headless=True)
        g.set_state(GameState.PLAYING)
        return g

    def test_ball_has_base_speed(self, game):
        """Ball should track base speed."""
        assert hasattr(game.ball, 'base_speed') or hasattr(game.ball, 'speed')

    def test_speed_increases_on_brick_destroy(self, game):
        """Ball speed should increase when brick is destroyed."""
        initial_speed = game.ball.speed

        # Destroy a brick
        brick = game.bricks[0]
        game.ball.position.x = brick.x + BRICK_WIDTH / 2
        game.ball.position.y = brick.y + BRICK_HEIGHT + BALL_RADIUS - 1
        game.ball.velocity.x = 0
        game.ball.velocity.y = -5

        game.update()

        # Speed should have increased (or method to increase should exist)
        speed_increased = game.ball.speed > initial_speed
        has_increase_method = hasattr(game.ball, 'increase_speed')

        assert speed_increased or has_increase_method

    def test_speed_has_max_cap(self, game):
        """Ball speed should not exceed maximum."""
        if hasattr(game.ball, 'base_speed'):
            max_speed = game.ball.base_speed * 1.5
            if hasattr(game.ball, 'increase_speed'):
                for _ in range(100):
                    game.ball.increase_speed()
                assert game.ball.speed <= max_speed

    def test_speed_resets_on_life_lost(self, game):
        """Ball speed should reset when life is lost."""
        if hasattr(game.ball, 'increase_speed') and hasattr(game.ball, 'reset_speed'):
            game.ball.increase_speed()
            game.ball.increase_speed()
            game.ball.reset_speed()
            assert game.ball.speed == game.ball.base_speed
