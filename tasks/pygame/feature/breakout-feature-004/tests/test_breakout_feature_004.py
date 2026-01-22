#!/usr/bin/env python3
"""Tests for breakout-feature-004: Add multi-hit bricks."""

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
from main import Game, GameState, BRICK_WIDTH, BRICK_HEIGHT, BALL_RADIUS


class TestMultiHitBricks:
    """Test multi-hit brick functionality."""

    @pytest.fixture
    def game(self):
        g = Game(headless=True)
        g.set_state(GameState.PLAYING)
        return g

    def test_bricks_have_health(self, game):
        """Bricks should have health attribute."""
        brick = game.bricks[0]
        assert hasattr(brick, 'health'), "Bricks should have 'health' attribute"

    def test_top_row_has_more_health(self, game):
        """Top row bricks should have more health than bottom."""
        if not hasattr(game.bricks[0], 'health'):
            pytest.skip("Multi-hit bricks not implemented")

        top_brick = game.bricks[0]  # Row 0
        bottom_brick = game.bricks[40]  # Row 4

        assert top_brick.health > bottom_brick.health

    def test_brick_takes_damage(self, game):
        """Hitting a brick should reduce its health."""
        if not hasattr(game.bricks[0], 'health'):
            pytest.skip("Multi-hit bricks not implemented")

        brick = game.bricks[0]
        initial_health = brick.health

        if hasattr(brick, 'take_damage'):
            brick.take_damage()
            assert brick.health < initial_health

    def test_brick_destroyed_at_zero_health(self, game):
        """Brick should be destroyed when health reaches 0."""
        if not hasattr(game.bricks[0], 'health'):
            pytest.skip("Multi-hit bricks not implemented")

        brick = game.bricks[0]
        brick.health = 1

        if hasattr(brick, 'take_damage'):
            brick.take_damage()
            assert not brick.alive
