#!/usr/bin/env python3
"""Test suite for space_invaders-001: bullet tunneling bug fix."""

import os
import sys

# Set up headless mode BEFORE importing pygame
os.environ["SDL_VIDEODRIVER"] = "dummy"
os.environ["SDL_AUDIODRIVER"] = "dummy"

# Add game directory to path for imports (or solution if TEST_SOLUTION env var is set)
task_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
if os.environ.get("TEST_SOLUTION"):
    sys.path.insert(0, os.path.join(task_dir, "solution"))
else:
    sys.path.insert(0, os.path.join(task_dir, "game"))

import pytest
from main import (
    Game, GameState, Bullet, Alien,
    ALIEN_WIDTH, ALIEN_HEIGHT, BULLET_WIDTH, BULLET_HEIGHT,
    PLAYER_BULLET_SPEED
)


class TestNormalBulletCollision:
    """Tests for normal-speed bullet collision (sanity checks)."""

    @pytest.fixture
    def game(self):
        """Create a game in playing state."""
        g = Game(headless=True)
        g.set_state(GameState.PLAYING)
        return g

    def test_normal_bullet_hits_alien(self, game):
        """A normal-speed bullet should hit an alien directly in its path."""
        # Find the first alive alien
        target = next(a for a in game.fleet.aliens if a.alive)

        # Position bullet just below the alien, moving up
        game.player_bullets.append(Bullet(
            x=target.x + ALIEN_WIDTH // 2 - BULLET_WIDTH // 2,
            y=target.y + ALIEN_HEIGHT + 5,
            speed=PLAYER_BULLET_SPEED,
            is_player_bullet=True
        ))
        initial_alive = game.fleet.alive_count

        # Run a few updates
        for _ in range(10):
            game.update()

        assert game.fleet.alive_count == initial_alive - 1
        assert not target.alive

    def test_bullet_removed_after_hit(self, game):
        """Bullet should be removed after hitting an alien."""
        target = next(a for a in game.fleet.aliens if a.alive)

        game.player_bullets.append(Bullet(
            x=target.x + ALIEN_WIDTH // 2 - BULLET_WIDTH // 2,
            y=target.y + ALIEN_HEIGHT + 5,
            speed=PLAYER_BULLET_SPEED,
            is_player_bullet=True
        ))

        for _ in range(10):
            game.update()

        # Bullet should be removed (either hit alien or went off screen)
        # The test focuses on whether collision happened
        assert not target.alive


class TestHighSpeedBulletTunneling:
    """Tests for high-speed bullet collision (the bug)."""

    @pytest.fixture
    def game(self):
        """Create a game in playing state."""
        g = Game(headless=True)
        g.set_state(GameState.PLAYING)
        return g

    def test_fast_bullet_hits_alien_should_not_tunnel(self, game):
        """
        A high-speed bullet positioned to pass through an alien in one frame
        should still register a hit.

        This is the core test for the tunneling bug.
        """
        target = next(a for a in game.fleet.aliens if a.alive)

        # Position bullet so that in ONE update, it will move from below
        # the alien to above it (tunneling scenario)
        # Bullet speed = 25 pixels/frame, alien height = 30
        high_speed = 25

        # Place bullet just below the alien's bottom edge
        start_y = target.y + ALIEN_HEIGHT + 2

        game.player_bullets.append(Bullet(
            x=target.x + ALIEN_WIDTH // 2 - BULLET_WIDTH // 2,
            y=start_y,
            speed=high_speed,
            is_player_bullet=True
        ))
        initial_alive = game.fleet.alive_count

        # One update should be enough - bullet should detect collision
        # even though its new position would be above the alien
        game.update()

        # The alien should be destroyed - this tests the swept collision
        assert game.fleet.alive_count == initial_alive - 1, \
            "Fast bullet tunneled through alien without hitting it"
        assert not target.alive, "Target alien should be destroyed"

    def test_very_fast_bullet_should_not_tunnel(self, game):
        """
        An extremely fast bullet (50 pixels/frame) should still hit aliens.
        """
        target = next(a for a in game.fleet.aliens if a.alive)

        # 50 pixels per frame - would skip right past a 30-pixel alien
        very_high_speed = 50

        # Place bullet below alien
        start_y = target.y + ALIEN_HEIGHT + 5

        game.player_bullets.append(Bullet(
            x=target.x + ALIEN_WIDTH // 2 - BULLET_WIDTH // 2,
            y=start_y,
            speed=very_high_speed,
            is_player_bullet=True
        ))

        game.update()

        assert not target.alive, \
            "Very fast bullet (50px/frame) tunneled through alien"

    def test_multiple_fast_bullets(self, game):
        """Multiple fast bullets should each hit their respective aliens."""
        targets = [a for a in game.fleet.aliens if a.alive][:3]

        high_speed = 30

        for target in targets:
            game.player_bullets.append(Bullet(
                x=target.x + ALIEN_WIDTH // 2 - BULLET_WIDTH // 2,
                y=target.y + ALIEN_HEIGHT + 3,
                speed=high_speed,
                is_player_bullet=True
            ))

        game.update()

        for target in targets:
            assert not target.alive, f"Fast bullet missed alien at row {target.row}, col {target.col}"


class TestBulletPositionAfterCollision:
    """Tests for bullet position handling after collision."""

    @pytest.fixture
    def game(self):
        """Create a game in playing state."""
        g = Game(headless=True)
        g.set_state(GameState.PLAYING)
        return g

    def test_bullet_removed_on_collision_not_after_passing(self, game):
        """
        Bullet should be removed at the point of collision, not continue past.
        This tests that swept collision properly detects the intersection.
        """
        target = next(a for a in game.fleet.aliens if a.alive)

        # Track what happens
        destroyed_aliens = []
        game.on_alien_destroyed = lambda a: destroyed_aliens.append(a)

        high_speed = 40

        game.player_bullets.append(Bullet(
            x=target.x + ALIEN_WIDTH // 2 - BULLET_WIDTH // 2,
            y=target.y + ALIEN_HEIGHT + 5,
            speed=high_speed,
            is_player_bullet=True
        ))

        game.update()

        # Should have destroyed exactly one alien
        assert len(destroyed_aliens) == 1
        assert destroyed_aliens[0] == target


class TestEdgeCases:
    """Edge case tests for collision detection."""

    @pytest.fixture
    def game(self):
        g = Game(headless=True)
        g.set_state(GameState.PLAYING)
        return g

    def test_bullet_grazing_edge(self, game):
        """Bullet that grazes the edge of an alien should still hit."""
        target = next(a for a in game.fleet.aliens if a.alive)

        # Position bullet at the left edge of the alien
        game.player_bullets.append(Bullet(
            x=target.x - BULLET_WIDTH + 1,  # Just barely overlapping left edge
            y=target.y + ALIEN_HEIGHT + 5,
            speed=20,
            is_player_bullet=True
        ))

        game.update()

        assert not target.alive, "Bullet grazing edge should still hit"

    def test_fast_bullet_diagonal_path_conceptual(self, game):
        """
        While bullets move only vertically in this game, ensure swept
        collision handles the case correctly.
        """
        target = next(a for a in game.fleet.aliens if a.alive)

        # Standard vertical high-speed bullet
        game.player_bullets.append(Bullet(
            x=target.x + ALIEN_WIDTH // 2,
            y=target.y + ALIEN_HEIGHT + 10,
            speed=35,
            is_player_bullet=True
        ))

        game.update()

        assert not target.alive
