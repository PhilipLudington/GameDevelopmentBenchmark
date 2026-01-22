#!/usr/bin/env python3
"""Tests for breakout-001: Ball passes through brick corners."""

import os
import sys

# Set up headless mode BEFORE importing pygame
os.environ["SDL_VIDEODRIVER"] = "dummy"
os.environ["SDL_AUDIODRIVER"] = "dummy"

# Add game directory to path (can be overridden by setting TEST_GAME_DIR env var)
game_dir = os.environ.get(
    "TEST_GAME_DIR",
    os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "game")
)
sys.path.insert(0, game_dir)

import pytest
from main import Game, GameState, BRICK_WIDTH, BRICK_HEIGHT, BALL_RADIUS


class TestBrickCollisionPositionCorrection:
    """Test that ball position is corrected after brick collision."""

    @pytest.fixture
    def game(self):
        """Create a game instance for testing."""
        g = Game(headless=True)
        g.set_state(GameState.PLAYING)
        return g

    def test_ball_not_overlapping_after_vertical_collision(self, game):
        """Ball should not overlap with destroyed brick after vertical hit."""
        # Get the first brick in bottom row (easier to test)
        target_brick = None
        for brick in game.bricks:
            if brick.row == 4:  # Bottom row
                target_brick = brick
                break

        assert target_brick is not None

        # Position ball just below the brick, moving up
        game.ball.position.x = target_brick.x + BRICK_WIDTH / 2
        game.ball.position.y = target_brick.y + BRICK_HEIGHT + BALL_RADIUS + 2
        game.ball.velocity.x = 0
        game.ball.velocity.y = -10  # Moving up fast

        # Run update to trigger collision
        game.update()

        # Brick should be destroyed
        assert not target_brick.alive

        # Ball should not overlap with the brick's original position
        ball_rect = game.ball.rect
        brick_rect = target_brick.rect
        assert not ball_rect.colliderect(brick_rect), \
            f"Ball still overlapping brick after collision. Ball: {ball_rect}, Brick: {brick_rect}"

    def test_ball_not_overlapping_after_horizontal_collision(self, game):
        """Ball should not overlap with destroyed brick after horizontal hit."""
        # Find a brick
        target_brick = game.bricks[25]  # Middle of the grid

        # Position ball just inside the left edge of brick, moving right
        # This simulates a fast ball that penetrated into the brick
        game.ball.position.x = target_brick.x + BALL_RADIUS - 2
        game.ball.position.y = target_brick.y + BRICK_HEIGHT / 2
        game.ball.velocity.x = 10  # Moving right fast
        game.ball.velocity.y = 0

        # Run update to trigger collision
        game.update()

        # Brick should be destroyed
        assert not target_brick.alive

        # Ball should not overlap with the brick's original position
        ball_rect = game.ball.rect
        brick_rect = target_brick.rect
        assert not ball_rect.colliderect(brick_rect), \
            f"Ball still overlapping brick after collision. Ball: {ball_rect}, Brick: {brick_rect}"

    def test_ball_pushed_below_brick_after_top_hit(self, game):
        """Ball hitting brick from below should be pushed below it."""
        target_brick = game.bricks[40]  # Bottom row

        # Position ball below brick, moving up
        game.ball.position.x = target_brick.x + BRICK_WIDTH / 2
        game.ball.position.y = target_brick.y + BRICK_HEIGHT + BALL_RADIUS - 3  # Slightly inside
        game.ball.velocity.x = 0
        game.ball.velocity.y = -8

        game.update()

        assert not target_brick.alive
        # Ball should be below the brick
        assert game.ball.position.y > target_brick.y + BRICK_HEIGHT

    def test_ball_pushed_above_brick_after_bottom_hit(self, game):
        """Ball hitting brick from above should be pushed above it."""
        target_brick = game.bricks[0]  # Top row

        # Position ball above brick, moving down
        game.ball.position.x = target_brick.x + BRICK_WIDTH / 2
        game.ball.position.y = target_brick.y - BALL_RADIUS + 3  # Slightly inside
        game.ball.velocity.x = 0
        game.ball.velocity.y = 8

        game.update()

        assert not target_brick.alive
        # Ball should be above the brick
        assert game.ball.position.y < target_brick.y

    def test_ball_pushed_left_after_right_side_hit(self, game):
        """Ball hitting brick from right should be pushed right."""
        target_brick = game.bricks[5]

        # Position ball to right of brick, moving left
        game.ball.position.x = target_brick.x + BRICK_WIDTH + BALL_RADIUS - 3
        game.ball.position.y = target_brick.y + BRICK_HEIGHT / 2
        game.ball.velocity.x = -8
        game.ball.velocity.y = 0

        game.update()

        assert not target_brick.alive
        # Ball should be to the right of the brick
        assert game.ball.position.x > target_brick.x + BRICK_WIDTH

    def test_ball_pushed_right_after_left_side_hit(self, game):
        """Ball hitting brick from left should be pushed left."""
        target_brick = game.bricks[5]

        # Position ball to left of brick, moving right
        game.ball.position.x = target_brick.x - BALL_RADIUS + 3
        game.ball.position.y = target_brick.y + BRICK_HEIGHT / 2
        game.ball.velocity.x = 8
        game.ball.velocity.y = 0

        game.update()

        assert not target_brick.alive
        # Ball should be to the left of the brick
        assert game.ball.position.x < target_brick.x

    def test_high_speed_collision_no_tunneling(self, game):
        """Ball moving at high speed should not tunnel through bricks."""
        target_brick = game.bricks[25]

        # Position ball far from brick, moving very fast toward it
        game.ball.position.x = target_brick.x + BRICK_WIDTH / 2
        game.ball.position.y = target_brick.y + BRICK_HEIGHT + BALL_RADIUS + 1
        game.ball.velocity.x = 0
        game.ball.velocity.y = -15  # Very fast

        game.update()

        # After collision, ball should be outside the brick
        ball_rect = game.ball.rect
        brick_rect = target_brick.rect
        assert not ball_rect.colliderect(brick_rect)


class TestBrickCollisionBounce:
    """Test that ball bounces correctly after brick collision."""

    @pytest.fixture
    def game(self):
        g = Game(headless=True)
        g.set_state(GameState.PLAYING)
        return g

    def test_vertical_bounce_reverses_y_velocity(self, game):
        """Ball hitting top/bottom of brick should reverse y velocity."""
        target_brick = game.bricks[40]

        # Moving up into brick
        game.ball.position.x = target_brick.x + BRICK_WIDTH / 2
        game.ball.position.y = target_brick.y + BRICK_HEIGHT + BALL_RADIUS - 1
        game.ball.velocity.x = 0
        game.ball.velocity.y = -5

        game.update()

        # Y velocity should now be positive (bounced down)
        assert game.ball.velocity.y > 0

    def test_horizontal_bounce_reverses_x_velocity(self, game):
        """Ball hitting left/right of brick should reverse x velocity."""
        target_brick = game.bricks[25]

        # Moving right into brick
        game.ball.position.x = target_brick.x - BALL_RADIUS + 1
        game.ball.position.y = target_brick.y + BRICK_HEIGHT / 2
        game.ball.velocity.x = 5
        game.ball.velocity.y = 0

        game.update()

        # X velocity should now be negative (bounced left)
        assert game.ball.velocity.x < 0
