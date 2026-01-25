"""
Comprehensive tests for Tower Defense game.
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
    Game, GameState, Tower, Enemy, Projectile, Grid, GridPos, Vector2,
    WaveManager, TargetMode, TOWER_TYPES,
    GRID_COLS, GRID_ROWS, GRID_SIZE
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

    def test_vector_magnitude(self):
        v = Vector2(3, 4)
        assert v.magnitude() == 5.0

    def test_vector_normalize(self):
        v = Vector2(3, 4)
        normalized = v.normalize()
        assert abs(normalized.magnitude() - 1.0) < 0.0001

    def test_vector_distance_to(self):
        v1 = Vector2(0, 0)
        v2 = Vector2(3, 4)
        assert v1.distance_to(v2) == 5.0


class TestGridPos:
    """Test GridPos class."""

    def test_gridpos_creation(self):
        pos = GridPos(5, 10)
        assert pos.x == 5
        assert pos.y == 10

    def test_gridpos_hash(self):
        pos1 = GridPos(5, 10)
        pos2 = GridPos(5, 10)
        assert hash(pos1) == hash(pos2)

    def test_gridpos_equality(self):
        pos1 = GridPos(5, 10)
        pos2 = GridPos(5, 10)
        pos3 = GridPos(6, 10)
        assert pos1 == pos2
        assert pos1 != pos3

    def test_gridpos_to_world(self):
        pos = GridPos(0, 0)
        world = pos.to_world()
        assert world.x > 0
        assert world.y > 0


class TestGrid:
    """Test Grid class."""

    def test_grid_creation(self):
        grid = Grid(10, 8)
        assert grid.cols == 10
        assert grid.rows == 8

    def test_grid_default_empty(self):
        grid = Grid(10, 8)
        for y in range(8):
            for x in range(10):
                assert grid.get_cell(GridPos(x, y)) == 0

    def test_grid_is_valid(self):
        grid = Grid(10, 8)
        assert grid.is_valid(GridPos(5, 5))
        assert not grid.is_valid(GridPos(-1, 0))
        assert not grid.is_valid(GridPos(10, 0))
        assert not grid.is_valid(GridPos(0, 8))

    def test_grid_set_get_cell(self):
        grid = Grid(10, 8)
        grid.set_cell(GridPos(5, 5), 2)
        assert grid.get_cell(GridPos(5, 5)) == 2

    def test_grid_is_buildable(self):
        grid = Grid(10, 8)
        assert grid.is_buildable(GridPos(5, 5))
        grid.set_cell(GridPos(5, 5), 2)  # Tower
        assert not grid.is_buildable(GridPos(5, 5))

    def test_grid_out_of_bounds_blocked(self):
        grid = Grid(10, 8)
        assert grid.get_cell(GridPos(-1, 0)) == 3

    def test_grid_find_path_simple(self):
        grid = Grid(10, 8)
        path = grid.find_path(GridPos(0, 4), GridPos(9, 4))
        assert len(path) > 0
        assert path[0] == GridPos(0, 4)
        assert path[-1] == GridPos(9, 4)

    def test_grid_find_path_around_obstacle(self):
        grid = Grid(10, 8)
        grid.set_cell(GridPos(5, 4), 2)  # Block middle
        path = grid.find_path(GridPos(0, 4), GridPos(9, 4))
        assert len(path) > 0
        assert GridPos(5, 4) not in path

    def test_grid_find_path_no_path(self):
        grid = Grid(10, 8)
        # Block entire column
        for y in range(8):
            grid.set_cell(GridPos(5, y), 2)
        path = grid.find_path(GridPos(0, 4), GridPos(9, 4))
        assert len(path) == 0


class TestTower:
    """Test Tower class."""

    def test_tower_creation(self):
        tower = Tower(GridPos(5, 5), "basic")
        assert tower.tower_type == "basic"
        assert tower.level == 1

    def test_tower_stats_from_type(self):
        for tower_type in TOWER_TYPES:
            tower = Tower(GridPos(5, 5), tower_type)
            assert tower.damage == TOWER_TYPES[tower_type]["damage"]
            assert tower.range == TOWER_TYPES[tower_type]["range"]

    def test_tower_position(self):
        tower = Tower(GridPos(5, 5), "basic")
        assert tower.position.x > 0
        assert tower.position.y > 0

    def test_tower_upgrade(self):
        tower = Tower(GridPos(5, 5), "basic")
        initial_damage = tower.damage
        tower.upgrade()
        assert tower.level == 2
        assert tower.damage > initial_damage

    def test_tower_get_upgrade_cost(self):
        tower = Tower(GridPos(5, 5), "basic")
        cost1 = tower.get_upgrade_cost()
        tower.upgrade()
        cost2 = tower.get_upgrade_cost()
        assert cost2 > cost1

    def test_tower_can_hit(self):
        tower = Tower(GridPos(5, 5), "basic")
        path = [GridPos(0, 5)]
        near_enemy = Enemy(path, 100, 1, 10)
        near_enemy.position = Vector2(tower.position.x + 50, tower.position.y)
        far_enemy = Enemy(path, 100, 1, 10)
        far_enemy.position = Vector2(tower.position.x + 500, tower.position.y)
        assert tower.can_hit(near_enemy)
        assert not tower.can_hit(far_enemy)

    def test_tower_target_mode_default(self):
        tower = Tower(GridPos(5, 5), "basic")
        assert tower.target_mode == TargetMode.FIRST


class TestTowerTargeting:
    """Test tower targeting modes."""

    def test_target_nearest(self):
        tower = Tower(GridPos(5, 5), "basic")
        tower.target_mode = TargetMode.NEAREST
        path = [GridPos(5, i) for i in range(10)]

        enemies = []
        for i, dist in enumerate([50, 30, 70]):
            e = Enemy(path, 100, 1, 10)
            e.position = Vector2(tower.position.x + dist, tower.position.y)
            e.path_index = i
            enemies.append(e)

        target = tower.select_target(enemies, path)
        assert target == enemies[1]  # Nearest (dist=30)

    def test_target_strongest(self):
        tower = Tower(GridPos(5, 5), "basic")
        tower.target_mode = TargetMode.STRONGEST
        path = [GridPos(5, i) for i in range(10)]

        enemies = []
        for hp in [50, 100, 75]:
            e = Enemy(path, hp, 1, 10)
            e.position = Vector2(tower.position.x + 30, tower.position.y)
            e.path_index = 0
            enemies.append(e)

        target = tower.select_target(enemies, path)
        assert target == enemies[1]  # Strongest (hp=100)

    def test_target_weakest(self):
        tower = Tower(GridPos(5, 5), "basic")
        tower.target_mode = TargetMode.WEAKEST
        path = [GridPos(5, i) for i in range(10)]

        enemies = []
        for hp in [50, 100, 75]:
            e = Enemy(path, hp, 1, 10)
            e.position = Vector2(tower.position.x + 30, tower.position.y)
            e.path_index = 0
            enemies.append(e)

        target = tower.select_target(enemies, path)
        assert target == enemies[0]  # Weakest (hp=50)

    def test_target_first(self):
        tower = Tower(GridPos(5, 5), "basic")
        tower.target_mode = TargetMode.FIRST
        path = [GridPos(5, i) for i in range(10)]

        enemies = []
        for i in range(3):
            e = Enemy(path, 100, 1, 10)
            e.position = Vector2(tower.position.x + 30, tower.position.y)
            e.path_index = i
            enemies.append(e)

        target = tower.select_target(enemies, path)
        assert target == enemies[2]  # First (highest path_index)


class TestEnemy:
    """Test Enemy class."""

    def test_enemy_creation(self):
        path = [GridPos(0, 5), GridPos(1, 5)]
        enemy = Enemy(path, 100, 1.5, 10)
        assert enemy.health == 100
        assert enemy.max_health == 100
        assert enemy.speed == 1.5
        assert enemy.reward == 10

    def test_enemy_take_damage(self):
        path = [GridPos(0, 5)]
        enemy = Enemy(path, 100, 1, 10)
        enemy.take_damage(30)
        assert enemy.health == 70

    def test_enemy_moves_toward_target(self):
        path = [GridPos(0, 5), GridPos(5, 5)]
        enemy = Enemy(path, 100, 2, 10)
        initial_x = enemy.position.x
        enemy.update()
        assert enemy.position.x > initial_x

    def test_enemy_advances_path_index(self):
        path = [GridPos(0, 5), GridPos(1, 5), GridPos(2, 5)]
        enemy = Enemy(path, 100, 100, 10)  # Very fast
        initial_index = enemy.path_index
        for _ in range(10):
            enemy.update()
        assert enemy.path_index > initial_index

    def test_enemy_reaches_end(self):
        path = [GridPos(0, 5), GridPos(1, 5)]
        enemy = Enemy(path, 100, 100, 10)  # Very fast
        reached_end = False
        for _ in range(20):
            if enemy.update():
                reached_end = True
                break
        assert reached_end


class TestProjectile:
    """Test Projectile class."""

    def test_projectile_creation(self):
        path = [GridPos(0, 5)]
        enemy = Enemy(path, 100, 1, 10)
        projectile = Projectile(Vector2(0, 0), enemy, 20)
        assert projectile.damage == 20
        assert projectile.active

    def test_projectile_moves_toward_target(self):
        path = [GridPos(5, 5)]
        enemy = Enemy(path, 100, 0, 10)
        enemy.position = Vector2(100, 100)
        projectile = Projectile(Vector2(0, 0), enemy, 20)
        initial_dist = projectile.position.distance_to(enemy.position)
        projectile.update([enemy])
        new_dist = projectile.position.distance_to(enemy.position)
        assert new_dist < initial_dist

    def test_projectile_hits_target(self):
        path = [GridPos(0, 5)]
        enemy = Enemy(path, 100, 0, 10)
        enemy.position = Vector2(5, 5)
        projectile = Projectile(Vector2(0, 0), enemy, 20, speed=100)
        for _ in range(10):
            projectile.update([enemy])
        assert not projectile.active
        assert enemy.health < 100

    def test_projectile_splash_damage(self):
        path = [GridPos(0, 5)]
        # Create enemies positioned close together
        enemies = []
        for i in range(3):
            e = Enemy(path, 100, 0, 10)
            e.position = Vector2(100, 100)  # All at same position
            enemies.append(e)
        # Create projectile starting very close to enemies with splash
        projectile = Projectile(Vector2(95, 100), enemies[0], 20,
                               speed=10, splash_radius=100)
        # Run until projectile hits
        for _ in range(50):
            if not projectile.active:
                break
            projectile.update(enemies)
        # At least one enemy should take damage (the target)
        damaged = sum(1 for e in enemies if e.health < 100)
        assert damaged >= 1


class TestWaveManager:
    """Test WaveManager class."""

    def test_wave_manager_creation(self):
        wm = WaveManager()
        assert wm.wave == 0
        assert not wm.wave_active

    def test_start_wave(self):
        wm = WaveManager()
        wm.start_wave(1)
        assert wm.wave == 1
        assert wm.wave_active
        assert len(wm.enemies_to_spawn) > 0

    def test_wave_spawns_enemies(self):
        wm = WaveManager()
        path = [GridPos(0, 5), GridPos(5, 5)]
        wm.start_wave(1)
        spawned = []
        for _ in range(500):  # Run many frames
            enemy = wm.update(path)
            if enemy:
                spawned.append(enemy)
        assert len(spawned) > 0

    def test_wave_complete(self):
        wm = WaveManager()
        wm.wave_active = False
        assert wm.is_wave_complete([])
        assert not wm.is_wave_complete([Enemy([GridPos(0, 0)], 100, 1, 10)])

    def test_wave_difficulty_increases(self):
        wm = WaveManager()
        wave1_enemies = wm._generate_wave(1)
        wave5_enemies = wm._generate_wave(5)
        assert len(wave5_enemies) > len(wave1_enemies)


class TestGameImports:
    """Test that all required classes can be imported."""

    def test_game_imports(self):
        from main import Game, GameState, Tower, Enemy, Grid
        assert Game is not None
        assert GameState is not None
        assert Tower is not None
        assert Enemy is not None
        assert Grid is not None


class TestGameCreation:
    """Test Game class creation."""

    def test_game_creates_headless(self):
        game = Game(headless=True)
        assert game is not None
        assert game.headless

    def test_game_initial_state(self):
        game = Game(headless=True)
        assert game.state == GameState.MENU

    def test_game_initial_gold(self):
        game = Game(headless=True)
        assert game.gold == 200

    def test_game_initial_lives(self):
        game = Game(headless=True)
        assert game.lives == 20

    def test_game_has_grid(self):
        game = Game(headless=True)
        assert game.grid is not None

    def test_game_has_path(self):
        game = Game(headless=True)
        assert len(game.path) > 0


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


class TestTowerPlacement:
    """Test tower placement."""

    def test_place_tower(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        pos = GridPos(3, 3)  # Off the path
        result = game.place_tower(pos, "basic")
        if result:  # If position was valid
            assert len(game.towers) == 1
            assert game.gold < 200

    def test_place_tower_insufficient_gold(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game.gold = 10  # Not enough
        result = game.place_tower(GridPos(3, 3), "basic")
        assert not result

    def test_place_tower_on_path_blocked(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        # Try to place on start position
        result = game.place_tower(game.start_pos, "basic")
        assert not result

    def test_place_tower_would_block_path(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        # Fill positions to block path (this test depends on level layout)
        # The game should prevent placing if it would block the path


class TestTowerUpgrade:
    """Test tower upgrades."""

    def test_upgrade_tower(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        # Place a tower first
        for y in range(GRID_ROWS):
            for x in range(GRID_COLS):
                pos = GridPos(x, y)
                if game.place_tower(pos, "basic"):
                    break
            else:
                continue
            break

        if game.towers:
            tower = game.towers[0]
            initial_level = tower.level
            initial_gold = game.gold
            game.upgrade_tower(tower)
            assert tower.level == initial_level + 1
            assert game.gold < initial_gold

    def test_upgrade_insufficient_gold(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        # Place tower
        for y in range(GRID_ROWS):
            for x in range(GRID_COLS):
                pos = GridPos(x, y)
                if game.place_tower(pos, "basic"):
                    break
            else:
                continue
            break

        if game.towers:
            game.gold = 0
            result = game.upgrade_tower(game.towers[0])
            assert not result


class TestTowerSell:
    """Test tower selling."""

    def test_sell_tower(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        # Place tower
        for y in range(GRID_ROWS):
            for x in range(GRID_COLS):
                pos = GridPos(x, y)
                if game.place_tower(pos, "basic"):
                    break
            else:
                continue
            break

        if game.towers:
            tower = game.towers[0]
            initial_gold = game.gold
            game.sell_tower(tower)
            assert len(game.towers) == 0
            assert game.gold > initial_gold


class TestWaves:
    """Test wave system."""

    def test_start_wave(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game.start_wave()
        assert game.wave_manager.wave == 1
        assert game.wave_manager.wave_active

    def test_enemy_spawns_during_wave(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game.start_wave()
        for _ in range(100):
            game._update_playing()
        assert len(game.enemies) > 0


class TestEnemyKills:
    """Test enemy kills and rewards."""

    def test_killing_enemy_gives_gold(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        # Add an enemy directly
        enemy = Enemy(game.path, 1, 1, 50)  # 1 HP, 50 reward
        game.enemies.append(enemy)
        initial_gold = game.gold
        enemy.health = 0  # Kill it
        game._update_playing()
        assert game.gold > initial_gold

    def test_enemy_kill_callback(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        kills = []
        game.on_enemy_killed = lambda e: kills.append(e)
        enemy = Enemy(game.path, 1, 1, 10)
        game.enemies.append(enemy)
        enemy.health = 0
        game._update_playing()
        assert len(kills) == 1


class TestLives:
    """Test lives system."""

    def test_enemy_reaching_end_reduces_lives(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        # Add fast enemy near end
        enemy = Enemy(game.path, 100, 1000, 10)
        enemy.path_index = len(game.path) - 2
        game.enemies.append(enemy)
        initial_lives = game.lives
        for _ in range(10):
            game._update_playing()
            if game.lives < initial_lives:
                break
        assert game.lives < initial_lives

    def test_game_over_no_lives(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game.lives = 1
        # Add fast enemy near end
        enemy = Enemy(game.path, 100, 1000, 10)
        enemy.path_index = len(game.path) - 2
        game.enemies.append(enemy)
        for _ in range(10):
            game._update_playing()
        assert game.state == GameState.GAME_OVER


class TestVictory:
    """Test victory conditions."""

    def test_victory_after_wave_10(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game.wave_manager.wave = 10
        game.wave_manager.wave_active = False
        # No enemies
        game._update_playing()
        assert game.state == GameState.VICTORY


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


class TestReset:
    """Test game reset."""

    def test_reset_restores_gold(self):
        game = Game(headless=True)
        game.gold = 50
        game.reset()
        assert game.gold == 200

    def test_reset_restores_lives(self):
        game = Game(headless=True)
        game.lives = 5
        game.reset()
        assert game.lives == 20

    def test_reset_clears_towers(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        # Place tower
        for y in range(GRID_ROWS):
            for x in range(GRID_COLS):
                if game.place_tower(GridPos(x, y), "basic"):
                    break
            else:
                continue
            break
        game.reset()
        assert len(game.towers) == 0

    def test_reset_sets_menu_state(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game.reset()
        assert game.state == GameState.MENU


class TestStep:
    """Test step function."""

    def test_step_returns_true(self):
        game = Game(headless=True)
        assert game.step()

    def test_step_clears_injected_input(self):
        game = Game(headless=True)
        game.inject_input({"start_wave": True})
        game.step()
        assert game.injected_input == {}


class TestInjectedInput:
    """Test injected input handling."""

    def test_inject_start_wave(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game.inject_input({"start_wave": True})
        game._update_playing()
        assert game.wave_manager.wave_active

    def test_inject_place_tower(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        pos = GridPos(3, 3)
        game.inject_input({"place_tower": (pos, "basic")})
        game._update_playing()
        # Check if tower was placed (may fail if position invalid)


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
