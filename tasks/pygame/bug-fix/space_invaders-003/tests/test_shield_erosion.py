#!/usr/bin/env python3
"""Test suite for space_invaders-003: shield collision symmetry bug."""

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
import pygame
from main import (
    Game, GameState, Shield, Bullet,
    BULLET_WIDTH, BULLET_HEIGHT, PLAYER_BULLET_SPEED, ALIEN_BULLET_SPEED
)


class TestShieldErosionDirection:
    """Tests for shield erosion based on bullet direction."""

    @pytest.fixture
    def game(self):
        g = Game(headless=True)
        g.set_state(GameState.PLAYING)
        return g

    def get_alive_blocks_by_y(self, shield):
        """Get alive blocks sorted by y position."""
        alive = [b for b in shield.blocks if b.alive]
        return sorted(alive, key=lambda b: b.y)

    def test_player_bullet_destroys_bottom_block(self, game):
        """Player bullet (moving up) should destroy bottom-most overlapping block."""
        shield = game.shields[0]

        # Get a column of blocks
        alive_blocks = self.get_alive_blocks_by_y(shield)
        if not alive_blocks:
            pytest.skip("No blocks available")

        # Find the bottom-most block (highest y value)
        bottom_block = max(alive_blocks, key=lambda b: b.y)
        bottom_y = bottom_block.y

        # Fire player bullet at this column
        game.player_bullets.append(Bullet(
            x=bottom_block.x,
            y=bottom_block.y + 20,  # Below the block
            speed=PLAYER_BULLET_SPEED,
            is_player_bullet=True
        ))

        game.update()

        # The bottom block should be destroyed
        assert not bottom_block.alive, "Player bullet should destroy bottom-most block"

    def test_alien_bullet_destroys_top_block(self, game):
        """Alien bullet (moving down) should destroy top-most overlapping block."""
        shield = game.shields[0]

        alive_blocks = self.get_alive_blocks_by_y(shield)
        if not alive_blocks:
            pytest.skip("No blocks available")

        # Find the top-most block (lowest y value)
        top_block = min(alive_blocks, key=lambda b: b.y)

        # Fire alien bullet from above
        game.alien_bullets.append(Bullet(
            x=top_block.x,
            y=top_block.y - 20,  # Above the block
            speed=ALIEN_BULLET_SPEED,
            is_player_bullet=False
        ))

        game.update()

        # The top block should be destroyed
        assert not top_block.alive, "Alien bullet should destroy top-most block"

    def test_erosion_pattern_from_above(self, game):
        """Multiple alien bullets should create top-down erosion pattern."""
        shield = game.shields[0]

        # Find a vertical column of blocks
        x_positions = set(b.x for b in shield.blocks if b.alive)
        if not x_positions:
            pytest.skip("No blocks available")

        target_x = min(x_positions)
        column_blocks = sorted(
            [b for b in shield.blocks if b.alive and b.x == target_x],
            key=lambda b: b.y
        )

        if len(column_blocks) < 2:
            pytest.skip("Not enough blocks in column")

        # Fire first alien bullet
        game.alien_bullets.append(Bullet(
            x=target_x,
            y=column_blocks[0].y - 20,
            speed=ALIEN_BULLET_SPEED,
            is_player_bullet=False
        ))
        game.update()

        # Top block should be destroyed
        assert not column_blocks[0].alive, "First bullet should destroy top block"

        # Fire second alien bullet
        game.alien_bullets.append(Bullet(
            x=target_x,
            y=column_blocks[1].y - 20,
            speed=ALIEN_BULLET_SPEED,
            is_player_bullet=False
        ))
        game.update()

        # Second-from-top should be destroyed
        assert not column_blocks[1].alive, "Second bullet should destroy next top block"

    def test_erosion_pattern_from_below(self, game):
        """Multiple player bullets should create bottom-up erosion pattern."""
        shield = game.shields[0]

        x_positions = set(b.x for b in shield.blocks if b.alive)
        if not x_positions:
            pytest.skip("No blocks available")

        target_x = min(x_positions)
        column_blocks = sorted(
            [b for b in shield.blocks if b.alive and b.x == target_x],
            key=lambda b: b.y,
            reverse=True  # Bottom first
        )

        if len(column_blocks) < 2:
            pytest.skip("Not enough blocks in column")

        # Fire first player bullet
        game.player_bullets.append(Bullet(
            x=target_x,
            y=column_blocks[0].y + 20,
            speed=PLAYER_BULLET_SPEED,
            is_player_bullet=True
        ))
        game.update()

        # Bottom block should be destroyed
        assert not column_blocks[0].alive, "First bullet should destroy bottom block"

        # Fire second player bullet
        game.player_bullets.append(Bullet(
            x=target_x,
            y=column_blocks[1].y + 20,
            speed=PLAYER_BULLET_SPEED,
            is_player_bullet=True
        ))
        game.update()

        # Second-from-bottom should be destroyed
        assert not column_blocks[1].alive, "Second bullet should destroy next bottom block"
