#!/usr/bin/env python3
"""Tests for breakout-006: Paddle moves off screen at edges."""

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
from main import Game, GameState, SCREEN_WIDTH, PADDLE_WIDTH


class TestPaddleBoundary:
    """Test that paddle stays within screen bounds."""

    @pytest.fixture
    def game(self):
        g = Game(headless=True)
        g.set_state(GameState.PLAYING)
        return g

    def test_paddle_left_boundary(self, game):
        """Paddle left edge should never go below 0."""
        game.paddle.x = 5

        for _ in range(10):
            game.paddle.move_left()

        assert game.paddle.x >= 0

    def test_paddle_right_boundary(self, game):
        """Paddle right edge should never exceed screen width."""
        game.paddle.x = SCREEN_WIDTH - PADDLE_WIDTH - 5

        for _ in range(10):
            game.paddle.move_right()

        assert game.paddle.x + game.paddle.width <= SCREEN_WIDTH

    def test_paddle_fully_visible_left(self, game):
        """Paddle should be fully visible at left edge."""
        game.paddle.x = 0
        game.paddle.move_left()

        assert game.paddle.x == 0
        assert game.paddle.rect.left >= 0

    def test_paddle_fully_visible_right(self, game):
        """Paddle should be fully visible at right edge."""
        game.paddle.x = SCREEN_WIDTH - PADDLE_WIDTH
        game.paddle.move_right()

        assert game.paddle.x == SCREEN_WIDTH - PADDLE_WIDTH
        assert game.paddle.rect.right <= SCREEN_WIDTH
