#!/usr/bin/env python3
"""Test suite for Space Invaders baseline game."""

import os
import sys

# Set up headless mode BEFORE importing pygame
os.environ["SDL_VIDEODRIVER"] = "dummy"
os.environ["SDL_AUDIODRIVER"] = "dummy"

# Add parent directory to path for imports
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

import pytest
from main import (
    Game, GameState, Player, Bullet, Alien, AlienFleet, Shield, ShieldBlock, Vector2,
    SCREEN_WIDTH, SCREEN_HEIGHT, PLAYER_WIDTH, PLAYER_SPEED, PLAYER_Y,
    ALIEN_ROWS, ALIEN_COLS, ALIEN_WIDTH, ALIEN_HEIGHT, INITIAL_LIVES,
    BULLET_WIDTH, BULLET_HEIGHT, PLAYER_BULLET_SPEED, ALIEN_BULLET_SPEED,
    POINTS_PER_ROW, SHIELD_COUNT
)


class TestVector2:
    """Tests for Vector2 dataclass."""

    def test_creation(self):
        v = Vector2(3, 4)
        assert v.x == 3
        assert v.y == 4

    def test_addition(self):
        v1 = Vector2(1, 2)
        v2 = Vector2(3, 4)
        result = v1 + v2
        assert result.x == 4
        assert result.y == 6

    def test_subtraction(self):
        v1 = Vector2(5, 7)
        v2 = Vector2(2, 3)
        result = v1 - v2
        assert result.x == 3
        assert result.y == 4

    def test_copy(self):
        v1 = Vector2(1, 2)
        v2 = v1.copy()
        v2.x = 10
        assert v1.x == 1
        assert v2.x == 10


class TestPlayer:
    """Tests for Player class."""

    def test_initialization(self):
        player = Player()
        assert player.width == PLAYER_WIDTH
        assert player.y == PLAYER_Y
        # Should be centered
        assert player.x == (SCREEN_WIDTH - PLAYER_WIDTH) // 2

    def test_move_left(self):
        player = Player()
        initial_x = player.x
        player.move_left()
        assert player.x == initial_x - PLAYER_SPEED

    def test_move_right(self):
        player = Player()
        initial_x = player.x
        player.move_right()
        assert player.x == initial_x + PLAYER_SPEED

    def test_left_boundary(self):
        player = Player()
        player.x = 3
        player.move_left()
        assert player.x == 0

    def test_right_boundary(self):
        player = Player()
        player.x = SCREEN_WIDTH - PLAYER_WIDTH - 3
        player.move_right()
        assert player.x == SCREEN_WIDTH - PLAYER_WIDTH

    def test_reset(self):
        player = Player()
        player.x = 100
        player.reset()
        assert player.x == (SCREEN_WIDTH - PLAYER_WIDTH) // 2

    def test_center_x(self):
        player = Player()
        expected_center = player.x + player.width / 2
        assert player.center_x == expected_center

    def test_rect(self):
        player = Player()
        rect = player.rect
        assert rect.x == player.x
        assert rect.y == player.y
        assert rect.width == player.width
        assert rect.height == player.height


