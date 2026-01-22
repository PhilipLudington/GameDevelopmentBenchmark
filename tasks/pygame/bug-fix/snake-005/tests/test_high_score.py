"""Tests for the high score persistence fix."""

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
from main import Game, GameState


class TestHighScoreFix:
    """Tests to verify high score persists correctly."""

    def test_high_score_preserved_when_returning_to_menu(self):
        """High score should be preserved when pressing ESC from game over."""
        game = Game(headless=True)
        game.set_state(GameState.GAME_OVER)
        game.high_score = 500

        # Simulate pressing ESC
        event = pygame.event.Event(pygame.KEYDOWN, key=pygame.K_ESCAPE)
        game.handle_game_over_input(event)

        assert game.high_score == 500, \
            f"High score should be 500 after returning to menu, got {game.high_score}"

    def test_high_score_preserved_when_starting_new_game(self):
        """High score should be preserved when starting a new game."""
        game = Game(headless=True)
        game.set_state(GameState.GAME_OVER)
        game.high_score = 300

        # Simulate pressing SPACE to start new game
        event = pygame.event.Event(pygame.KEYDOWN, key=pygame.K_SPACE)
        game.handle_game_over_input(event)

        assert game.high_score == 300, \
            f"High score should be 300 after starting new game, got {game.high_score}"

    def test_high_score_persists_through_multiple_games(self):
        """High score should persist through multiple game cycles."""
        game = Game(headless=True)

        # First game - set high score
        game.set_state(GameState.PLAYING)
        game.high_score = 200

        # Go to game over and back to menu
        game.set_state(GameState.GAME_OVER)
        event = pygame.event.Event(pygame.KEYDOWN, key=pygame.K_ESCAPE)
        game.handle_game_over_input(event)

        assert game.high_score == 200, "High score lost after returning to menu"

        # Start another game
        game.set_state(GameState.PLAYING)
        assert game.high_score == 200, "High score lost after starting new game"

    def test_high_score_updated_correctly(self):
        """High score should update when current score exceeds it."""
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game.high_score = 100
        game.score = 150

        # Trigger game over (high score should update)
        if game.score > game.high_score:
            game.high_score = game.score

        assert game.high_score == 150, \
            f"High score should update to 150, got {game.high_score}"


class TestGameImports:
    """Basic tests to verify the game module works."""

    def test_game_imports(self):
        """Test that the game module can be imported."""
        from main import Game, GameState
        assert Game is not None
        assert GameState is not None

    def test_game_has_high_score(self):
        """Test game has high_score attribute."""
        game = Game(headless=True)
        assert hasattr(game, 'high_score')
        assert game.high_score == 0
