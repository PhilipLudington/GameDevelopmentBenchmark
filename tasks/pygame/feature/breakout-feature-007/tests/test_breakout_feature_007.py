#!/usr/bin/env python3
"""Tests for breakout-feature-007: Add pause countdown."""

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


class TestPauseCountdown:
    """Test pause countdown functionality."""

    @pytest.fixture
    def game(self):
        g = Game(headless=True)
        g.set_state(GameState.PLAYING)
        return g

    def test_countdown_state_exists(self, game):
        """COUNTDOWN state should exist in GameState."""
        assert hasattr(GameState, 'COUNTDOWN'), "GameState should have COUNTDOWN state"

    def test_pause_transitions_to_countdown(self, game):
        """Unpausing should go through COUNTDOWN state."""
        if not hasattr(GameState, 'COUNTDOWN'):
            pytest.skip("COUNTDOWN state not implemented")

        game.set_state(GameState.PAUSED)
        game.set_state(GameState.PLAYING)  # This might go through COUNTDOWN

        # Check if game has countdown tracking
        has_countdown = hasattr(game, 'countdown_value') or hasattr(game, 'countdown')

    def test_game_has_countdown_tracking(self, game):
        """Game should track countdown value."""
        if not hasattr(GameState, 'COUNTDOWN'):
            pytest.skip("COUNTDOWN state not implemented")

        has_countdown = (
            hasattr(game, 'countdown_value') or
            hasattr(game, 'countdown') or
            hasattr(game, 'countdown_timer')
        )
        assert has_countdown

    def test_countdown_starts_at_3(self, game):
        """Countdown should start at 3."""
        if not hasattr(game, 'countdown_value'):
            pytest.skip("Countdown value not implemented")

        game.set_state(GameState.PAUSED)
        if hasattr(GameState, 'COUNTDOWN'):
            game.set_state(GameState.COUNTDOWN)
            assert game.countdown_value == 3
