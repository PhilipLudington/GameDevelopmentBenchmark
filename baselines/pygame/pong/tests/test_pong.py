"""Tests for the Pong baseline game."""

import os
import sys
import pytest

# Set up headless mode
os.environ["SDL_VIDEODRIVER"] = "dummy"
os.environ["SDL_AUDIODRIVER"] = "dummy"

# Add parent directory to path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from main import (
    Game,
    GameState,
    Ball,
    Paddle,
    Vector2,
    SCREEN_WIDTH,
    SCREEN_HEIGHT,
    PADDLE_WIDTH,
    PADDLE_HEIGHT,
    BALL_SIZE,
)


class TestVector2:
    """Tests for Vector2 class."""

    def test_addition(self):
        """Test vector addition."""
        v1 = Vector2(1, 2)
        v2 = Vector2(3, 4)
        result = v1 + v2
        assert result.x == 4
        assert result.y == 6

    def test_scalar_multiplication(self):
        """Test scalar multiplication."""
        v = Vector2(2, 3)
        result = v * 2
        assert result.x == 4
        assert result.y == 6


class TestPaddle:
    """Tests for Paddle class."""

    def test_creation(self):
        """Test paddle creation."""
        paddle = Paddle(100, 200)
        assert paddle.rect.x == 100
        assert paddle.rect.y == 200
        assert paddle.rect.width == PADDLE_WIDTH
        assert paddle.rect.height == PADDLE_HEIGHT
        assert paddle.is_ai is False
        assert paddle.score == 0

    def test_move_up(self):
        """Test paddle moving up."""
        paddle = Paddle(100, 100)
        initial_y = paddle.rect.y
        paddle.move_up()
        assert paddle.rect.y < initial_y

    def test_move_up_boundary(self):
        """Test paddle doesn't move above screen."""
        paddle = Paddle(100, 0)
        paddle.move_up()
        assert paddle.rect.y >= 0

    def test_move_down(self):
        """Test paddle moving down."""
        paddle = Paddle(100, 100)
        initial_y = paddle.rect.y
        paddle.move_down()
        assert paddle.rect.y > initial_y

    def test_move_down_boundary(self):
        """Test paddle doesn't move below screen."""
        paddle = Paddle(100, SCREEN_HEIGHT - PADDLE_HEIGHT)
        paddle.move_down()
        assert paddle.rect.bottom <= SCREEN_HEIGHT

    def test_reset(self):
        """Test paddle reset."""
        paddle = Paddle(100, 0)
        paddle.move_down()
        paddle.move_down()
        paddle.reset()
        assert paddle.rect.centery == SCREEN_HEIGHT // 2

    def test_ai_paddle(self):
        """Test AI paddle creation."""
        paddle = Paddle(100, 200, is_ai=True)
        assert paddle.is_ai is True


