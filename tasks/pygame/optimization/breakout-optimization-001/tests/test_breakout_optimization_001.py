#!/usr/bin/env python3
"""Tests for breakout-optimization-001: Optimize brick collision detection."""

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


class TestOptimizedCollision:
    """Test optimized collision detection."""

    @pytest.fixture
    def game(self):
        g = Game(headless=True)
        g.set_state(GameState.PLAYING)
        return g

    def test_collision_still_works(self, game):
        """Optimization should not break collision detection."""
        brick = game.bricks[25]

        game.ball.position.x = brick.x + BRICK_WIDTH / 2
        game.ball.position.y = brick.y + BRICK_HEIGHT + BALL_RADIUS - 1
        game.ball.velocity.x = 0
        game.ball.velocity.y = -5

        game.update()

        assert not brick.alive

    def test_all_bricks_can_be_hit(self, game):
        """All bricks should still be hittable."""
        # Test a few bricks across the grid
        test_indices = [0, 24, 49]  # First, middle, last

        for idx in test_indices:
            game.reset_game()
            brick = game.bricks[idx]

            game.ball.position.x = brick.x + BRICK_WIDTH / 2
            game.ball.position.y = brick.y + BRICK_HEIGHT + BALL_RADIUS - 1
            game.ball.velocity.x = 0
            game.ball.velocity.y = -5

            game.update()

            assert not brick.alive, f"Brick at index {idx} should be destroyed"

    def test_game_has_optimization(self, game):
        """Game should have optimization-related method or structure."""
        # Check for common optimization patterns
        has_optimization = (
            hasattr(game, '_get_nearby_bricks') or
            hasattr(game, 'brick_grid') or
            hasattr(game, '_check_brick_collision') or
            hasattr(game, 'spatial_hash')
        )
        # This is informational - not required to pass
