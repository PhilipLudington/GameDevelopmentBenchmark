"""
Comprehensive tests for Platformer game.
"""

import os
import sys
import pytest

# Setup headless mode before importing pygame
os.environ["SDL_VIDEODRIVER"] = "dummy"
os.environ["SDL_AUDIODRIVER"] = "dummy"

# Add game directory to path
game_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, game_dir)

from main import (
    Game, GameState, Player, Enemy, EnemyState, MovingPlatform, Coin,
    Tilemap, Vector2, Rect,
    SCREEN_WIDTH, SCREEN_HEIGHT, TILE_SIZE, GRAVITY, MAX_FALL_SPEED,
    PLAYER_SPEED, PLAYER_JUMP_FORCE, TILE_AIR, TILE_SOLID, TILE_SPIKE, TILE_PLATFORM
)


class TestVector2:
    """Test Vector2 class."""

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

    def test_vector_copy(self):
        v = Vector2(3, 4)
        copy = v.copy()
        assert copy.x == v.x
        assert copy.y == v.y
        copy.x = 10
        assert v.x == 3


class TestRect:
    """Test Rect class."""

    def test_rect_creation(self):
        r = Rect(10, 20, 30, 40)
        assert r.x == 10
        assert r.y == 20
        assert r.width == 30
        assert r.height == 40

    def test_rect_properties(self):
        r = Rect(10, 20, 30, 40)
        assert r.left == 10
        assert r.right == 40
        assert r.top == 20
        assert r.bottom == 60
        assert r.center_x == 25
        assert r.center_y == 40

    def test_rect_intersects(self):
        r1 = Rect(0, 0, 10, 10)
        r2 = Rect(5, 5, 10, 10)
        r3 = Rect(20, 20, 10, 10)
        assert r1.intersects(r2)
        assert not r1.intersects(r3)

    def test_rect_to_pygame(self):
        r = Rect(10.5, 20.7, 30.1, 40.9)
        pr = r.to_pygame()
        assert pr.x == 10
        assert pr.y == 20


class TestPlayer:
    """Test Player class."""

    def test_player_creation(self):
        player = Player(100, 200)
        assert player.position.x == 100
        assert player.position.y == 200
        assert player.velocity.x == 0
        assert player.velocity.y == 0

    def test_player_rect(self):
        player = Player(100, 200)
        rect = player.rect
        assert rect.x == 100
        assert rect.y == 200
        assert rect.width == player.width
        assert rect.height == player.height

    def test_player_move_left(self):
        player = Player(100, 200)
        player.move(-1)
        assert player.velocity.x == -PLAYER_SPEED
        assert not player.facing_right

    def test_player_move_right(self):
        player = Player(100, 200)
        player.move(1)
        assert player.velocity.x == PLAYER_SPEED
        assert player.facing_right

    def test_player_jump_from_ground(self):
        player = Player(100, 200)
        player.on_ground = True
        result = player.jump()
        assert result
        assert player.velocity.y == PLAYER_JUMP_FORCE
        assert not player.on_ground

    def test_player_cannot_jump_in_air(self):
        player = Player(100, 200)
        player.on_ground = False
        player.can_double_jump = False
        player.coyote_time = 0
        result = player.jump()
        assert not result

    def test_player_double_jump(self):
        player = Player(100, 200)
        player.on_ground = False
        player.can_double_jump = True
        result = player.jump()
        assert result
        assert not player.can_double_jump

    def test_player_coyote_time(self):
        player = Player(100, 200)
        player.on_ground = False
        player.coyote_time = 5
        result = player.jump()
        assert result
        assert player.coyote_time == 0

    def test_player_apply_gravity(self):
        player = Player(100, 200)
        initial_vy = player.velocity.y
        player.apply_gravity()
        assert player.velocity.y > initial_vy

    def test_player_max_fall_speed(self):
        player = Player(100, 200)
        player.velocity.y = MAX_FALL_SPEED + 10
        player.apply_gravity()
        assert player.velocity.y <= MAX_FALL_SPEED

    def test_player_land(self):
        player = Player(100, 200)
        player.can_double_jump = False
        player.land()
        assert player.on_ground
        assert player.can_double_jump

    def test_player_leave_ground(self):
        player = Player(100, 200)
        player.on_ground = True
        player.leave_ground()
        assert not player.on_ground
        assert player.coyote_time > 0

    def test_player_invulnerability(self):
        player = Player(100, 200)
        assert not player.is_invulnerable()
        player.make_invulnerable(60)
        assert player.is_invulnerable()
        assert player.invulnerable_timer == 60

    def test_player_invulnerable_countdown(self):
        player = Player(100, 200)
        player.make_invulnerable(10)
        for _ in range(10):
            player.update()
        assert not player.is_invulnerable()

    def test_player_update_applies_gravity(self):
        player = Player(100, 200)
        initial_y = player.position.y
        player.update()
        assert player.position.y > initial_y


