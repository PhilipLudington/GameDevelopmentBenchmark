#!/usr/bin/env python3
"""Test suite for space_invaders-feature-002: progressive difficulty waves."""

import os
import sys

os.environ["SDL_VIDEODRIVER"] = "dummy"
os.environ["SDL_AUDIODRIVER"] = "dummy"

task_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
if os.environ.get("TEST_SOLUTION"):
    sys.path.insert(0, os.path.join(task_dir, "solution"))
else:
    sys.path.insert(0, os.path.join(task_dir, "game"))

import pytest
from main import Game, GameState, ALIEN_ROWS, ALIEN_COLS


class TestWaveTracking:
    """Tests for wave number tracking."""

    @pytest.fixture
    def game(self):
        g = Game(headless=True)
        g.set_state(GameState.PLAYING)
        return g

    def test_wave_number_exists(self, game):
        """Game should track wave number."""
        assert hasattr(game, 'wave_number'), \
            "Game should have 'wave_number' attribute"

    def test_starts_at_wave_one(self, game):
        """Game should start at wave 1."""
        if not hasattr(game, 'wave_number'):
            pytest.skip("wave_number not implemented")

        assert game.wave_number == 1, "Game should start at wave 1"

    def test_wave_increments_on_clear(self, game):
        """Wave number should increment when all aliens cleared."""
        if not hasattr(game, 'wave_number'):
            pytest.skip("wave_number not implemented")

        initial_wave = game.wave_number

        # Kill all aliens
        for alien in game.fleet.aliens:
            alien.alive = False

        # Update should trigger next wave
        game.update()

        assert game.wave_number == initial_wave + 1, \
            "Wave should increment after clearing aliens"


class TestWaveDifficulty:
    """Tests for progressive difficulty."""

    @pytest.fixture
    def game(self):
        g = Game(headless=True)
        g.set_state(GameState.PLAYING)
        return g

    def test_wave_2_harder_than_wave_1(self, game):
        """Wave 2 should be more difficult than wave 1."""
        if not hasattr(game, 'wave_number'):
            pytest.skip("wave_number not implemented")

        wave1_delay = game.fleet.move_delay

        # Clear wave 1
        for alien in game.fleet.aliens:
            alien.alive = False
        game.update()

        # Check wave 2 difficulty
        wave2_delay = game.fleet.move_delay

        assert wave2_delay < wave1_delay, \
            "Wave 2 should have faster aliens (lower delay)"

    def test_difficulty_continues_increasing(self, game):
        """Each subsequent wave should be harder."""
        if not hasattr(game, 'wave_number'):
            pytest.skip("wave_number not implemented")

        delays = []

        for wave in range(3):
            delays.append(game.fleet.move_delay)

            # Clear current wave
            for alien in game.fleet.aliens:
                alien.alive = False
            game.update()

        # Each wave should be at least as hard as the previous
        for i in range(len(delays) - 1):
            assert delays[i + 1] <= delays[i], \
                f"Wave {i + 2} should be harder than wave {i + 1}"


class TestWaveTransition:
    """Tests for wave transition behavior."""

    @pytest.fixture
    def game(self):
        g = Game(headless=True)
        g.set_state(GameState.PLAYING)
        return g

    def test_aliens_respawn_on_new_wave(self, game):
        """Aliens should respawn when new wave starts."""
        if not hasattr(game, 'wave_number'):
            pytest.skip("wave_number not implemented")

        # Clear all aliens
        for alien in game.fleet.aliens:
            alien.alive = False

        assert game.fleet.alive_count == 0

        game.update()

        # After wave transition, aliens should be back
        assert game.fleet.alive_count == ALIEN_ROWS * ALIEN_COLS, \
            "Full alien fleet should spawn for new wave"

    def test_score_persists_across_waves(self, game):
        """Score should persist when moving to next wave."""
        if not hasattr(game, 'wave_number'):
            pytest.skip("wave_number not implemented")

        game.score = 1000

        # Clear wave
        for alien in game.fleet.aliens:
            alien.alive = False
        game.update()

        assert game.score == 1000, "Score should persist across waves"

    def test_lives_persist_across_waves(self, game):
        """Lives should persist when moving to next wave."""
        if not hasattr(game, 'wave_number'):
            pytest.skip("wave_number not implemented")

        game.lives = 2

        # Clear wave
        for alien in game.fleet.aliens:
            alien.alive = False
        game.update()

        assert game.lives == 2, "Lives should persist across waves"


class TestFinalVictory:
    """Tests for final victory condition."""

    @pytest.fixture
    def game(self):
        g = Game(headless=True)
        g.set_state(GameState.PLAYING)
        return g

    def test_victory_after_max_waves(self, game):
        """Game should end in victory after completing all waves."""
        if not hasattr(game, 'wave_number'):
            pytest.skip("wave_number not implemented")

        max_waves = getattr(game, 'max_waves', 5)

        # Clear multiple waves
        for _ in range(max_waves):
            for alien in game.fleet.aliens:
                alien.alive = False
            game.update()

        assert game.state == GameState.VICTORY, \
            f"Should reach victory after {max_waves} waves"
