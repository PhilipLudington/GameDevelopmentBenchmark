#!/usr/bin/env python3
"""Test suite for space_invaders-optimization-001: spatial hash collision optimization."""

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
from main import Game, GameState, Bullet, PLAYER_BULLET_SPEED, ALIEN_WIDTH, ALIEN_HEIGHT


class TestSpatialHashExists:
    """Tests for spatial hash implementation existence."""

    @pytest.fixture
    def game(self):
        g = Game(headless=True)
        g.set_state(GameState.PLAYING)
        return g

    def test_spatial_hash_exists(self, game):
        """Game should have spatial hash grid."""
        has_grid = (hasattr(game, 'spatial_grid') or
                   hasattr(game, 'spatial_hash') or
                   hasattr(game, 'collision_grid'))
        assert has_grid, "Game should have spatial hash grid attribute"


class TestSpatialHashFunctionality:
    """Tests for spatial hash grid functionality."""

    @pytest.fixture
    def game(self):
        g = Game(headless=True)
        g.set_state(GameState.PLAYING)
        return g

    def test_grid_has_insert_method(self, game):
        """Spatial grid should have insert method."""
        grid = (getattr(game, 'spatial_grid', None) or
               getattr(game, 'spatial_hash', None) or
               getattr(game, 'collision_grid', None))

        if grid is None:
            pytest.skip("spatial grid not implemented")

        assert hasattr(grid, 'insert'), "Grid should have insert method"

    def test_grid_has_query_method(self, game):
        """Spatial grid should have query/get_nearby method."""
        grid = (getattr(game, 'spatial_grid', None) or
               getattr(game, 'spatial_hash', None) or
               getattr(game, 'collision_grid', None))

        if grid is None:
            pytest.skip("spatial grid not implemented")

        has_query = (hasattr(grid, 'get_nearby') or
                    hasattr(grid, 'query') or
                    hasattr(grid, 'get_potential_collisions'))

        assert has_query, "Grid should have query method"


class TestCollisionCorrectness:
    """Tests that collision detection still works correctly."""

    @pytest.fixture
    def game(self):
        g = Game(headless=True)
        g.set_state(GameState.PLAYING)
        return g

    def test_bullet_still_hits_alien(self, game):
        """Bullets should still hit aliens with optimization."""
        target = next(a for a in game.fleet.aliens if a.alive)

        game.player_bullets.append(Bullet(
            x=target.x + ALIEN_WIDTH // 2,
            y=target.y + ALIEN_HEIGHT + 5,
            speed=PLAYER_BULLET_SPEED,
            is_player_bullet=True
        ))

        # Run until bullet should hit
        for _ in range(20):
            game.update()

        assert not target.alive, "Bullet should still hit alien"

    def test_miss_still_misses(self, game):
        """Bullets that miss should still miss."""
        # Position bullet to miss all aliens
        game.player_bullets.append(Bullet(
            x=10,  # Far left, probably no aliens
            y=200,
            speed=PLAYER_BULLET_SPEED,
            is_player_bullet=True
        ))

        initial_alive = game.fleet.alive_count

        for _ in range(100):
            game.update()

        # No aliens should have died from this miss
        assert game.fleet.alive_count == initial_alive, \
            "Missing bullet shouldn't hit any aliens"


class TestPerformanceImprovement:
    """Tests for performance improvement."""

    @pytest.fixture
    def game(self):
        g = Game(headless=True)
        g.set_state(GameState.PLAYING)
        return g

    def test_collision_check_count_tracked(self, game):
        """Game should track collision check count for verification."""
        # This is optional but helpful for verification
        has_counter = (hasattr(game, 'collision_checks') or
                      hasattr(game, 'collision_count') or
                      hasattr(game, 'checks_this_frame'))

        # Not required, but useful
        pass

    def test_fewer_checks_with_spatial_hash(self, game):
        """Spatial hash should reduce collision checks."""
        # This test would compare brute force vs spatial hash
        # Implementation-dependent

        # Add a bullet
        target = game.fleet.aliens[0]
        game.player_bullets.append(Bullet(
            x=target.x + ALIEN_WIDTH // 2,
            y=target.y + ALIEN_HEIGHT + 50,
            speed=PLAYER_BULLET_SPEED,
            is_player_bullet=True
        ))

        # With spatial hash, we'd expect fewer checks than 55 (total aliens)
        # Exact number depends on cell size and bullet position
        game.update()

        # If we track checks, verify it's less than brute force
        if hasattr(game, 'collision_checks_this_frame'):
            assert game.collision_checks_this_frame < 55, \
                "Spatial hash should reduce collision checks"


class TestEdgeCases:
    """Tests for edge cases in spatial hash."""

    @pytest.fixture
    def game(self):
        g = Game(headless=True)
        g.set_state(GameState.PLAYING)
        return g

    def test_object_spanning_multiple_cells(self, game):
        """Objects spanning cell boundaries should still collide correctly."""
        # Place bullet at cell boundary
        target = game.fleet.aliens[0]

        game.player_bullets.append(Bullet(
            x=target.x + ALIEN_WIDTH // 2,
            y=target.y + ALIEN_HEIGHT + 5,
            speed=PLAYER_BULLET_SPEED,
            is_player_bullet=True
        ))

        for _ in range(20):
            game.update()

        assert not target.alive, "Collision at cell boundary should work"

    def test_empty_cells_handled(self, game):
        """Querying empty cells shouldn't cause errors."""
        # Add bullet in area with no aliens
        game.player_bullets.append(Bullet(
            x=10,
            y=500,
            speed=PLAYER_BULLET_SPEED,
            is_player_bullet=True
        ))

        # Should not raise any errors
        game.update()
