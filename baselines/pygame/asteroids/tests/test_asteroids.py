"""
Comprehensive tests for Asteroids game.
"""

import os
import sys
import math
import pytest

# Setup headless mode before importing pygame
os.environ["SDL_VIDEODRIVER"] = "dummy"
os.environ["SDL_AUDIODRIVER"] = "dummy"

# Add game directory to path
game_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, game_dir)

from main import (
    Game, GameState, Ship, Bullet, Asteroid, UFO, Vector2, Particle,
    SCREEN_WIDTH, SCREEN_HEIGHT, SHIP_SIZE, BULLET_SPEED, MAX_BULLETS,
    ASTEROID_RADII, ASTEROID_SCORES, SHIP_INVULNERABLE_TIME, UFO_SCORE
)


class TestVector2:
    """Test Vector2 class operations."""

    def test_vector_creation(self):
        v = Vector2(3, 4)
        assert v.x == 3
        assert v.y == 4

    def test_vector_addition(self):
        v1 = Vector2(1, 2)
        v2 = Vector2(3, 4)
        result = v1 + v2
        assert result.x == 4
        assert result.y == 6

    def test_vector_subtraction(self):
        v1 = Vector2(5, 7)
        v2 = Vector2(2, 3)
        result = v1 - v2
        assert result.x == 3
        assert result.y == 4

    def test_vector_multiplication(self):
        v = Vector2(2, 3)
        result = v * 2
        assert result.x == 4
        assert result.y == 6

    def test_vector_rmul(self):
        v = Vector2(2, 3)
        result = 2 * v
        assert result.x == 4
        assert result.y == 6

    def test_vector_magnitude(self):
        v = Vector2(3, 4)
        assert v.magnitude() == 5.0

    def test_vector_normalize(self):
        v = Vector2(3, 4)
        normalized = v.normalize()
        assert abs(normalized.magnitude() - 1.0) < 0.0001

    def test_vector_normalize_zero(self):
        v = Vector2(0, 0)
        normalized = v.normalize()
        assert normalized.x == 0
        assert normalized.y == 0

    def test_vector_limit(self):
        v = Vector2(10, 0)
        limited = v.limit(5)
        assert limited.magnitude() <= 5.0

    def test_vector_limit_under(self):
        v = Vector2(3, 0)
        limited = v.limit(5)
        assert limited.x == 3

    def test_vector_from_angle(self):
        v = Vector2.from_angle(0, 1)
        assert abs(v.x - 1) < 0.0001
        assert abs(v.y) < 0.0001

    def test_vector_from_angle_90(self):
        v = Vector2.from_angle(90, 1)
        assert abs(v.x) < 0.0001
        assert abs(v.y - 1) < 0.0001

    def test_vector_distance_to(self):
        v1 = Vector2(0, 0)
        v2 = Vector2(3, 4)
        assert v1.distance_to(v2) == 5.0

    def test_vector_to_tuple(self):
        v = Vector2(3.5, 4.5)
        assert v.to_tuple() == (3.5, 4.5)


