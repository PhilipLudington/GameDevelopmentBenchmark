"""Tests for the pause functionality feature."""

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


class TestPauseFeature:
    """Tests for pause functionality."""

    def test_p_key_pauses_game(self):
        """Pressing P should pause the game."""
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)

        event = pygame.event.Event(pygame.KEYDOWN, key=pygame.K_p)
        game.handle_playing_input(event)

        assert game.state == GameState.PAUSED, \
            f"Game should be PAUSED after pressing P, got {game.state}"

    def test_escape_pauses_game(self):
        """Pressing ESC should pause the game."""
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)

        event = pygame.event.Event(pygame.KEYDOWN, key=pygame.K_ESCAPE)
        game.handle_playing_input(event)

        assert game.state == GameState.PAUSED, \
            f"Game should be PAUSED after pressing ESC, got {game.state}"

    def test_p_key_resumes_game(self):
        """Pressing P should resume a paused game."""
        game = Game(headless=True)
        game.set_state(GameState.PAUSED)

        event = pygame.event.Event(pygame.KEYDOWN, key=pygame.K_p)
        game.handle_paused_input(event)

        assert game.state == GameState.PLAYING, \
            f"Game should be PLAYING after pressing P in pause, got {game.state}"

    def test_space_resumes_game(self):
        """Pressing SPACE should resume a paused game."""
        game = Game(headless=True)
        game.set_state(GameState.PAUSED)

        event = pygame.event.Event(pygame.KEYDOWN, key=pygame.K_SPACE)
        game.handle_paused_input(event)

        assert game.state == GameState.PLAYING, \
            f"Game should be PLAYING after pressing SPACE in pause, got {game.state}"

    def test_pause_preserves_game_state(self):
        """Pausing should preserve the game state."""
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game.score = 100
        original_snake_length = len(game.snake.body)

        # Pause
        game.set_state(GameState.PAUSED)

        assert game.score == 100, "Score should be preserved during pause"
        assert len(game.snake.body) == original_snake_length, \
            "Snake length should be preserved during pause"


class TestGameImports:
    """Basic tests to verify the game module works."""

    def test_game_imports(self):
        """Test that the game module can be imported."""
        from main import Game, GameState
        assert Game is not None
        assert GameState.PAUSED is not None
