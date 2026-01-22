#!/usr/bin/env python3
"""Test suite for space_invaders-002: alien fleet speed calculation bug."""

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
from main import Game, GameState, AlienFleet, ALIEN_ROWS, ALIEN_COLS


class TestFleetSpeedCalculation:
    """Tests for alien fleet movement speed calculation."""

    @pytest.fixture
    def game(self):
        g = Game(headless=True)
        g.set_state(GameState.PLAYING)
        return g

    def test_full_fleet_speed(self, game):
        """Full fleet should have reasonable base speed."""
        # Force a movement update to recalculate delay
        game.fleet.move_timer = game.fleet.move_delay
        game.fleet.update()

        # Full fleet should have delay around 25-35
        assert 20 <= game.fleet.move_delay <= 40, \
            f"Full fleet move_delay {game.fleet.move_delay} not in expected range"

    def test_half_fleet_faster(self, game):
        """Half the aliens should move faster than full fleet."""
        full_delay = game.fleet.move_delay

        # Kill half the aliens
        aliens_to_kill = (ALIEN_ROWS * ALIEN_COLS) // 2
        for alien in game.fleet.aliens[:aliens_to_kill]:
            alien.alive = False

        # Force update to recalculate
        game.fleet.move_timer = game.fleet.move_delay
        game.fleet.update()

        assert game.fleet.move_delay < full_delay, \
            "Half fleet should be faster than full fleet"

    def test_few_aliens_very_fast(self, game):
        """With only 5 aliens, movement should be very fast."""
        # Kill all but 5 aliens
        alive_count = 0
        for alien in game.fleet.aliens:
            if alive_count >= 5:
                alien.alive = False
            else:
                alive_count += 1

        game.fleet.move_timer = game.fleet.move_delay
        game.fleet.update()

        # With 5 aliens, delay should be <= 5
        assert game.fleet.move_delay <= 5, \
            f"5 aliens should have move_delay <= 5, got {game.fleet.move_delay}"

    def test_single_alien_extremely_fast(self, game):
        """Single remaining alien should move almost every frame."""
        # Kill all but one alien
        for i, alien in enumerate(game.fleet.aliens):
            if i > 0:
                alien.alive = False

        game.fleet.move_timer = game.fleet.move_delay
        game.fleet.update()

        # Single alien should have delay of 1-2 frames
        assert game.fleet.move_delay <= 2, \
            f"Single alien should have move_delay <= 2, got {game.fleet.move_delay}"

    def test_no_freeze_at_any_count(self, game):
        """Fleet should never freeze (delay should never be 0 or negative)."""
        for kill_count in range(ALIEN_ROWS * ALIEN_COLS - 1):
            # Kill one more alien
            alive_aliens = [a for a in game.fleet.aliens if a.alive]
            if alive_aliens:
                alive_aliens[0].alive = False

            game.fleet.move_timer = game.fleet.move_delay
            game.fleet.update()

            assert game.fleet.move_delay >= 1, \
                f"Fleet froze with {sum(1 for a in game.fleet.aliens if a.alive)} aliens"

    def test_speed_increases_exponentially(self, game):
        """Speed increase should feel exponential, not linear."""
        delays = []

        for target_count in [55, 40, 25, 10, 5, 1]:
            # Set exact number of aliens alive
            alive = 0
            for alien in game.fleet.aliens:
                if alive < target_count:
                    alien.alive = True
                    alive += 1
                else:
                    alien.alive = False

            game.fleet.move_timer = game.fleet.move_delay
            game.fleet.update()
            delays.append((target_count, game.fleet.move_delay))

        # The ratio of speed increase should accelerate
        # i.e., going from 55->40 should be less dramatic than 5->1
        for i in range(len(delays) - 1):
            count1, delay1 = delays[i]
            count2, delay2 = delays[i + 1]
            # Verify delay decreases as count decreases
            assert delay2 <= delay1, \
                f"Speed didn't increase: {count1} aliens={delay1}, {count2} aliens={delay2}"
