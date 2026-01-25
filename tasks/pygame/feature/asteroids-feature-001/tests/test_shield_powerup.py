"""
Tests for shield power-up system in Asteroids.
"""

import os
import sys
import pytest

# Setup headless mode
os.environ["SDL_VIDEODRIVER"] = "dummy"
os.environ["SDL_AUDIODRIVER"] = "dummy"

# Add game directory to path
game_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(game_dir, "game"))

from main import Game, GameState, Ship, Vector2


class TestPowerUpClass:
    """Test PowerUp class exists and has correct attributes."""

    def test_powerup_class_exists(self):
        from main import PowerUp
        assert PowerUp is not None

    def test_powerup_creation(self):
        from main import PowerUp
        powerup = PowerUp(100, 200)
        assert powerup.position.x == 100
        assert powerup.position.y == 200
        assert powerup.power_type == "shield"
        assert powerup.active == True
        assert powerup.radius == 15

    def test_powerup_has_spawn_timer(self):
        from main import PowerUp
        powerup = PowerUp(100, 200)
        assert hasattr(powerup, 'spawn_timer')
        assert powerup.spawn_timer > 0


class TestShipShield:
    """Test ship shield functionality."""

    def test_ship_has_shield_active(self):
        ship = Ship(100, 100)
        assert hasattr(ship, 'shield_active')

    def test_ship_has_shield_timer(self):
        ship = Ship(100, 100)
        assert hasattr(ship, 'shield_timer')

    def test_ship_shield_initially_inactive(self):
        ship = Ship(100, 100)
        assert ship.shield_active == False
        assert ship.shield_timer == 0

    def test_ship_activate_shield(self):
        ship = Ship(100, 100)
        ship.shield_active = True
        ship.shield_timer = 300
        assert ship.shield_active
        assert ship.shield_timer == 300


class TestGamePowerUpSpawning:
    """Test power-up spawning in game."""

    def test_game_has_power_up_attribute(self):
        game = Game(headless=True)
        assert hasattr(game, 'power_up')

    def test_game_has_power_up_spawn_timer(self):
        game = Game(headless=True)
        assert hasattr(game, 'power_up_spawn_timer')

    def test_spawn_power_up_method_exists(self):
        game = Game(headless=True)
        assert hasattr(game, 'spawn_power_up')
        assert callable(game.spawn_power_up)

    def test_spawn_power_up_creates_powerup(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game.power_up = None
        game.spawn_power_up()
        assert game.power_up is not None

    def test_powerup_spawns_at_safe_location(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game.spawn_power_up()
        # Power-up should not spawn on ship
        ship_pos = game.ship.position
        powerup_pos = game.power_up.position
        distance = ship_pos.distance_to(powerup_pos)
        assert distance > 50  # Safe distance from ship


class TestPowerUpCollection:
    """Test power-up collection mechanics."""

    def test_collecting_powerup_activates_shield(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)

        # Spawn power-up at ship location
        from main import PowerUp
        game.power_up = PowerUp(game.ship.position.x, game.ship.position.y)

        # Run update to check collision
        game._update_playing()

        # Shield should be activated
        assert game.ship.shield_active == True
        assert game.ship.shield_timer > 0

    def test_powerup_removed_after_collection(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)

        # Spawn power-up at ship location
        from main import PowerUp
        game.power_up = PowerUp(game.ship.position.x, game.ship.position.y)

        game._update_playing()

        # Power-up should be removed
        assert game.power_up is None


class TestShieldProtection:
    """Test shield provides protection."""

    def test_shield_prevents_asteroid_damage(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)

        # Activate shield
        game.ship.shield_active = True
        game.ship.shield_timer = 300

        # Place asteroid on ship
        from main import Asteroid
        game.asteroids = [Asteroid(game.ship.position.x, game.ship.position.y, 3)]

        initial_lives = game.lives
        game._check_collisions()

        # Should not lose life
        assert game.lives == initial_lives

    def test_shield_timer_decrements(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)

        game.ship.shield_active = True
        game.ship.shield_timer = 100

        game._update_playing()

        assert game.ship.shield_timer < 100

    def test_shield_deactivates_when_timer_zero(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)

        game.ship.shield_active = True
        game.ship.shield_timer = 1

        game._update_playing()

        assert game.ship.shield_active == False
        assert game.ship.shield_timer == 0


class TestPowerUpDespawn:
    """Test power-up despawn behavior."""

    def test_powerup_despawns_after_timeout(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)

        from main import PowerUp
        game.power_up = PowerUp(100, 100)
        game.power_up.spawn_timer = 1  # About to despawn

        # Run update
        game._update_playing()

        # Power-up should be despawned
        assert game.power_up is None or game.power_up.active == False


class TestPowerUpSpawnTimer:
    """Test automatic power-up spawning."""

    def test_powerup_spawns_after_timer(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game.power_up = None
        game.power_up_spawn_timer = 1  # About to spawn

        game._update_playing()

        # Power-up should spawn
        assert game.power_up is not None


class TestOnlyOnePowerUp:
    """Test only one power-up at a time."""

    def test_no_spawn_if_powerup_exists(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)

        from main import PowerUp
        existing = PowerUp(100, 100)
        game.power_up = existing

        game.spawn_power_up()

        # Should still be the same power-up
        assert game.power_up is existing


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
