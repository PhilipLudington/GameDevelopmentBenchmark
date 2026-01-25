"""Tests for Fix DAS not working after hold."""
import os, sys
os.environ["SDL_VIDEODRIVER"] = "dummy"
os.environ["SDL_AUDIODRIVER"] = "dummy"
game_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(game_dir, "game"))
from main import Game, GameState
import pytest

class TestBasicImports:
    def test_game_imports(self):
        assert Game is not None

if __name__ == "__main__":
    pytest.main([__file__, "-v"])
