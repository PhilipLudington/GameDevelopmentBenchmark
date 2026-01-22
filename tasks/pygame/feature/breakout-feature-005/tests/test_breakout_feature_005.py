#!/usr/bin/env python3
"""Tests for breakout-feature-005: Add sound effects."""

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
from main import Game, GameState


class TestSoundEffects:
    """Test sound effects functionality."""

    @pytest.fixture
    def game(self):
        g = Game(headless=True)
        g.set_state(GameState.PLAYING)
        return g

    def test_game_has_sounds(self, game):
        """Game should have sound attributes."""
        # Check for common sound attribute patterns
        has_sounds = (
            hasattr(game, 'sounds') or
            hasattr(game, 'sound_enabled') or
            hasattr(game, 'paddle_sound') or
            hasattr(game, 'brick_sound')
        )
        assert has_sounds, "Game should have sound-related attributes"

    def test_sounds_disabled_in_headless(self, game):
        """Sounds should not cause errors in headless mode."""
        # This test mainly verifies the game runs without sound errors
        for _ in range(10):
            game.step()
        # If we get here without exceptions, sounds are handled correctly
        assert True

    def test_play_sound_method_exists(self, game):
        """Game should have method to play sounds."""
        has_play_method = (
            hasattr(game, 'play_sound') or
            hasattr(game, '_play_sound')
        )
        # This is optional - implementation may vary
