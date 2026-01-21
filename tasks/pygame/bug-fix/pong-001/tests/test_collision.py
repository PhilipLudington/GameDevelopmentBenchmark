"""Tests for the collision detection fix."""

import os
import sys
import pytest

# Set up headless mode
os.environ["SDL_VIDEODRIVER"] = "dummy"
os.environ["SDL_AUDIODRIVER"] = "dummy"

# Add game directory to path
game_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(game_dir, "game"))

from main import Ball, Paddle, PADDLE_WIDTH, PADDLE_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT


class TestCollisionFix:
    """Tests to verify the collision detection fix."""

    def test_ball_does_not_pass_through_left_paddle(self):
        """Ball should not pass through the left paddle."""
        ball = Ball()
        paddle = Paddle(50, SCREEN_HEIGHT // 2 - PADDLE_HEIGHT // 2)

        # Position ball just to the right of paddle, moving left
        ball.rect.right = paddle.rect.left + 5
        ball.rect.centery = paddle.rect.centery
        ball.velocity.x = -10  # Moving left fast

        # Check collision
        collision = ball.check_paddle_collision(paddle)

        assert collision is True
        # Ball should be pushed outside the paddle
        assert ball.rect.left >= paddle.rect.right, "Ball should be outside paddle after collision"
        # Velocity should be reversed (now moving right)
        assert ball.velocity.x > 0, "Ball should be moving right after hitting left paddle"

    def test_ball_does_not_pass_through_right_paddle(self):
        """Ball should not pass through the right paddle."""
        ball = Ball()
        paddle = Paddle(SCREEN_WIDTH - 50 - PADDLE_WIDTH, SCREEN_HEIGHT // 2 - PADDLE_HEIGHT // 2)

        # Position ball just to the left of paddle, moving right
        ball.rect.left = paddle.rect.right - 5
        ball.rect.centery = paddle.rect.centery
        ball.velocity.x = 10  # Moving right fast

        # Check collision
        collision = ball.check_paddle_collision(paddle)

        assert collision is True
        # Ball should be pushed outside the paddle
        assert ball.rect.right <= paddle.rect.left, "Ball should be outside paddle after collision"
        # Velocity should be reversed (now moving left)
        assert ball.velocity.x < 0, "Ball should be moving left after hitting right paddle"

    def test_ball_bounces_at_correct_angle(self):
        """Ball should bounce at an angle based on hit position."""
        ball = Ball()
        paddle = Paddle(50, SCREEN_HEIGHT // 2 - PADDLE_HEIGHT // 2)

        # Hit top of paddle
        ball.rect.right = paddle.rect.left + 5
        ball.rect.centery = paddle.rect.top + 10
        ball.velocity.x = -5
        ball.velocity.y = 2

        ball.check_paddle_collision(paddle)

        # Should bounce upward (negative y velocity)
        assert ball.velocity.y < 0, "Ball should bounce upward when hitting top of paddle"

    def test_ball_bounces_downward_from_bottom_hit(self):
        """Ball should bounce downward when hitting bottom of paddle."""
        ball = Ball()
        paddle = Paddle(50, SCREEN_HEIGHT // 2 - PADDLE_HEIGHT // 2)

        # Hit bottom of paddle
        ball.rect.right = paddle.rect.left + 5
        ball.rect.centery = paddle.rect.bottom - 10
        ball.velocity.x = -5
        ball.velocity.y = -2

        ball.check_paddle_collision(paddle)

        # Should bounce downward (positive y velocity)
        assert ball.velocity.y > 0, "Ball should bounce downward when hitting bottom of paddle"

    def test_no_collision_when_ball_not_touching_paddle(self):
        """No collision should occur when ball is not touching paddle."""
        ball = Ball()
        paddle = Paddle(50, SCREEN_HEIGHT // 2 - PADDLE_HEIGHT // 2)

        # Position ball far from paddle
        ball.rect.centerx = SCREEN_WIDTH // 2
        ball.rect.centery = SCREEN_HEIGHT // 2
        ball.velocity.x = 5
        ball.velocity.y = 3

        original_vx = ball.velocity.x
        original_vy = ball.velocity.y

        collision = ball.check_paddle_collision(paddle)

        assert collision is False
        assert ball.velocity.x == original_vx
        assert ball.velocity.y == original_vy

    def test_fast_ball_collision(self):
        """Fast-moving ball should still collide correctly."""
        ball = Ball()
        paddle = Paddle(50, SCREEN_HEIGHT // 2 - PADDLE_HEIGHT // 2)

        # Position ball overlapping paddle significantly (simulating fast movement)
        ball.rect.centerx = paddle.rect.centerx
        ball.rect.centery = paddle.rect.centery
        ball.velocity.x = -15  # Very fast
        ball.speed = 15

        collision = ball.check_paddle_collision(paddle)

        assert collision is True
        # Ball should be completely outside paddle
        assert not ball.rect.colliderect(paddle.rect), "Ball should not overlap paddle after collision"


class TestGameImports:
    """Basic tests to verify the game module works."""

    def test_game_imports(self):
        """Test that the game module can be imported."""
        from main import Game, GameState, Ball, Paddle
        assert Game is not None
        assert GameState is not None

    def test_ball_creation(self):
        """Test ball can be created."""
        ball = Ball()
        assert ball.rect is not None
        assert ball.velocity is not None

    def test_paddle_creation(self):
        """Test paddle can be created."""
        paddle = Paddle(100, 200)
        assert paddle.rect.x == 100
        assert paddle.rect.y == 200