class TestBall:
    """Tests for Ball class."""

    def test_creation(self):
        """Test ball creation at center."""
        ball = Ball()
        assert ball.rect.centerx == SCREEN_WIDTH // 2
        assert ball.rect.centery == SCREEN_HEIGHT // 2
        assert ball.rect.width == BALL_SIZE
        assert ball.rect.height == BALL_SIZE

    def test_reset(self):
        """Test ball reset."""
        ball = Ball()
        ball.rect.x = 0
        ball.rect.y = 0
        ball.reset()
        assert ball.rect.centerx == SCREEN_WIDTH // 2
        assert ball.rect.centery == SCREEN_HEIGHT // 2

    def test_reset_direction(self):
        """Test ball reset with direction."""
        ball = Ball()
        ball.reset(direction=-1)
        assert ball.velocity.x < 0

        ball.reset(direction=1)
        assert ball.velocity.x > 0

    def test_top_wall_collision(self):
        """Test ball bounces off top wall."""
        ball = Ball()
        ball.rect.top = 0
        ball.velocity.y = -5
        ball.update()
        assert ball.velocity.y > 0

    def test_bottom_wall_collision(self):
        """Test ball bounces off bottom wall."""
        ball = Ball()
        ball.rect.bottom = SCREEN_HEIGHT
        ball.velocity.y = 5
        ball.update()
        assert ball.velocity.y < 0

    def test_score_left(self):
        """Test scoring when ball goes off left side."""
        ball = Ball()
        ball.rect.right = -10
        result = ball.update()
        assert result == 1  # Player 2 scores

    def test_score_right(self):
        """Test scoring when ball goes off right side."""
        ball = Ball()
        ball.rect.left = SCREEN_WIDTH + 10
        result = ball.update()
        assert result == -1  # Player 1 scores

    def test_no_score(self):
        """Test no scoring during normal play."""
        ball = Ball()
        result = ball.update()
        assert result == 0

    def test_paddle_collision(self):
        """Test ball-paddle collision."""
        ball = Ball()
        paddle = Paddle(100, SCREEN_HEIGHT // 2 - PADDLE_HEIGHT // 2)

        # Position ball to collide with paddle
        ball.rect.right = paddle.rect.left + 5
        ball.rect.centery = paddle.rect.centery
        ball.velocity.x = 5  # Moving right towards paddle

        initial_vx = ball.velocity.x
        collision = ball.check_paddle_collision(paddle)

        assert collision is True
        assert ball.velocity.x != initial_vx  # Direction should change


class TestGame:
    """Tests for Game class."""

    @pytest.fixture
    def game(self):
        """Create a game instance for testing."""
        return Game(headless=True)

    def test_creation(self, game):
        """Test game creation."""
        assert game.state == GameState.MENU
        assert game.running is True
        assert game.player1 is not None
        assert game.player2 is not None
        assert game.ball is not None

    def test_initial_scores(self, game):
        """Test initial scores are zero."""
        assert game.player1.score == 0
        assert game.player2.score == 0

    def test_reset_game(self, game):
        """Test game reset."""
        game.player1.score = 5
        game.player2.score = 3
        game.reset_game()
        assert game.player1.score == 0
        assert game.player2.score == 0

    def test_state_transition(self, game):
        """Test state transitions."""
        game.set_state(GameState.PLAYING)
        assert game.state == GameState.PLAYING

        game.set_state(GameState.PAUSED)
        assert game.state == GameState.PAUSED

    def test_state_change_callback(self, game):
        """Test state change callback."""
        states_recorded = []
        game.on_state_change = lambda s: states_recorded.append(s)

        game.set_state(GameState.PLAYING)
        assert GameState.PLAYING in states_recorded

    def test_step(self, game):
        """Test step function."""
        game.set_state(GameState.PLAYING)
        state = game.step()

        assert "state" in state
        assert "player1_score" in state
        assert "player2_score" in state
        assert "ball_pos" in state
        assert "frame" in state
        assert state["frame"] == 1

    def test_player2_is_ai(self, game):
        """Test player 2 is AI controlled."""
        assert game.player2.is_ai is True
        assert game.player1.is_ai is False


class TestGameplay:
    """Integration tests for gameplay."""

    @pytest.fixture
    def game(self):
        """Create a game in playing state."""
        g = Game(headless=True)
        g.set_state(GameState.PLAYING)
        return g

    def test_ball_moves(self, game):
        """Test that ball moves during gameplay."""
        initial_pos = game.ball.rect.center
        for _ in range(10):
            game.step()
        new_pos = game.ball.rect.center
        assert new_pos != initial_pos

    def test_scoring_updates(self, game):
        """Test that scoring works correctly."""
        scores_recorded = []
        game.on_score = lambda p1, p2: scores_recorded.append((p1, p2))

        # Force a score
        game.ball.rect.left = SCREEN_WIDTH + 100
        game.step()

        assert len(scores_recorded) == 1
        assert scores_recorded[0][0] == 1  # Player 1 scored

    def test_game_over_at_winning_score(self, game):
        """Test game ends when winning score is reached."""
        game.player1.score = 10
        game.ball.rect.left = SCREEN_WIDTH + 100  # Force player 1 to score
        game.step()

        assert game.state == GameState.GAME_OVER