class TestBullet:
    """Tests for Bullet class."""

    def test_player_bullet_creation(self):
        bullet = Bullet(x=100, y=200, speed=PLAYER_BULLET_SPEED, is_player_bullet=True)
        assert bullet.x == 100
        assert bullet.y == 200
        assert bullet.is_player_bullet is True

    def test_alien_bullet_creation(self):
        bullet = Bullet(x=100, y=200, speed=ALIEN_BULLET_SPEED, is_player_bullet=False)
        assert bullet.is_player_bullet is False

    def test_player_bullet_moves_up(self):
        bullet = Bullet(x=100, y=200, speed=PLAYER_BULLET_SPEED, is_player_bullet=True)
        bullet.update()
        assert bullet.y == 200 - PLAYER_BULLET_SPEED

    def test_alien_bullet_moves_down(self):
        bullet = Bullet(x=100, y=200, speed=ALIEN_BULLET_SPEED, is_player_bullet=False)
        bullet.update()
        assert bullet.y == 200 + ALIEN_BULLET_SPEED

    def test_off_screen_top(self):
        bullet = Bullet(x=100, y=-BULLET_HEIGHT - 1, speed=PLAYER_BULLET_SPEED, is_player_bullet=True)
        assert bullet.is_off_screen() is True

    def test_off_screen_bottom(self):
        bullet = Bullet(x=100, y=SCREEN_HEIGHT + 1, speed=ALIEN_BULLET_SPEED, is_player_bullet=False)
        assert bullet.is_off_screen() is True

    def test_on_screen(self):
        bullet = Bullet(x=100, y=300, speed=PLAYER_BULLET_SPEED, is_player_bullet=True)
        assert bullet.is_off_screen() is False

    def test_rect(self):
        bullet = Bullet(x=100, y=200, speed=5, is_player_bullet=True)
        rect = bullet.rect
        assert rect.x == 100
        assert rect.y == 200
        assert rect.width == BULLET_WIDTH
        assert rect.height == BULLET_HEIGHT


class TestAlien:
    """Tests for Alien class."""

    def test_creation(self):
        alien = Alien(row=0, col=0, x=100, y=100)
        assert alien.row == 0
        assert alien.col == 0
        assert alien.x == 100
        assert alien.y == 100
        assert alien.alive is True

    def test_points_by_row(self):
        alien_top = Alien(row=0, col=0, x=0, y=0)
        alien_bottom = Alien(row=4, col=0, x=0, y=0)
        assert alien_top.points == POINTS_PER_ROW[0]
        assert alien_bottom.points == POINTS_PER_ROW[4]
        # Top row should be worth more
        assert alien_top.points > alien_bottom.points

    def test_rect(self):
        alien = Alien(row=0, col=0, x=100, y=100)
        rect = alien.rect
        assert rect.x == 100
        assert rect.y == 100
        assert rect.width == ALIEN_WIDTH
        assert rect.height == ALIEN_HEIGHT


class TestAlienFleet:
    """Tests for AlienFleet class."""

    def test_initialization(self):
        fleet = AlienFleet()
        assert len(fleet.aliens) == ALIEN_ROWS * ALIEN_COLS
        assert fleet.direction == 1  # Moving right initially

    def test_all_aliens_alive_initially(self):
        fleet = AlienFleet()
        assert all(alien.alive for alien in fleet.aliens)

    def test_alive_count(self):
        fleet = AlienFleet()
        assert fleet.alive_count == ALIEN_ROWS * ALIEN_COLS

        fleet.aliens[0].alive = False
        assert fleet.alive_count == ALIEN_ROWS * ALIEN_COLS - 1

    def test_all_dead(self):
        fleet = AlienFleet()
        assert fleet.all_dead is False

        for alien in fleet.aliens:
            alien.alive = False
        assert fleet.all_dead is True

    def test_reset(self):
        fleet = AlienFleet()
        fleet.aliens[0].alive = False
        fleet.direction = -1

        fleet.reset()

        assert fleet.alive_count == ALIEN_ROWS * ALIEN_COLS
        assert fleet.direction == 1

    def test_get_shooters(self):
        fleet = AlienFleet()
        shooters = fleet.get_shooters()
        # Should have one shooter per column
        assert len(shooters) == ALIEN_COLS

        # Each shooter should be the bottom-most alien in its column
        for shooter in shooters:
            column_aliens = [a for a in fleet.aliens if a.col == shooter.col and a.alive]
            bottom = max(column_aliens, key=lambda a: a.y)
            assert shooter == bottom

    def test_get_shooters_with_dead_aliens(self):
        fleet = AlienFleet()
        # Kill the bottom row
        for alien in fleet.aliens:
            if alien.row == ALIEN_ROWS - 1:
                alien.alive = False

        shooters = fleet.get_shooters()
        # Should still have one shooter per column
        assert len(shooters) == ALIEN_COLS
        # All shooters should be from row ALIEN_ROWS - 2
        for shooter in shooters:
            assert shooter.row == ALIEN_ROWS - 2


