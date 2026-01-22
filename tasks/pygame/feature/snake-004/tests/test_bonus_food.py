"""Tests for the bonus food feature."""

import os
import sys
import pytest

# Set up headless mode
os.environ["SDL_VIDEODRIVER"] = "dummy"
os.environ["SDL_AUDIODRIVER"] = "dummy"

# Add game directory to path
game_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(game_dir, "game"))

from main import Game, GameState, Position


class TestBonusFoodFeature:
    """Tests for bonus food functionality."""

    def test_bonus_food_class_exists(self):
        """BonusFood class should exist."""
        from main import BonusFood
        assert BonusFood is not None

    def test_bonus_food_has_required_attributes(self):
        """BonusFood should have required attributes."""
        from main import BonusFood
        bonus = BonusFood()

        assert hasattr(bonus, 'position'), "BonusFood should have position"
        assert hasattr(bonus, 'active'), "BonusFood should have active"
        assert hasattr(bonus, 'timer'), "BonusFood should have timer"
        assert hasattr(bonus, 'points'), "BonusFood should have points"

    def test_bonus_food_starts_inactive(self):
        """BonusFood should start inactive."""
        from main import BonusFood
        bonus = BonusFood()
        assert bonus.active is False, "BonusFood should start inactive"

    def test_bonus_food_has_update_method(self):
        """BonusFood should have update method."""
        from main import BonusFood
        bonus = BonusFood()
        assert hasattr(bonus, 'update'), "BonusFood should have update method"

    def test_bonus_food_deactivates_when_timer_expires(self):
        """BonusFood should deactivate when timer reaches 0."""
        from main import BonusFood
        bonus = BonusFood()
        bonus.active = True
        bonus.timer = 1

        bonus.update()

        assert bonus.active is False, "BonusFood should deactivate when timer expires"

    def test_bonus_food_check_eaten(self):
        """BonusFood should detect when eaten by snake."""
        from main import BonusFood, Snake
        bonus = BonusFood()
        snake = Snake(Position(10, 10))

        bonus.active = True
        bonus.position = Position(11, 10)  # One cell to the right

        # Move snake to bonus food
        snake.move()

        assert bonus.check_eaten(snake), "BonusFood should detect when eaten"

    def test_game_has_bonus_food(self):
        """Game should have bonus_food attribute."""
        game = Game(headless=True)
        assert hasattr(game, 'bonus_food'), "Game should have bonus_food"

    def test_eating_bonus_food_gives_extra_points(self):
        """Eating bonus food should give extra points."""
        from main import BonusFood
        bonus = BonusFood()
        assert bonus.points > 10, "Bonus food should give more than regular food (10 points)"


class TestGameImports:
    """Basic tests to verify the game module works."""

    def test_game_imports(self):
        """Test that the game module can be imported."""
        from main import Game, GameState
        assert Game is not None
