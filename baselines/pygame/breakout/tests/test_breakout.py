#!/usr/bin/env python3
"""Test suite for Breakout baseline game."""

import os
import sys

# Set up headless mode BEFORE importing pygame
os.environ["SDL_VIDEODRIVER"] = "dummy"
os.environ["SDL_AUDIODRIVER"] = "dummy"

# Add parent directory to path for imports
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

import pytest
from main import (
    Game, GameState, Paddle, Ball, Brick, Vector2,
    SCREEN_WIDTH, SCREEN_HEIGHT, PADDLE_WIDTH, PADDLE_SPEED,
    BRICK_ROWS, BRICK_COLS, BALL_RADIUS, BALL_SPEED, INITIAL_LIVES,
    POINTS_PER_BRICK, PADDLE_Y
)


class TestVector2:
    """Tests for Vector2 dataclass."""

    def test_creation(self):
        v = Vector2(3, 4)
        assert v.x == 3
        assert v.y == 4

    def test_addition(self):
        v1 = Vector2(1, 2)
        v2 = Vector2(3, 4)
        result = v1 + v2
        assert result.x == 4
        assert result.y == 6

    def test_subtraction(self):
        v1 = Vector2(5, 7)
        v2 = Vector2(2, 3)
        result = v1 - v2
        assert result.x == 3
        assert result.y == 4

    def test_multiplication(self):
        v = Vector2(3, 4)
        result = v * 2
        assert result.x == 6
        assert result.y == 8

    def test_copy(self):
        v1 = Vector2(1, 2)
        v2 = v1.copy()
        v2.x = 10
        assert v1.x == 1
        assert v2.x == 10


class TestPaddle:
    """Tests for Paddle class."""

    def test_initialization(self):
        paddle = Paddle()
        assert paddle.width == PADDLE_WIDTH
        assert paddle.y == PADDLE_Y
        # Should be centered
        assert paddle.x == (SCREEN_WIDTH - PADDLE_WIDTH) // 2

    def test_move_left(self):
        paddle = Paddle()
        initial_x = paddle.x
        paddle.move_left()
        assert paddle.x == initial_x - PADDLE_SPEED

    def test_move_right(self):
        paddle = Paddle()
        initial_x = paddle.x
        paddle.move_right()
        assert paddle.x == initial_x + PADDLE_SPEED

    def test_left_boundary(self):
        paddle = Paddle()
        paddle.x = 5
        paddle.move_left()
        assert paddle.x == 0

    def test_right_boundary(self):
        paddle = Paddle()
        paddle.x = SCREEN_WIDTH - PADDLE_WIDTH - 5
        paddle.move_right()
        assert paddle.x == SCREEN_WIDTH - PADDLE_WIDTH

    def test_reset(self):
        paddle = Paddle()
        paddle.x = 100
        paddle.reset()
        assert paddle.x == (SCREEN_WIDTH - PADDLE_WIDTH) // 2

    def test_center_x(self):
        paddle = Paddle()
        expected_center = paddle.x + paddle.width / 2
        assert paddle.center_x == expected_center

    def test_rect(self):
        paddle = Paddle()
        rect = paddle.rect
        assert rect.x == paddle.x
        assert rect.y == paddle.y
        assert rect.width == paddle.width
        assert rect.height == paddle.height


class TestBall:
    """Tests for Ball class."""

    def test_initialization(self):
        ball = Ball()
        assert ball.radius == BALL_RADIUS
        assert ball.speed == BALL_SPEED

    def test_reset(self):
        ball = Ball()
        ball.position = Vector2(100, 100)
        ball.velocity = Vector2(0, 0)
        ball.reset()
        assert ball.position.x == SCREEN_WIDTH // 2
        # Velocity should have some magnitude
        assert ball.velocity.x != 0 or ball.velocity.y != 0

    def test_update(self):
        ball = Ball()
        ball.position = Vector2(100, 100)
        ball.velocity = Vector2(5, 3)
        ball.update()
        assert ball.position.x == 105
        assert ball.position.y == 103

    def test_bounce_horizontal(self):
        ball = Ball()
        ball.velocity = Vector2(5, 3)
        ball.bounce_horizontal()
        assert ball.velocity.x == -5
        assert ball.velocity.y == 3

    def test_bounce_vertical(self):
        ball = Ball()
        ball.velocity = Vector2(5, 3)
        ball.bounce_vertical()
        assert ball.velocity.x == 5
        assert ball.velocity.y == -3

    def test_bounce_off_paddle_center(self):
        ball = Ball()
        paddle = Paddle()
        ball.position.x = paddle.center_x  # Hit center
        ball.velocity.y = ball.speed  # Moving down
        ball.bounce_off_paddle(paddle)
        # After center hit, should go mostly straight up
        assert ball.velocity.y < 0
        assert abs(ball.velocity.x) < ball.speed * 0.1

    def test_bounce_off_paddle_left_edge(self):
        ball = Ball()
        paddle = Paddle()
        ball.position.x = paddle.x  # Hit left edge
        ball.bounce_off_paddle(paddle)
        # Should bounce left and up
        assert ball.velocity.x < 0
        assert ball.velocity.y < 0

    def test_bounce_off_paddle_right_edge(self):
        ball = Ball()
        paddle = Paddle()
        ball.position.x = paddle.x + paddle.width  # Hit right edge
        ball.bounce_off_paddle(paddle)
        # Should bounce right and up
        assert ball.velocity.x > 0
        assert ball.velocity.y < 0

    def test_rect(self):
        ball = Ball()
        ball.position = Vector2(100, 100)
        rect = ball.rect
        assert rect.x == 100 - ball.radius
        assert rect.y == 100 - ball.radius
        assert rect.width == ball.radius * 2
        assert rect.height == ball.radius * 2


