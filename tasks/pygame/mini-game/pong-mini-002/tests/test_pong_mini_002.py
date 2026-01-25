"""
Tests for Add local network multiplayer.
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

from main import Game, GameState


class TestBasicImports:
    """Test that game can be imported."""

    def test_game_imports(self):
        assert Game is not None

    def test_game_creates(self):
        game = Game(headless=True)
        assert game is not None


# TODO: Add specific tests for Add local network multiplayer
# Tests should verify the requirements in prompt.md


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
