#!/usr/bin/env python3
"""Tests for breakout-002: Score callback receives wrong value."""

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
from main import Game, GameState, BRICK_WIDTH, BRICK_HEIGHT, BALL_RADIUS, POINTS_PER_BRICK, BRICK_ROWS


class TestScoreCallback:
    """Test that on_score callback receives correct value."""

    @pytest.fixture
    def game(self):
        g = Game(headless=True)
        g.set_state(GameState.PLAYING)
        return g

    def test_score_callback_receives_points_not_total(self, game):
        """on_score should receive points earned, not total score."""
        callback_values = []
        game.on_score = lambda points: callback_values.append(points)

        # Hit a brick in the top row (worth most points)
        top_brick = game.bricks[0]  # Row 0
        expected_points = POINTS_PER_BRICK * (BRICK_ROWS - 0)  # 50 points

        game.ball.position.x = top_brick.x + BRICK_WIDTH / 2
        game.ball.position.y = top_brick.y + BRICK_HEIGHT + BALL_RADIUS - 1
        game.ball.velocity.x = 0
        game.ball.velocity.y = -5

        game.update()

        assert len(callback_values) == 1
        # The callback should receive the points earned (50), not the total score
        assert callback_values[0] == expected_points
        assert callback_values[0] != game.score or game.score == expected_points

    def test_score_callback_receives_correct_points_for_each_row(self, game):
        """Points should vary by row (higher rows worth more)."""
        callback_values = []
        game.on_score = lambda points: callback_values.append(points)

        # Test bottom row brick (worth least)
        bottom_brick = game.bricks[40]  # Row 4
        expected_bottom_points = POINTS_PER_BRICK * (BRICK_ROWS - 4)  # 10 points

        game.ball.position.x = bottom_brick.x + BRICK_WIDTH / 2
        game.ball.position.y = bottom_brick.y + BRICK_HEIGHT + BALL_RADIUS - 1
        game.ball.velocity.x = 0
        game.ball.velocity.y = -5

        game.update()

        assert len(callback_values) == 1
        assert callback_values[0] == expected_bottom_points

    def test_callback_not_cumulative(self, game):
        """Each callback should only report that brick's points."""
        callback_values = []
        game.on_score = lambda points: callback_values.append(points)

        # Destroy first brick
        brick1 = game.bricks[0]
        game.ball.position.x = brick1.x + BRICK_WIDTH / 2
        game.ball.position.y = brick1.y + BRICK_HEIGHT + BALL_RADIUS - 1
        game.ball.velocity.x = 0
        game.ball.velocity.y = -5
        game.update()

        first_points = callback_values[0]

        # Destroy second brick in same row
        brick2 = game.bricks[1]
        game.ball.position.x = brick2.x + BRICK_WIDTH / 2
        game.ball.position.y = brick2.y + BRICK_HEIGHT + BALL_RADIUS - 1
        game.ball.velocity.x = 0
        game.ball.velocity.y = -5
        game.update()

        # Second callback should be same as first (same row = same points)
        # NOT the cumulative total
        assert len(callback_values) == 2
        assert callback_values[1] == first_points

    def test_total_score_still_accumulates(self, game):
        """Total score should still accumulate correctly."""
        # Destroy two bricks
        brick1 = game.bricks[0]
        brick2 = game.bricks[1]

        game.ball.position.x = brick1.x + BRICK_WIDTH / 2
        game.ball.position.y = brick1.y + BRICK_HEIGHT + BALL_RADIUS - 1
        game.ball.velocity.x = 0
        game.ball.velocity.y = -5
        game.update()

        first_score = game.score

        game.ball.position.x = brick2.x + BRICK_WIDTH / 2
        game.ball.position.y = brick2.y + BRICK_HEIGHT + BALL_RADIUS - 1
        game.ball.velocity.x = 0
        game.ball.velocity.y = -5
        game.update()

        # Total score should be sum of both bricks
        assert game.score == first_score * 2  # Same row, same points