class TestBrick:
    """Tests for Brick class."""

    def test_creation(self):
        brick = Brick(row=0, col=0, color=(255, 0, 0))
        assert brick.row == 0
        assert brick.col == 0
        assert brick.alive is True

    def test_position_calculation(self):
        brick1 = Brick(row=0, col=0, color=(255, 0, 0))
        brick2 = Brick(row=1, col=1, color=(255, 0, 0))
        # Second brick should be offset
        assert brick2.x > brick1.x
        assert brick2.y > brick1.y

    def test_points_decrease_by_row(self):
        top_brick = Brick(row=0, col=0, color=(255, 0, 0))
        bottom_brick = Brick(row=BRICK_ROWS - 1, col=0, color=(0, 255, 0))
        # Top row worth more
        assert top_brick.points > bottom_brick.points

    def test_rect(self):
        brick = Brick(row=0, col=0, color=(255, 0, 0))
        rect = brick.rect
        assert rect.x == brick.x
        assert rect.y == brick.y


class TestGame:
    """Tests for Game class."""

    @pytest.fixture
    def game(self):
        """Create a game instance for testing."""
        return Game(headless=True)

    def test_initialization(self, game):
        assert game.state == GameState.MENU
        assert game.score == 0
        assert game.lives == INITIAL_LIVES
        assert len(game.bricks) == BRICK_ROWS * BRICK_COLS

    def test_all_bricks_alive_initially(self, game):
        assert all(brick.alive for brick in game.bricks)

    def test_create_bricks(self, game):
        # Destroy some bricks
        game.bricks[0].alive = False
        game.bricks[5].alive = False
        # Recreate
        game._create_bricks()
        assert len(game.bricks) == BRICK_ROWS * BRICK_COLS
        assert all(brick.alive for brick in game.bricks)

    def test_state_change_menu_to_playing(self, game):
        state_changes = []
        game.on_state_change = lambda old, new: state_changes.append((old, new))

        game.set_state(GameState.PLAYING)

        assert game.state == GameState.PLAYING
        assert len(state_changes) == 1
        assert state_changes[0] == (GameState.MENU, GameState.PLAYING)

    def test_state_change_same_state_no_callback(self, game):
        game.set_state(GameState.PLAYING)
        state_changes = []
        game.on_state_change = lambda old, new: state_changes.append((old, new))

        game.set_state(GameState.PLAYING)  # Same state

        assert len(state_changes) == 0

    def test_pause_toggle(self, game):
        game.set_state(GameState.PLAYING)
        game.set_state(GameState.PAUSED)
        assert game.state == GameState.PAUSED

        game.set_state(GameState.PLAYING)
        assert game.state == GameState.PLAYING

    def test_reset_game(self, game):
        game.set_state(GameState.PLAYING)
        game.score = 500
        game.lives = 1
        game.bricks[0].alive = False

        game.reset_game()

        assert game.score == 0
        assert game.lives == INITIAL_LIVES
        assert all(brick.alive for brick in game.bricks)

    def test_reset_ball(self, game):
        game.ball.position = Vector2(100, 100)
        game.paddle.x = 100

        game.reset_ball()

        assert game.ball.position.x == SCREEN_WIDTH // 2
        assert game.paddle.x == (SCREEN_WIDTH - PADDLE_WIDTH) // 2

    def test_high_score_update_on_game_over(self, game):
        game.set_state(GameState.PLAYING)
        game.score = 100
        game.high_score = 50

        game.set_state(GameState.GAME_OVER)

        assert game.high_score == 100

    def test_high_score_not_lowered(self, game):
        game.high_score = 200
        game.set_state(GameState.PLAYING)
        game.score = 100

        game.set_state(GameState.GAME_OVER)

        assert game.high_score == 200

    def test_step_returns_state(self, game):
        game.set_state(GameState.PLAYING)
        result = game.step()

        assert "state" in result
        assert "score" in result
        assert "lives" in result
        assert "ball_position" in result
        assert "paddle_position" in result
        assert "bricks_remaining" in result
        assert "frame_count" in result
        assert result["state"] == "playing"
        assert result["bricks_remaining"] == BRICK_ROWS * BRICK_COLS


