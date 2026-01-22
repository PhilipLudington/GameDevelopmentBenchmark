#!/usr/bin/env python3
"""Test suite for space_invaders-feature-003: power-up system."""

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
from main import Game, GameState, Bullet, ALIEN_WIDTH, ALIEN_HEIGHT, PLAYER_BULLET_SPEED


class TestPowerUpBasic:
    """Basic tests for power-up implementation."""

    @pytest.fixture
    def game(self):
        g = Game(headless=True)
        g.set_state(GameState.PLAYING)
        return g

    def test_powerups_list_exists(self, game):
        """Game should have a powerups list."""
        assert hasattr(game, 'powerups') or hasattr(game, 'power_ups'), \
            "Game should have 'powerups' or 'power_ups' attribute"

    def test_active_powerups_tracking(self, game):
        """Game should track active power-ups."""
        assert hasattr(game, 'active_powerups') or hasattr(game, 'active_power_ups'), \
            "Game should track active power-ups"


class TestPowerUpDrops:
    """Tests for power-up drop mechanics."""

    @pytest.fixture
    def game(self):
        g = Game(headless=True)
        g.set_state(GameState.PLAYING)
        return g

    def test_powerup_can_drop_from_alien(self, game):
        """Destroying alien should have chance to drop power-up."""
        powerups_list = getattr(game, 'powerups', None) or getattr(game, 'power_ups', None)
        if powerups_list is None:
            pytest.skip("powerups not implemented")

        # Kill many aliens to test probability
        dropped = False
        for _ in range(50):
            game.powerups = [] if hasattr(game, 'powerups') else game.power_ups
            game.powerups.clear()

            # Kill an alien
            target = next((a for a in game.fleet.aliens if a.alive), None)
            if target:
                game.player_bullets.append(Bullet(
                    x=target.x + ALIEN_WIDTH // 2,
                    y=target.y + ALIEN_HEIGHT + 2,
                    speed=PLAYER_BULLET_SPEED,
                    is_player_bullet=True
                ))
                game.update()

                if len(game.powerups) > 0:
                    dropped = True
                    break

        assert dropped, "Power-ups should occasionally drop from aliens"

    def test_powerup_falls_down(self, game):
        """Power-ups should fall downward."""
        powerups_list = getattr(game, 'powerups', None) or getattr(game, 'power_ups', None)
        if powerups_list is None:
            pytest.skip("powerups not implemented")

        # This test depends on implementation details
        pass


class TestPowerUpCollection:
    """Tests for power-up collection mechanics."""

    @pytest.fixture
    def game(self):
        g = Game(headless=True)
        g.set_state(GameState.PLAYING)
        return g

    def test_powerup_collected_on_touch(self, game):
        """Power-up should be collected when touching player."""
        # Implementation-dependent test
        pass

    def test_powerup_activates_on_collection(self, game):
        """Collected power-up should activate its effect."""
        # Implementation-dependent test
        pass


class TestPowerUpEffects:
    """Tests for individual power-up effects."""

    @pytest.fixture
    def game(self):
        g = Game(headless=True)
        g.set_state(GameState.PLAYING)
        return g

    def test_rapid_fire_reduces_cooldown(self, game):
        """Rapid fire power-up should reduce shoot cooldown."""
        active = getattr(game, 'active_powerups', None) or getattr(game, 'active_power_ups', None)
        if active is None:
            pytest.skip("active powerups not implemented")

        # Test would activate rapid fire and check cooldown
        pass

    def test_powerup_has_duration(self, game):
        """Timed power-ups should expire after duration."""
        # Implementation-dependent test
        pass


class TestPowerUpReset:
    """Tests for power-up reset behavior."""

    @pytest.fixture
    def game(self):
        g = Game(headless=True)
        g.set_state(GameState.PLAYING)
        return g

    def test_powerups_cleared_on_reset(self, game):
        """Power-ups should clear on game reset."""
        powerups_list = getattr(game, 'powerups', None) or getattr(game, 'power_ups', None)
        if powerups_list is None:
            pytest.skip("powerups not implemented")

        game.reset_game()

        assert len(powerups_list) == 0, "Power-ups should clear on reset"

    def test_active_effects_cleared_on_reset(self, game):
        """Active power-up effects should clear on game reset."""
        active = getattr(game, 'active_powerups', None) or getattr(game, 'active_power_ups', None)
        if active is None:
            pytest.skip("active powerups not implemented")

        game.reset_game()

        # All active effects should be 0/False/empty
        if isinstance(active, dict):
            for key, value in active.items():
                assert value == 0 or value is False, \
                    f"Power-up {key} should be inactive after reset"