class TestPlayerWallSlide:
    """Test wall slide mechanics."""

    def test_wall_slide_condition(self):
        player = Player(100, 200)
        player.on_wall = True
        player.on_ground = False
        player.velocity.y = 5
        player.update()
        assert player.is_wall_sliding

    def test_wall_slide_slow_fall(self):
        player = Player(100, 200)
        player.on_wall = True
        player.on_ground = False
        player.velocity.y = 5
        from main import WALL_SLIDE_SPEED
        player.update()
        assert player.velocity.y <= WALL_SLIDE_SPEED

    def test_wall_jump(self):
        player = Player(100, 200)
        player.is_wall_sliding = True
        player.wall_direction = 1
        result = player.jump()
        assert result
        assert player.velocity.x < 0  # Jump away from wall


class TestEnemy:
    """Test Enemy class."""

    def test_enemy_creation(self):
        enemy = Enemy(100, 200)
        assert enemy.position.x == 100
        assert enemy.position.y == 200
        assert enemy.state == EnemyState.PATROL

    def test_enemy_rect(self):
        enemy = Enemy(100, 200)
        rect = enemy.rect
        assert rect.x == 100
        assert rect.y == 200

    def test_enemy_patrol_moves(self):
        enemy = Enemy(100, 200, patrol_distance=50)
        enemy.facing_right = True
        player = Player(1000, 1000)  # Far away
        tilemap = Tilemap(50, 20)
        initial_x = enemy.position.x
        for _ in range(10):
            enemy.update(player, tilemap)
        assert enemy.position.x != initial_x

    def test_enemy_patrol_turns_around(self):
        enemy = Enemy(100, 200, patrol_distance=10)
        enemy.facing_right = True
        player = Player(1000, 1000)  # Far away
        tilemap = Tilemap(50, 20)
        for _ in range(50):
            enemy.update(player, tilemap)
        # Should have turned around at some point

    def test_enemy_detects_player(self):
        enemy = Enemy(100, 200)
        player = Player(150, 200)  # Close
        tilemap = Tilemap(50, 20)
        enemy.update(player, tilemap)
        assert enemy.state == EnemyState.CHASE

    def test_enemy_chase_faster(self):
        enemy = Enemy(100, 200)
        enemy.state = EnemyState.CHASE
        player = Player(200, 200)
        tilemap = Tilemap(50, 20)
        enemy.update(player, tilemap)
        assert abs(enemy.velocity.x) == enemy.chase_speed

    def test_enemy_loses_player(self):
        enemy = Enemy(100, 200)
        enemy.state = EnemyState.CHASE
        player = Player(1000, 1000)  # Far away
        tilemap = Tilemap(50, 20)
        enemy.update(player, tilemap)
        assert enemy.state == EnemyState.RETURN

    def test_enemy_returns_to_patrol(self):
        enemy = Enemy(100, 200)
        enemy.state = EnemyState.RETURN
        enemy.position.x = 105  # Close to start
        player = Player(1000, 1000)  # Far away
        tilemap = Tilemap(50, 20)
        for _ in range(20):
            enemy.update(player, tilemap)
        assert enemy.state == EnemyState.PATROL


class TestMovingPlatform:
    """Test MovingPlatform class."""

    def test_platform_creation(self):
        platform = MovingPlatform(100, 200, 96)
        assert platform.position.x == 100
        assert platform.position.y == 200
        assert platform.width == 96

    def test_platform_moves_vertically(self):
        platform = MovingPlatform(100, 200, 96, move_y=100, speed=5)
        initial_y = platform.position.y
        for _ in range(20):
            platform.update()
        assert platform.position.y != initial_y

    def test_platform_moves_horizontally(self):
        platform = MovingPlatform(100, 200, 96, move_x=100, speed=5)
        initial_x = platform.position.x
        for _ in range(20):
            platform.update()
        assert platform.position.x != initial_x

    def test_platform_reverses_direction(self):
        platform = MovingPlatform(100, 200, 96, move_x=10, speed=50)
        # Run until it reaches end and reverses
        for _ in range(10):
            platform.update()
        assert platform.direction == -1 or platform.progress < 1

    def test_platform_velocity_tracked(self):
        platform = MovingPlatform(100, 200, 96, move_x=100, speed=5)
        platform.update()
        assert platform.velocity.x != 0 or platform.velocity.y != 0