class TestGameplay:
    """Integration tests for gameplay mechanics."""

    @pytest.fixture
    def game(self):
        """Create a game in playing state."""
        g = Game(headless=True)
        g.set_state(GameState.PLAYING)
        return g

    def test_ball_wall_bounce_left(self, game):
        game.ball.position = Vector2(BALL_RADIUS, 300)
        game.ball.velocity = Vector2(-5, 0)

        game.update()

        assert game.ball.velocity.x > 0  # Now moving right

    def test_ball_wall_bounce_right(self, game):
        game.ball.position = Vector2(SCREEN_WIDTH - BALL_RADIUS, 300)
        game.ball.velocity = Vector2(5, 0)

        game.update()

        assert game.ball.velocity.x < 0  # Now moving left

    def test_ball_wall_bounce_top(self, game):
        game.ball.position = Vector2(400, BALL_RADIUS)
        game.ball.velocity = Vector2(0, -5)

        game.update()

        assert game.ball.velocity.y > 0  # Now moving down

    def test_ball_falls_loses_life(self, game):
        lives_lost = []
        game.on_life_lost = lambda lives: lives_lost.append(lives)

        game.ball.position = Vector2(400, SCREEN_HEIGHT + BALL_RADIUS)
        game.ball.velocity = Vector2(0, 5)
        initial_lives = game.lives

        game.update()

        assert game.lives == initial_lives - 1
        assert len(lives_lost) == 1

    def test_ball_falls_game_over_when_no_lives(self, game):
        game.lives = 1
        game.ball.position = Vector2(400, SCREEN_HEIGHT + BALL_RADIUS)
        game.ball.velocity = Vector2(0, 5)

        game.update()

        assert game.state == GameState.GAME_OVER

    def test_brick_destroyed_on_collision(self, game):
        destroyed = []
        game.on_brick_destroyed = lambda brick: destroyed.append(brick)

        # Position ball to hit first brick
        target_brick = game.bricks[0]
        game.ball.position = Vector2(
            target_brick.x + 35,  # Center of brick
            target_brick.y + 30   # Just below brick
        )
        game.ball.velocity = Vector2(0, -5)

        game.update()

        assert not target_brick.alive
        assert len(destroyed) == 1

    def test_score_increases_on_brick_destroy(self, game):
        scores = []
        game.on_score = lambda points: scores.append(points)

        target_brick = game.bricks[0]
        game.ball.position = Vector2(
            target_brick.x + 35,
            target_brick.y + 30
        )
        game.ball.velocity = Vector2(0, -5)
        initial_score = game.score

        game.update()

        assert game.score > initial_score
        assert len(scores) == 1

    def test_victory_when_all_bricks_destroyed(self, game):
        # Destroy all but one brick
        for brick in game.bricks[:-1]:
            brick.alive = False

        # Position ball to hit last brick
        last_brick = game.bricks[-1]
        game.ball.position = Vector2(
            last_brick.x + 35,
            last_brick.y + 30
        )
        game.ball.velocity = Vector2(0, -5)

        game.update()

        assert game.state == GameState.VICTORY

    def test_paddle_collision(self, game):
        # Position ball above paddle, moving down
        game.ball.position = Vector2(
            game.paddle.center_x,
            game.paddle.y - BALL_RADIUS - 1
        )
        game.ball.velocity = Vector2(0, 5)

        game.update()

        # Ball should bounce up
        assert game.ball.velocity.y < 0

    def test_game_runs_frames(self, game):
        result = None
        for _ in range(10):
            result = game.step()

        assert result["frame_count"] == 10
        assert game.running


class TestGameStateTransitions:
    """Test game state machine transitions."""

    @pytest.fixture
    def game(self):
        return Game(headless=True)

    def test_menu_to_playing_resets_game(self, game):
        game.score = 100
        game.set_state(GameState.PLAYING)
        assert game.score == 0

    def test_game_over_to_playing_resets_game(self, game):
        game.set_state(GameState.PLAYING)
        game.score = 100
        game.lives = 0
        game.set_state(GameState.GAME_OVER)

        game.set_state(GameState.PLAYING)

        assert game.score == 0
        assert game.lives == INITIAL_LIVES

    def test_victory_to_playing_resets_game(self, game):
        game.set_state(GameState.PLAYING)
        game.score = 1000
        for brick in game.bricks:
            brick.alive = False
        game.set_state(GameState.VICTORY)

        game.set_state(GameState.PLAYING)

        assert game.score == 0
        assert all(brick.alive for brick in game.bricks)


class TestGameImports:
    """Test that the game module imports correctly."""

    def test_imports(self):
        from main import Game, GameState, Paddle, Ball, Brick, Vector2
        assert Game is not None
        assert GameState is not None
        assert Paddle is not None
        assert Ball is not None
        assert Brick is not None
        assert Vector2 is not None

    def test_constants_exist(self):
        from main import (
            SCREEN_WIDTH, SCREEN_HEIGHT, FPS, PADDLE_WIDTH, PADDLE_HEIGHT,
            BALL_RADIUS, BALL_SPEED, BRICK_ROWS, BRICK_COLS, INITIAL_LIVES
        )
        assert all(isinstance(c, int) for c in [
            SCREEN_WIDTH, SCREEN_HEIGHT, FPS, PADDLE_WIDTH, PADDLE_HEIGHT,
            BALL_RADIUS, BALL_SPEED, BRICK_ROWS, BRICK_COLS, INITIAL_LIVES
        ])