class TestShip:
    """Test Ship class."""

    def test_ship_creation(self):
        ship = Ship(100, 200)
        assert ship.position.x == 100
        assert ship.position.y == 200
        assert ship.angle == -90  # Pointing up

    def test_ship_rotate_left(self):
        ship = Ship(100, 100)
        initial_angle = ship.angle
        ship.rotate_left()
        assert ship.angle < initial_angle

    def test_ship_rotate_right(self):
        ship = Ship(100, 100)
        initial_angle = ship.angle
        ship.rotate_right()
        assert ship.angle > initial_angle

    def test_ship_thrust(self):
        ship = Ship(100, 100)
        initial_velocity = ship.velocity.magnitude()
        ship.thrust()
        assert ship.thrusting
        assert ship.velocity.magnitude() > initial_velocity

    def test_ship_thrust_respects_max_speed(self):
        ship = Ship(100, 100)
        for _ in range(100):
            ship.thrust()
            ship.update()
        # Velocity should be limited
        from main import SHIP_MAX_SPEED
        assert ship.velocity.magnitude() <= SHIP_MAX_SPEED + 0.1

    def test_ship_drag(self):
        ship = Ship(100, 100)
        ship.velocity = Vector2(10, 0)
        ship.update()
        assert ship.velocity.x < 10

    def test_ship_wrap_position_right(self):
        ship = Ship(SCREEN_WIDTH + 10, 100)
        ship.wrap_position()
        assert ship.position.x == 0

    def test_ship_wrap_position_left(self):
        ship = Ship(-10, 100)
        ship.wrap_position()
        assert ship.position.x == SCREEN_WIDTH

    def test_ship_wrap_position_bottom(self):
        ship = Ship(100, SCREEN_HEIGHT + 10)
        ship.wrap_position()
        assert ship.position.y == 0

    def test_ship_wrap_position_top(self):
        ship = Ship(100, -10)
        ship.wrap_position()
        assert ship.position.y == SCREEN_HEIGHT

    def test_ship_invulnerability(self):
        ship = Ship(100, 100)
        assert not ship.is_invulnerable()
        ship.make_invulnerable()
        assert ship.is_invulnerable()
        assert ship.invulnerable_timer == SHIP_INVULNERABLE_TIME

    def test_ship_invulnerable_countdown(self):
        ship = Ship(100, 100)
        ship.make_invulnerable()
        for _ in range(SHIP_INVULNERABLE_TIME):
            ship.update()
        assert not ship.is_invulnerable()

    def test_ship_get_points(self):
        ship = Ship(100, 100)
        points = ship.get_points()
        assert len(points) == 3  # Triangle

    def test_ship_thrust_particles(self):
        ship = Ship(100, 100)
        ship.thrusting = False
        assert ship.get_thrust_particles() == []
        ship.thrusting = True
        particles = ship.get_thrust_particles()
        assert len(particles) > 0


class TestBullet:
    """Test Bullet class."""

    def test_bullet_creation(self):
        bullet = Bullet(100, 100, 0)
        assert bullet.position.x == 100
        assert bullet.position.y == 100
        assert bullet.is_player

    def test_bullet_enemy(self):
        bullet = Bullet(100, 100, 0, is_player=False)
        assert not bullet.is_player

    def test_bullet_velocity(self):
        bullet = Bullet(100, 100, 0)
        assert abs(bullet.velocity.x - BULLET_SPEED) < 0.1
        assert abs(bullet.velocity.y) < 0.1

    def test_bullet_update(self):
        bullet = Bullet(100, 100, 0)
        initial_x = bullet.position.x
        bullet.update()
        assert bullet.position.x > initial_x

    def test_bullet_lifetime(self):
        bullet = Bullet(100, 100, 0)
        from main import BULLET_LIFETIME
        assert bullet.lifetime == BULLET_LIFETIME
        alive = bullet.update()
        assert alive
        assert bullet.lifetime == BULLET_LIFETIME - 1

    def test_bullet_expires(self):
        bullet = Bullet(100, 100, 0)
        bullet.lifetime = 1
        alive = bullet.update()
        assert not alive

    def test_bullet_wrap(self):
        bullet = Bullet(SCREEN_WIDTH + 10, 100, 0)
        bullet.wrap_position()
        assert bullet.position.x == 0


