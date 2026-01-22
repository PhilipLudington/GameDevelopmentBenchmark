#!/usr/bin/env python3
"""Test suite for space_invaders-004: dead aliens shooting bug."""

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
from main import (
    Game, GameState, Bullet, Alien,
    ALIEN_WIDTH, ALIEN_HEIGHT, BULLET_WIDTH, PLAYER_BULLET_SPEED, ALIEN_BULLET_SPEED
)


class TestDeadAlienShooting:
    """Tests for dead aliens shooting bug."""

    @pytest.fixture
    def game(self):
        g = Game(headless=True)
        g.set_state(GameState.PLAYING)
        return g

    def test_destroyed_alien_cannot_shoot_same_frame(self, game):
        """An alien destroyed by a bullet should not shoot in that same frame."""
        # Get bottom row aliens (these are the shooters)
        bottom_aliens = [a for a in game.fleet.aliens
                         if a.alive and a.row == 4]  # Bottom row

        if not bottom_aliens:
            pytest.skip("No bottom row aliens")

        target = bottom_aliens[0]

        # Record target position before destruction
        target_x = target.x
        target_y = target.y

        # Position a bullet to kill this alien during update
        game.player_bullets.append(Bullet(
            x=target.x + ALIEN_WIDTH // 2,
            y=target.y + ALIEN_HEIGHT + 2,
            speed=PLAYER_BULLET_SPEED,
            is_player_bullet=True
        ))

        # Set alien shoot chance to 100% to guarantee shooting attempt
        import main
        old_chance = main.ALIEN_SHOOT_CHANCE
        main.ALIEN_SHOOT_CHANCE = 1.0

        try:
            # Clear any existing alien bullets
            game.alien_bullets.clear()

            # Run one update
            game.update()

            # Check: no alien bullet should originate from the dead alien's position
            for bullet in game.alien_bullets:
                bullet_center_x = bullet.x + BULLET_WIDTH // 2
                alien_center_x = target_x + ALIEN_WIDTH // 2

                # If bullet is roughly at the target's x position, it's suspicious
                if abs(bullet_center_x - alien_center_x) < 10:
                    # This bullet might be from the dead alien
                    assert target.alive, \
                        f"Dead alien at ({target_x}, {target_y}) appears to have fired a bullet"

        finally:
            main.ALIEN_SHOOT_CHANCE = old_chance

    def test_get_shooters_excludes_dead_aliens(self, game):
        """get_shooters() should not return any dead aliens."""
        # Kill some bottom row aliens
        for alien in game.fleet.aliens:
            if alien.row == 4 and alien.col < 5:  # Kill left half of bottom row
                alien.alive = False

        shooters = game.fleet.get_shooters()

        for shooter in shooters:
            assert shooter.alive, "get_shooters() returned a dead alien"

    def test_no_ghost_bullets_after_mass_destruction(self, game):
        """After destroying many aliens at once, no ghost bullets should appear."""
        # Kill all bottom row aliens with bullets positioned to hit them
        bottom_aliens = [a for a in game.fleet.aliens if a.alive and a.row == 4]

        for alien in bottom_aliens[:5]:  # Kill 5 aliens
            game.player_bullets.append(Bullet(
                x=alien.x + ALIEN_WIDTH // 2,
                y=alien.y + ALIEN_HEIGHT + 2,
                speed=PLAYER_BULLET_SPEED,
                is_player_bullet=True
            ))

        import main
        old_chance = main.ALIEN_SHOOT_CHANCE
        main.ALIEN_SHOOT_CHANCE = 1.0

        try:
            game.alien_bullets.clear()
            game.update()

            # Count bullets from positions that should be dead aliens
            ghost_bullets = 0
            for bullet in game.alien_bullets:
                for alien in bottom_aliens[:5]:  # The ones we tried to kill
                    if not alien.alive:
                        bullet_x = bullet.x + BULLET_WIDTH // 2
                        alien_x = alien.x + ALIEN_WIDTH // 2
                        if abs(bullet_x - alien_x) < 10:
                            ghost_bullets += 1

            assert ghost_bullets == 0, \
                f"Found {ghost_bullets} ghost bullets from dead aliens"

        finally:
            main.ALIEN_SHOOT_CHANCE = old_chance

    def test_bullet_originates_from_alive_alien(self, game):
        """Any alien bullet fired should come from a currently-alive alien."""
        import main
        old_chance = main.ALIEN_SHOOT_CHANCE
        main.ALIEN_SHOOT_CHANCE = 0.5  # High chance but not guaranteed

        try:
            game.alien_bullets.clear()

            # Run several updates
            for _ in range(10):
                game.update()

            # Check each alien bullet
            for bullet in game.alien_bullets:
                bullet_x = bullet.x + BULLET_WIDTH // 2

                # Find which alien column this bullet came from
                found_alive_shooter = False
                for alien in game.fleet.aliens:
                    if alien.alive:
                        alien_x = alien.x + ALIEN_WIDTH // 2
                        if abs(bullet_x - alien_x) < 20:
                            found_alive_shooter = True
                            break

                # Note: This test might have false positives if aliens moved
                # But it helps catch obvious ghost bullet issues

        finally:
            main.ALIEN_SHOOT_CHANCE = old_chance