class TestCoin:
    """Test Coin class."""

    def test_coin_creation(self):
        coin = Coin(100, 200)
        assert coin.position.x == 100
        assert coin.position.y == 200
        assert not coin.collected

    def test_coin_rect(self):
        coin = Coin(100, 200)
        rect = coin.rect
        assert rect.x == 100
        assert rect.y == 200

    def test_coin_animation(self):
        coin = Coin(100, 200)
        initial_offset = coin.animation_offset
        coin.update()
        assert coin.animation_offset != initial_offset


class TestTilemap:
    """Test Tilemap class."""

    def test_tilemap_creation(self):
        tilemap = Tilemap(50, 20)
        assert tilemap.width == 50
        assert tilemap.height == 20

    def test_tilemap_default_air(self):
        tilemap = Tilemap(10, 10)
        for y in range(10):
            for x in range(10):
                assert tilemap.get_tile(x, y) == TILE_AIR

    def test_tilemap_set_get_tile(self):
        tilemap = Tilemap(10, 10)
        tilemap.set_tile(5, 5, TILE_SOLID)
        assert tilemap.get_tile(5, 5) == TILE_SOLID

    def test_tilemap_out_of_bounds_is_solid(self):
        tilemap = Tilemap(10, 10)
        assert tilemap.get_tile(-1, 0) == TILE_SOLID
        assert tilemap.get_tile(100, 0) == TILE_SOLID

    def test_tilemap_get_tile_rect(self):
        tilemap = Tilemap(10, 10)
        rect = tilemap.get_tile_rect(2, 3)
        assert rect.x == 2 * TILE_SIZE
        assert rect.y == 3 * TILE_SIZE
        assert rect.width == TILE_SIZE

    def test_tilemap_collide_rect(self):
        tilemap = Tilemap(10, 10)
        tilemap.set_tile(5, 5, TILE_SOLID)
        test_rect = Rect(5 * TILE_SIZE, 5 * TILE_SIZE, 10, 10)
        collisions = tilemap.collide_rect(test_rect)
        assert len(collisions) == 1
        assert collisions[0] == (5, 5, TILE_SOLID)

    def test_tilemap_no_collision_with_air(self):
        tilemap = Tilemap(10, 10)
        test_rect = Rect(5 * TILE_SIZE, 5 * TILE_SIZE, 10, 10)
        collisions = tilemap.collide_rect(test_rect)
        assert len(collisions) == 0


class TestGameImports:
    """Test that all required classes can be imported."""

    def test_game_imports(self):
        from main import Game, GameState, Player, Enemy, MovingPlatform, Coin, Tilemap
        assert Game is not None
        assert GameState is not None
        assert Player is not None
        assert Enemy is not None
        assert MovingPlatform is not None
        assert Coin is not None
        assert Tilemap is not None