class TestAsteroid:
    """Test Asteroid class."""

    def test_asteroid_creation(self):
        asteroid = Asteroid(100, 100, 3)
        assert asteroid.position.x == 100
        assert asteroid.position.y == 100
        assert asteroid.size == 3

    def test_asteroid_radius_by_size(self):
        for size in [1, 2, 3]:
            asteroid = Asteroid(100, 100, size)
            assert asteroid.radius == ASTEROID_RADII[size]

    def test_asteroid_score_by_size(self):
        for size in [1, 2, 3]:
            asteroid = Asteroid(100, 100, size)
            assert asteroid.score == ASTEROID_SCORES[size]

    def test_asteroid_has_velocity(self):
        asteroid = Asteroid(100, 100, 3)
        assert asteroid.velocity.magnitude() > 0

    def test_asteroid_custom_velocity(self):
        vel = Vector2(5, 0)
        asteroid = Asteroid(100, 100, 3, vel)
        assert asteroid.velocity.x == 5
        assert asteroid.velocity.y == 0

    def test_asteroid_vertices(self):
        asteroid = Asteroid(100, 100, 3)
        assert len(asteroid.vertices) >= 8
        assert len(asteroid.vertices) <= 12

    def test_asteroid_update_moves(self):
        asteroid = Asteroid(100, 100, 3, Vector2(5, 0))
        asteroid.update()
        assert asteroid.position.x == 105

    def test_asteroid_wrap(self):
        asteroid = Asteroid(SCREEN_WIDTH + 50, 100, 3)
        asteroid.wrap_position()
        assert asteroid.position.x < 0

    def test_asteroid_split_large(self):
        asteroid = Asteroid(100, 100, 3)
        pieces = asteroid.split()
        assert len(pieces) == 2
        assert all(a.size == 2 for a in pieces)

    def test_asteroid_split_medium(self):
        asteroid = Asteroid(100, 100, 2)
        pieces = asteroid.split()
        assert len(pieces) == 2
        assert all(a.size == 1 for a in pieces)

    def test_asteroid_split_small(self):
        asteroid = Asteroid(100, 100, 1)
        pieces = asteroid.split()
        assert len(pieces) == 0

    def test_asteroid_collides_with_point(self):
        asteroid = Asteroid(100, 100, 3)
        assert asteroid.collides_with_point(Vector2(100, 100))
        assert asteroid.collides_with_point(Vector2(110, 100))
        assert not asteroid.collides_with_point(Vector2(200, 200))

    def test_asteroid_collides_with_circle(self):
        asteroid = Asteroid(100, 100, 3)
        assert asteroid.collides_with_circle(Vector2(150, 100), 20)
        assert not asteroid.collides_with_circle(Vector2(200, 100), 10)


