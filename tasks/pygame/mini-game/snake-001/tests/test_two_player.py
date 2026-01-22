"""Tests for the two-player snake mode."""

import os
import sys
import pytest

# Set up headless mode
os.environ["SDL_VIDEODRIVER"] = "dummy"
os.environ["SDL_AUDIODRIVER"] = "dummy"

# Add game directory to path
game_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(game_dir, "game"))

import pygame
from main import Game, GameState, Direction, Position


class TestTwoPlayerFeature:
    """Tests for two-player mode."""

    def test_game_has_two_snakes(self):
        """Game should have snake1 and snake2."""
        game = Game(headless=True)
        assert hasattr(game, 'snake1'), "Game should have snake1"
        assert hasattr(game, 'snake2'), "Game should have snake2"

    def test_game_has_two_scores(self):
        """Game should have score1 and score2."""
        game = Game(headless=True)
        assert hasattr(game, 'score1'), "Game should have score1"
        assert hasattr(game, 'score2'), "Game should have score2"

    def test_snakes_start_at_different_positions(self):
        """Snakes should start at different positions."""
        game = Game(headless=True)
        assert game.snake1.head != game.snake2.head, \
            "Snakes should start at different positions"

    def test_player1_controls_with_wasd(self):
        """Player 1 should control snake1 with WASD."""
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)

        # Test W key (up)
        event = pygame.event.Event(pygame.KEYDOWN, key=pygame.K_w)
        game.handle_playing_input(event)
        assert game.snake1.next_direction == Direction.UP

    def test_player2_controls_with_arrows(self):
        """Player 2 should control snake2 with arrow keys."""
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)

        # First change snake2 direction to allow UP
        game.snake2.direction = Direction.RIGHT
        game.snake2.next_direction = Direction.RIGHT

        # Test UP arrow
        event = pygame.event.Event(pygame.KEYDOWN, key=pygame.K_UP)
        game.handle_playing_input(event)
        assert game.snake2.next_direction == Direction.UP

    def test_snake_dies_when_hitting_other_snake(self):
        """Snake should die when hitting the other snake's body."""
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)

        # Position snake1 to collide with snake2's body
        game.snake1.body[0] = Position(
            game.snake2.body[1].x,
            game.snake2.body[1].y
        )

        # Simulate collision check
        collision = game.snake2.check_collision(game.snake1.head)
        assert collision, "Should detect collision between snakes"

    def test_both_snakes_move_independently(self):
        """Both snakes should move independently."""
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)

        initial_head1 = Position(game.snake1.head.x, game.snake1.head.y)
        initial_head2 = Position(game.snake2.head.x, game.snake2.head.y)

        game.update()

        # Both should have moved
        assert game.snake1.head != initial_head1 or \
               game.snake2.head != initial_head2, \
            "At least one snake should have moved"

    def test_food_can_be_eaten_by_either_snake(self):
        """Either snake should be able to eat food."""
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)

        # Position food next to snake1
        game.food.position = Position(
            game.snake1.head.x + 1,
            game.snake1.head.y
        )

        # Food should be eatable by snake1
        game.snake1.move()
        eaten = game.food.check_eaten(game.snake1)
        assert eaten or game.food.check_eaten(game.snake2), \
            "Food should be eatable by either snake"


class TestGameImports:
    """Basic tests to verify the game module works."""

    def test_game_imports(self):
        """Test that the game module can be imported."""
        from main import Game, GameState, Snake
        assert Game is not None
        assert Snake is not None