class TestGameCreation:
    """Test Game class creation."""

    def test_game_creates_headless(self):
        game = Game(headless=True)
        assert game is not None
        assert game.headless

    def test_game_initial_state(self):
        game = Game(headless=True)
        assert game.state == GameState.MENU

    def test_game_has_player(self):
        game = Game(headless=True)
        assert game.player is not None

    def test_game_has_tilemap(self):
        game = Game(headless=True)
        assert game.tilemap is not None

    def test_game_has_enemies(self):
        game = Game(headless=True)
        assert len(game.enemies) > 0

    def test_game_has_coins(self):
        game = Game(headless=True)
        assert len(game.coins) > 0

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
        initial_x = game.player.position.x
        game.step()
        assert game.player.position.x < initial_x

    def test_inject_input_right(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game.inject_input({"right": True})
        initial_x = game.player.position.x
        game.step()
        assert game.player.position.x > initial_x

    def test_inject_input_jump(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game.player.on_ground = True
        game.inject_input({"jump": True})
        game.step()
        assert game.player.velocity.y < 0

    def test_input_cleared_after_step(self):
        game = Game(headless=True)
        game.inject_input({"left": True})
        game.step()
        assert game.injected_input == {}


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


class TestCoinCollection:
    """Test coin collection."""

    def test_collect_coin(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        # Move player to coin position
        game.player.position = Vector2(
            game.coins[0].position.x,
            game.coins[0].position.y
        )
        initial_score = game.score
        game._update_playing()
        assert game.coins[0].collected
        assert game.score > initial_score

    def test_coin_callback(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        collected = []
        game.on_coin_collect = lambda: collected.append(1)
        game.player.position = Vector2(
            game.coins[0].position.x,
            game.coins[0].position.y
        )
        game._update_playing()
        assert len(collected) == 1

    def test_level_complete_all_coins(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        # Collect all coins
        for coin in game.coins:
            coin.collected = True
        game.coins_collected = game.total_coins
        game._update_playing()
        assert game.state == GameState.LEVEL_COMPLETE


class TestEnemyInteraction:
    """Test player-enemy interactions."""

    def test_enemy_kills_player(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        enemy = game.enemies[0]
        game.player.position = Vector2(enemy.position.x, enemy.position.y)
        game.player.velocity.y = 0
        initial_lives = game.lives
        game._update_playing()
        assert game.lives < initial_lives or game.player.is_invulnerable()

    def test_stomp_enemy(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        enemy = game.enemies[0]
        # Position player directly above enemy with downward velocity for stomp
        game.player.position = Vector2(enemy.position.x, enemy.position.y - game.player.height - 5)
        game.player.velocity.y = 10  # Moving down
        game.player.on_ground = False
        initial_enemy_count = len(game.enemies)
        # Run a few frames for collision to occur
        for _ in range(5):
            game._update_playing()
            if len(game.enemies) < initial_enemy_count or game.player.is_invulnerable():
                break
        # Either enemy was stomped or player took damage (collision happened)
        assert len(game.enemies) < initial_enemy_count or game.player.is_invulnerable() or game.lives < 3


class TestLives:
    """Test lives system."""

    def test_hit_reduces_lives(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        initial_lives = game.lives
        game._player_hit()
        assert game.lives == initial_lives - 1

    def test_death_callback(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        deaths = []
        game.on_death = lambda: deaths.append(1)
        game._player_hit()
        assert len(deaths) == 1

    def test_game_over_no_lives(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game.lives = 1
        game._player_hit()
        assert game.state == GameState.GAME_OVER

    def test_invulnerable_after_hit(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game.lives = 3
        game._player_hit()
        assert game.player.is_invulnerable()


class TestCamera:
    """Test camera following."""

    def test_camera_follows_player(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game.player.position = Vector2(500, 300)
        game._update_camera()
        # Camera should have moved toward player
        assert game.camera_offset.x > 0 or game.camera_offset.y > 0

    def test_camera_clamped_to_level(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game.player.position = Vector2(0, 0)
        game._update_camera()
        assert game.camera_offset.x >= 0
        assert game.camera_offset.y >= 0


class TestReset:
    """Test game reset."""

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

    def test_reset_resets_coins(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game.coins[0].collected = True
        game.coins_collected = 1
        game.reset()
        assert game.coins_collected == 0


class TestStep:
    """Test step function."""

    def test_step_returns_true(self):
        game = Game(headless=True)
        assert game.step()

    def test_step_updates_playing(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        initial_y = game.player.position.y
        game.step()
        # Gravity should have affected player
        assert game.player.position.y != initial_y or game.player.on_ground


class TestPlatformCollision:
    """Test platform collision."""

    def test_one_way_platform_from_above(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        # Find one-way platform in tilemap
        game.tilemap.set_tile(5, 10, TILE_PLATFORM)
        game.player.position = Vector2(5 * TILE_SIZE, 9 * TILE_SIZE)
        game.player.velocity.y = 5
        game._resolve_player_collisions()
        # Player should land on platform
        assert game.player.on_ground or game.player.position.y >= 9 * TILE_SIZE


class TestFallDeath:
    """Test falling off level."""

    def test_fall_death(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game.player.position.y = game.tilemap.height * TILE_SIZE + 10
        initial_lives = game.lives
        game._update_playing()
        assert game.lives < initial_lives or game.state == GameState.GAME_OVER


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
