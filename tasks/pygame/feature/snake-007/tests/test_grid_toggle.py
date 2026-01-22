"""Tests for the grid toggle feature."""

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


class TestGridToggleFeature:
    """Tests for grid toggle functionality."""

    def test_game_has_show_grid_attribute(self):
        """Game should have show_grid attribute."""
        game = Game(headless=True)
        assert hasattr(game, 'show_grid'), \
            "Game should have show_grid attribute"

    def test_show_grid_default_true(self):
        """show_grid should default to True."""
        game = Game(headless=True)
        assert game.show_grid is True, \
            "show_grid should default to True"

    def test_g_key_toggles_grid(self):
        """Pressing G should toggle grid visibility."""
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)

        assert game.show_grid is True

        # Press G
        event = pygame.event.Event(pygame.KEYDOWN, key=pygame.K_g)
        game.handle_playing_input(event)

        assert game.show_grid is False, \
            "G key should toggle show_grid to False"

        # Press G again
        game.handle_playing_input(event)

        assert game.show_grid is True, \
            "G key should toggle show_grid back to True"

    def test_grid_toggle_persists_during_game(self):
        """Grid toggle state should persist during gameplay."""
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)

        # Toggle off
        game.show_grid = False

        # Simulate some game updates
        for _ in range(5):
            game.update()

        assert game.show_grid is False, \
            "Grid toggle should persist during gameplay"


class TestGameImports:
    """Basic tests to verify the game module works."""

    def test_game_imports(self):
        """Test that the game module can be imported."""
        from main import Game, GameState
        assert Game is not None