class TestShield:
    """Tests for Shield class."""

    def test_initialization(self):
        shield = Shield(x=100, y=400)
        assert shield.x == 100
        assert shield.y == 400
        assert len(shield.blocks) > 0

    def test_blocks_have_positions(self):
        shield = Shield(x=100, y=400)
        for block in shield.blocks:
            assert block.x >= 100
            assert block.y >= 400

    def test_collision_destroys_block(self):
        shield = Shield(x=100, y=400)
        # Find an alive block
        block = next(b for b in shield.blocks if b.alive)
        initial_alive = sum(1 for b in shield.blocks if b.alive)

        # Create a bullet rect that overlaps the block
        import pygame
        bullet_rect = pygame.Rect(block.x, block.y, BULLET_WIDTH, BULLET_HEIGHT)

        result = shield.check_collision(bullet_rect)

        assert result is True
        assert not block.alive
        assert sum(1 for b in shield.blocks if b.alive) == initial_alive - 1

    def test_no_collision_with_dead_block(self):
        shield = Shield(x=100, y=400)
        # Kill all blocks first, then check collision with area where a block was
        block = shield.blocks[0]
        for b in shield.blocks:
            b.alive = False

        import pygame
        bullet_rect = pygame.Rect(block.x, block.y, BULLET_WIDTH, BULLET_HEIGHT)

        result = shield.check_collision(bullet_rect)
        assert result is False

    def test_is_destroyed(self):
        shield = Shield(x=100, y=400)
        assert shield.is_destroyed is False

        for block in shield.blocks:
            block.alive = False
        assert shield.is_destroyed is True


class TestGame:
    """Tests for Game class."""

    @pytest.fixture
    def game(self):
        """Create a game instance for testing."""
        return Game(headless=True)

    def test_initialization(self, game):
        assert game.state == GameState.MENU
        assert game.score == 0
        assert game.lives == INITIAL_LIVES
        assert len(game.shields) == SHIELD_COUNT
        assert game.fleet.alive_count == ALIEN_ROWS * ALIEN_COLS

    def test_state_change_menu_to_playing(self, game):
        state_changes = []
        game.on_state_change = lambda old, new: state_changes.append((old, new))

        game.set_state(GameState.PLAYING)

        assert game.state == GameState.PLAYING
        assert len(state_changes) == 1
        assert state_changes[0] == (GameState.MENU, GameState.PLAYING)

    def test_state_change_same_state_no_callback(self, game):
        game.set_state(GameState.PLAYING)
        state_changes = []
        game.on_state_change = lambda old, new: state_changes.append((old, new))

        game.set_state(GameState.PLAYING)  # Same state

        assert len(state_changes) == 0

    def test_pause_toggle(self, game):
        game.set_state(GameState.PLAYING)
        game.set_state(GameState.PAUSED)
        assert game.state == GameState.PAUSED

        game.set_state(GameState.PLAYING)
        assert game.state == GameState.PLAYING

    def test_reset_game(self, game):
        game.set_state(GameState.PLAYING)
        game.score = 500
        game.lives = 1
        game.fleet.aliens[0].alive = False
        game.player_bullets.append(Bullet(100, 100, 5, True))

        game.reset_game()

        assert game.score == 0
        assert game.lives == INITIAL_LIVES
        assert game.fleet.alive_count == ALIEN_ROWS * ALIEN_COLS
        assert len(game.player_bullets) == 0

    def test_high_score_update_on_game_over(self, game):
        game.set_state(GameState.PLAYING)
        game.score = 100
        game.high_score = 50

        game.set_state(GameState.GAME_OVER)

        assert game.high_score == 100

    def test_high_score_not_lowered(self, game):
        game.high_score = 200
        game.set_state(GameState.PLAYING)
        game.score = 100

        game.set_state(GameState.GAME_OVER)

        assert game.high_score == 200

    def test_step_returns_state(self, game):
        game.set_state(GameState.PLAYING)
        result = game.step()

        assert "state" in result
        assert "score" in result
        assert "lives" in result
        assert "player_x" in result
        assert "aliens_remaining" in result
        assert "player_bullets" in result
        assert "alien_bullets" in result
        assert "frame_count" in result
        assert result["state"] == "PLAYING"
        assert result["aliens_remaining"] == ALIEN_ROWS * ALIEN_COLS