class TestUFO:
    """Test UFO class."""

    def test_ufo_creation(self):
        ship = Ship(SCREEN_WIDTH // 2, SCREEN_HEIGHT // 2)
        ufo = UFO(ship)
        assert ufo.target == ship
        # UFO spawns from edge
        assert ufo.position.x < 0 or ufo.position.x > SCREEN_WIDTH

    def test_ufo_moves(self):
        ship = Ship(SCREEN_WIDTH // 2, SCREEN_HEIGHT // 2)
        ufo = UFO(ship)
        initial_x = ufo.position.x
        ufo.update()
        assert ufo.position.x != initial_x

    def test_ufo_shoots(self):
        ship = Ship(SCREEN_WIDTH // 2, SCREEN_HEIGHT // 2)
        ufo = UFO(ship)
        ufo.shoot_timer = 1
        bullet = ufo.update()
        assert bullet is not None
        assert not bullet.is_player

    def test_ufo_offscreen_detection(self):
        ship = Ship(SCREEN_WIDTH // 2, SCREEN_HEIGHT // 2)
        ufo = UFO(ship)
        ufo.position.x = -100
        assert ufo.is_offscreen()

    def test_ufo_collides_with_point(self):
        ship = Ship(SCREEN_WIDTH // 2, SCREEN_HEIGHT // 2)
        ufo = UFO(ship)
        ufo.position = Vector2(100, 100)
        assert ufo.collides_with_point(Vector2(100, 100))


class TestParticle:
    """Test Particle class."""

    def test_particle_creation(self):
        p = Particle(Vector2(100, 100), Vector2(1, 0), 30, (255, 255, 255))
        assert p.lifetime == 30

    def test_particle_update(self):
        p = Particle(Vector2(100, 100), Vector2(1, 0), 30, (255, 255, 255))
        alive = p.update()
        assert alive
        assert p.lifetime == 29
        assert p.position.x > 100

    def test_particle_dies(self):
        p = Particle(Vector2(100, 100), Vector2(1, 0), 1, (255, 255, 255))
        alive = p.update()
        assert not alive


class TestGameImports:
    """Test that all required classes can be imported."""

    def test_game_imports(self):
        from main import Game, GameState, Ship, Bullet, Asteroid, UFO
        assert Game is not None
        assert GameState is not None
        assert Ship is not None
        assert Bullet is not None
        assert Asteroid is not None
        assert UFO is not None


class TestGameCreation:
    """Test Game class creation."""

    def test_game_creates_headless(self):
        game = Game(headless=True)
        assert game is not None
        assert game.headless

    def test_game_initial_state(self):
        game = Game(headless=True)
        assert game.state == GameState.MENU

    def test_game_has_ship(self):
        game = Game(headless=True)
        assert game.ship is not None

    def test_game_has_asteroids(self):
        game = Game(headless=True)
        assert len(game.asteroids) > 0

    def test_game_initial_lives(self):
        game = Game(headless=True)
        assert game.lives == 3

    def test_game_initial_score(self):
        game = Game(headless=True)
        assert game.score == 0


class TestGameState:
    """Test game state transitions."""

    def test_set_state(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        assert game.state == GameState.PLAYING

    def test_state_callback(self):
        game = Game(headless=True)
        states = []
        game.on_state_change = lambda s: states.append(s)
        game.set_state(GameState.PLAYING)
        assert GameState.PLAYING in states

    def test_state_callback_not_called_for_same_state(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        states = []
        game.on_state_change = lambda s: states.append(s)
        game.set_state(GameState.PLAYING)
        assert len(states) == 0


class TestGameInput:
    """Test game input handling."""

    def test_inject_input(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game.inject_input({"left": True})
        initial_angle = game.ship.angle
        game.step()
        assert game.ship.angle < initial_angle

    def test_inject_input_right(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game.inject_input({"right": True})
        initial_angle = game.ship.angle
        game.step()
        assert game.ship.angle > initial_angle

    def test_inject_input_thrust(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game.inject_input({"thrust": True})
        game.step()
        assert game.ship.velocity.magnitude() > 0

    def test_input_cleared_after_step(self):
        game = Game(headless=True)
        game.inject_input({"left": True})
        game.set_state(GameState.PLAYING)
        game.step()
        assert game.injected_input == {}


class TestShooting:
    """Test shooting mechanics."""

    def test_shooting_creates_bullet(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game._shoot()
        assert len(game.bullets) == 1

    def test_max_bullets(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        for _ in range(MAX_BULLETS + 5):
            game._shoot()
        assert len(game.bullets) == MAX_BULLETS

    def test_bullet_is_player(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game._shoot()
        assert game.bullets[0].is_player


class TestCollisions:
    """Test collision detection."""

    def test_bullet_asteroid_collision(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game.asteroids = [Asteroid(100, 100, 3)]
        game.bullets = [Bullet(100, 100, 0)]
        initial_score = game.score
        game._check_collisions()
        assert game.score > initial_score

    def test_asteroid_splits_on_hit(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game.asteroids = [Asteroid(100, 100, 3)]
        game.bullets = [Bullet(100, 100, 0)]
        game._check_collisions()
        # Large asteroid splits into 2 medium
        assert len(game.asteroids) == 2
        assert all(a.size == 2 for a in game.asteroids)

    def test_small_asteroid_destroyed(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game.asteroids = [Asteroid(100, 100, 1)]
        game.bullets = [Bullet(100, 100, 0)]
        game._check_collisions()
        assert len(game.asteroids) == 0

    def test_ship_asteroid_collision(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game.ship.position = Vector2(100, 100)
        game.asteroids = [Asteroid(100, 100, 3)]
        initial_lives = game.lives
        game._check_collisions()
        assert game.lives == initial_lives - 1

    def test_ship_invulnerable_no_collision(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game.ship.position = Vector2(100, 100)
        game.ship.make_invulnerable()
        game.asteroids = [Asteroid(100, 100, 3)]
        initial_lives = game.lives
        game._check_collisions()
        assert game.lives == initial_lives

    def test_bullet_ufo_collision(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game.ufo = UFO(game.ship)
        game.ufo.position = Vector2(100, 100)
        game.bullets = [Bullet(100, 100, 0)]
        game._check_collisions()
        assert game.ufo is None
        assert game.score == UFO_SCORE


class TestScoring:
    """Test scoring system."""

    def test_add_score(self):
        game = Game(headless=True)
        game.add_score(100)
        assert game.score == 100

    def test_score_callback(self):
        game = Game(headless=True)
        scores = []
        game.on_score = lambda s: scores.append(s)
        game.add_score(100)
        assert 100 in scores

    def test_ufo_spawns_at_score_threshold(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        assert game.ufo is None
        game.add_score(1000)
        assert game.ufo is not None


class TestLives:
    """Test lives system."""

    def test_death_reduces_lives(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        initial_lives = game.lives
        game._player_death()
        assert game.lives == initial_lives - 1

    def test_death_callback(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        deaths = []
        game.on_death = lambda: deaths.append(1)
        game._player_death()
        assert len(deaths) == 1

    def test_game_over_on_zero_lives(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game.lives = 1
        game._player_death()
        assert game.state == GameState.GAME_OVER

    def test_respawn_after_death(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game.lives = 3
        game._player_death()
        assert game.ship.is_invulnerable()
        assert game.ship.position.x == SCREEN_WIDTH // 2


class TestLevels:
    """Test level progression."""

    def test_next_level_increases_level(self):
        game = Game(headless=True)
        initial_level = game.level
        game._next_level()
        assert game.level == initial_level + 1

    def test_next_level_spawns_asteroids(self):
        game = Game(headless=True)
        game.asteroids = []
        game._next_level()
        assert len(game.asteroids) > 0

    def test_level_cleared_when_no_asteroids(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game.asteroids = []
        initial_level = game.level
        game._update_playing()
        assert game.level == initial_level + 1


class TestReset:
    """Test game reset."""

    def test_reset_clears_bullets(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game._shoot()
        game.reset()
        assert len(game.bullets) == 0

    def test_reset_restores_lives(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game.lives = 1
        game.reset()
        assert game.lives == 3

    def test_reset_clears_score(self):
        game = Game(headless=True)
        game.add_score(1000)
        game.reset()
        assert game.score == 0

    def test_reset_sets_menu_state(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game.reset()
        assert game.state == GameState.MENU


class TestExplosions:
    """Test explosion particle effects."""

    def test_spawn_explosion_creates_particles(self):
        game = Game(headless=True)
        game._spawn_explosion(Vector2(100, 100))
        assert len(game.particles) > 0

    def test_spawn_explosion_custom_count(self):
        game = Game(headless=True)
        game._spawn_explosion(Vector2(100, 100), count=10)
        assert len(game.particles) == 10

    def test_particles_update(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game._spawn_explosion(Vector2(100, 100), count=5)
        initial = len(game.particles)
        # Run many frames to let particles die
        for _ in range(100):
            game._update_playing()
        assert len(game.particles) < initial


class TestStep:
    """Test step function."""

    def test_step_returns_true(self):
        game = Game(headless=True)
        assert game.step()

    def test_step_updates_playing(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game.ship.velocity = Vector2(5, 0)
        initial_x = game.ship.position.x
        game.step()
        assert game.ship.position.x != initial_x

    def test_step_does_not_update_menu(self):
        game = Game(headless=True)
        game.ship.velocity = Vector2(5, 0)
        initial_x = game.ship.position.x
        game.step()
        # Ship should not move in menu state
        assert game.ship.position.x == initial_x


class TestThrustParticles:
    """Test thrust particle generation."""

    def test_thrust_generates_particles(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game.inject_input({"thrust": True})
        game.step()
        assert len(game.particles) > 0


class TestScreenWrapping:
    """Test screen wrapping for all entities."""

    def test_ship_wraps(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game.ship.position = Vector2(SCREEN_WIDTH + 10, 100)
        game.ship.update()
        assert game.ship.position.x < SCREEN_WIDTH

    def test_bullet_wraps(self):
        bullet = Bullet(SCREEN_WIDTH + 10, 100, 0)
        bullet.wrap_position()
        assert bullet.position.x < SCREEN_WIDTH

    def test_asteroid_wraps(self):
        asteroid = Asteroid(SCREEN_WIDTH + 100, 100, 3)
        asteroid.wrap_position()
        assert asteroid.position.x < 0


class TestUFOIntegration:
    """Test UFO behavior in game."""

    def test_ufo_spawns_on_score(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game.add_score(1000)
        assert game.ufo is not None

    def test_ufo_removed_when_offscreen(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game.ufo = UFO(game.ship)
        game.ufo.position = Vector2(-200, 100)
        game._update_playing()
        assert game.ufo is None

    def test_ufo_bullet_added_to_game(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game.ufo = UFO(game.ship)
        game.ufo.shoot_timer = 1
        initial_bullets = len(game.bullets)
        game._update_playing()
        assert len(game.bullets) > initial_bullets


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
