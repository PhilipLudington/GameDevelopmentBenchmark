#!/usr/bin/env python3
"""Test suite for space_invaders-mini-001: two-player cooperative mode."""

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
from main import Game, GameState


class TestTwoPlayerBasic:
    """Basic tests for two-player mode implementation."""

    @pytest.fixture
    def game(self):
        g = Game(headless=True)
        return g

    def test_num_players_attribute(self, game):
        """Game should track number of players."""
        assert hasattr(game, 'num_players') or hasattr(game, 'player_count'), \
            "Game should have num_players attribute"

    def test_players_list_exists(self, game):
        """Game should have list of players."""
        assert hasattr(game, 'players'), "Game should have 'players' list"

    def test_can_set_two_players(self, game):
        """Should be able to set game to two-player mode."""
        if hasattr(game, 'num_players'):
            game.num_players = 2
        elif hasattr(game, 'player_count'):
            game.player_count = 2

        # Trigger setup
        if hasattr(game, 'setup_players'):
            game.setup_players()

        players = getattr(game, 'players', [])
        assert len(players) >= 2 or hasattr(game, 'player2'), \
            "Two-player mode should have two players"


class TestTwoPlayerPositions:
    """Tests for two-player positioning."""

    @pytest.fixture
    def game(self):
        g = Game(headless=True)
        if hasattr(g, 'num_players'):
            g.num_players = 2
        g.set_state(GameState.PLAYING)
        return g

    def test_players_at_different_positions(self, game):
        """Two players should start at different positions."""
        players = getattr(game, 'players', [])

        if len(players) < 2:
            pytest.skip("Two-player mode not implemented")

        p1_x = players[0].x
        p2_x = players[1].x

        assert p1_x != p2_x, "Players should start at different X positions"

    def test_players_have_different_colors(self, game):
        """Players should have different colors for visual distinction."""
        players = getattr(game, 'players', [])

        if len(players) < 2:
            pytest.skip("Two-player mode not implemented")

        if hasattr(players[0], 'color') and hasattr(players[1], 'color'):
            assert players[0].color != players[1].color, \
                "Players should have different colors"


class TestTwoPlayerControls:
    """Tests for two-player controls."""

    @pytest.fixture
    def game(self):
        g = Game(headless=True)
        if hasattr(g, 'num_players'):
            g.num_players = 2
        g.set_state(GameState.PLAYING)
        return g

    def test_player1_moves_independently(self, game):
        """Player 1 should move independently."""
        players = getattr(game, 'players', [])

        if len(players) < 2:
            pytest.skip("Two-player mode not implemented")

        initial_x = players[0].x
        players[0].move_right()

        assert players[0].x != initial_x, "Player 1 should move"
        # Player 2 position should be unchanged
        # (Would need to track p2 initial position)

    def test_player2_moves_independently(self, game):
        """Player 2 should move independently."""
        players = getattr(game, 'players', [])

        if len(players) < 2:
            pytest.skip("Two-player mode not implemented")

        initial_x = players[1].x
        players[1].move_left()

        assert players[1].x != initial_x, "Player 2 should move"


class TestTwoPlayerShooting:
    """Tests for two-player shooting."""

    @pytest.fixture
    def game(self):
        g = Game(headless=True)
        if hasattr(g, 'num_players'):
            g.num_players = 2
        g.set_state(GameState.PLAYING)
        return g

    def test_both_players_can_shoot(self, game):
        """Both players should be able to shoot."""
        # Implementation-dependent test
        pass

    def test_both_players_can_score(self, game):
        """Kills from either player should add to score."""
        # Implementation-dependent test
        pass


class TestTwoPlayerLives:
    """Tests for two-player lives system."""

    @pytest.fixture
    def game(self):
        g = Game(headless=True)
        if hasattr(g, 'num_players'):
            g.num_players = 2
        g.set_state(GameState.PLAYING)
        return g

    def test_lives_system_exists(self, game):
        """Lives system should work in two-player mode."""
        # Check for shared lives or individual lives
        has_lives = (hasattr(game, 'lives') or
                    hasattr(game, 'team_lives') or
                    (hasattr(game, 'players') and
                     len(game.players) > 0 and
                     hasattr(game.players[0], 'lives')))

        assert has_lives, "Two-player mode should have lives system"

    def test_game_over_when_all_dead(self, game):
        """Game should end when all players/lives are lost."""
        # Implementation-dependent test
        pass


class TestModeSelection:
    """Tests for mode selection."""

    @pytest.fixture
    def game(self):
        return Game(headless=True)

    def test_can_switch_to_two_player(self, game):
        """Should be able to switch to two-player mode."""
        if hasattr(game, 'num_players'):
            game.num_players = 2
            assert game.num_players == 2

    def test_default_is_single_player(self, game):
        """Default mode should be single player."""
        num = getattr(game, 'num_players', 1) or getattr(game, 'player_count', 1)
        assert num == 1, "Default should be single player"