class TestGameplay:
    """Integration tests for gameplay mechanics."""

    @pytest.fixture
    def game(self):
        """Create a game in playing state."""
        g = Game(headless=True)
        g.set_state(GameState.PLAYING)
        return g

    def test_inject_input_left(self, game):
        initial_x = game.player.x
        game.inject_input('left')
        assert game.player.x < initial_x

    def test_inject_input_right(self, game):
        initial_x = game.player.x
        game.inject_input('right')
        assert game.player.x > initial_x

    def test_inject_input_shoot(self, game):
        assert len(game.player_bullets) == 0
        game.inject_input('shoot')
        assert len(game.player_bullets) == 1

    def test_shoot_cooldown(self, game):
        game.inject_input('shoot')
        assert len(game.player_bullets) == 1

        # Should not be able to shoot immediately
        game.inject_input('shoot')
        assert len(game.player_bullets) == 1  # Still just 1

    def test_player_bullet_moves_up(self, game):
        game.inject_input('shoot')
        initial_y = game.player_bullets[0].y

        game.update()

        assert game.player_bullets[0].y < initial_y

    def test_player_bullet_removed_off_screen(self, game):
        game.player_bullets.append(Bullet(100, -BULLET_HEIGHT - 1, PLAYER_BULLET_SPEED, True))

        game.update()

        assert len(game.player_bullets) == 0

    def test_alien_bullet_hits_player(self, game):
        lives_lost = []
        game.on_life_lost = lambda lives: lives_lost.append(lives)

        # Position alien bullet to hit player
        game.alien_bullets.append(Bullet(
            x=game.player.x + game.player.width // 2,
            y=game.player.y,
            speed=ALIEN_BULLET_SPEED,
            is_player_bullet=False
        ))
        initial_lives = game.lives

        game.update()

        assert game.lives == initial_lives - 1
        assert len(lives_lost) == 1

    def test_player_dies_game_over(self, game):
        game.lives = 1
        game.alien_bullets.append(Bullet(
            x=game.player.x + game.player.width // 2,
            y=game.player.y,
            speed=ALIEN_BULLET_SPEED,
            is_player_bullet=False
        ))

        game.update()

        assert game.state == GameState.GAME_OVER

    def test_player_bullet_destroys_alien(self, game):
        destroyed = []
        game.on_alien_destroyed = lambda alien: destroyed.append(alien)

        # Find a living alien and position bullet to hit it
        target = next(a for a in game.fleet.aliens if a.alive)
        game.player_bullets.append(Bullet(
            x=target.x + ALIEN_WIDTH // 2,
            y=target.y + ALIEN_HEIGHT,
            speed=PLAYER_BULLET_SPEED,
            is_player_bullet=True
        ))
        initial_alive = game.fleet.alive_count

        game.update()

        assert game.fleet.alive_count == initial_alive - 1
        assert len(destroyed) == 1
        assert not target.alive

    def test_score_increases_on_alien_kill(self, game):
        target = next(a for a in game.fleet.aliens if a.alive)
        expected_points = target.points
        game.player_bullets.append(Bullet(
            x=target.x + ALIEN_WIDTH // 2,
            y=target.y + ALIEN_HEIGHT,
            speed=PLAYER_BULLET_SPEED,
            is_player_bullet=True
        ))
        initial_score = game.score

        game.update()

        assert game.score == initial_score + expected_points

    def test_victory_when_all_aliens_killed(self, game):
        # Kill all but one alien
        for alien in game.fleet.aliens[:-1]:
            alien.alive = False

        # Position bullet to kill last alien
        last_alien = next(a for a in game.fleet.aliens if a.alive)
        game.player_bullets.append(Bullet(
            x=last_alien.x + ALIEN_WIDTH // 2,
            y=last_alien.y + ALIEN_HEIGHT,
            speed=PLAYER_BULLET_SPEED,
            is_player_bullet=True
        ))

        game.update()

        assert game.state == GameState.VICTORY

    def test_bullet_hits_shield(self, game):
        shield = game.shields[0]
        block = next(b for b in shield.blocks if b.alive)

        game.player_bullets.append(Bullet(
            x=block.x,
            y=block.y + 10,
            speed=PLAYER_BULLET_SPEED,
            is_player_bullet=True
        ))

        game.update()

        # Bullet should be removed
        assert len(game.player_bullets) == 0
        # Block should be destroyed
        assert not block.alive

    def test_alien_bullet_hits_shield(self, game):
        shield = game.shields[0]
        block = next(b for b in shield.blocks if b.alive)

        game.alien_bullets.append(Bullet(
            x=block.x,
            y=block.y - 10,
            speed=ALIEN_BULLET_SPEED,
            is_player_bullet=False
        ))

        game.update()

        # Bullet should be removed
        assert len(game.alien_bullets) == 0
        # Block should be destroyed
        assert not block.alive

    def test_game_runs_frames(self, game):
        result = None
        for _ in range(10):
            result = game.step()

        assert result["frame_count"] == 10
        assert game.running


class TestGameStateTransitions:
    """Test game state machine transitions."""

    @pytest.fixture
    def game(self):
        return Game(headless=True)

    def test_menu_to_playing_resets_game(self, game):
        game.score = 100
        game.set_state(GameState.PLAYING)
        assert game.score == 0

    def test_game_over_to_playing_resets_game(self, game):
        game.set_state(GameState.PLAYING)
        game.score = 100
        game.lives = 0
        game.set_state(GameState.GAME_OVER)

        game.set_state(GameState.PLAYING)

        assert game.score == 0
        assert game.lives == INITIAL_LIVES

    def test_victory_to_playing_resets_game(self, game):
        game.set_state(GameState.PLAYING)
        game.score = 1000
        for alien in game.fleet.aliens:
            alien.alive = False
        game.set_state(GameState.VICTORY)

        game.set_state(GameState.PLAYING)

        assert game.score == 0
        assert game.fleet.alive_count == ALIEN_ROWS * ALIEN_COLS


class TestGameImports:
    """Test that the game module imports correctly."""

    def test_imports(self):
        from main import Game, GameState, Player, Bullet, Alien, AlienFleet, Shield, Vector2
        assert Game is not None
        assert GameState is not None
        assert Player is not None
        assert Bullet is not None
        assert Alien is not None
        assert AlienFleet is not None
        assert Shield is not None
        assert Vector2 is not None

    def test_constants_exist(self):
        from main import (
            SCREEN_WIDTH, SCREEN_HEIGHT, FPS, PLAYER_WIDTH, PLAYER_HEIGHT,
            ALIEN_ROWS, ALIEN_COLS, INITIAL_LIVES, SHIELD_COUNT
        )
        assert all(isinstance(c, int) for c in [
            SCREEN_WIDTH, SCREEN_HEIGHT, FPS, PLAYER_WIDTH, PLAYER_HEIGHT,
            ALIEN_ROWS, ALIEN_COLS, INITIAL_LIVES, SHIELD_COUNT
        ])
