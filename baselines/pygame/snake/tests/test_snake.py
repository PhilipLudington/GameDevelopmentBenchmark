"""Unit tests for the Snake game."""

import os
import sys
import pytest

# Set up headless mode
os.environ["SDL_VIDEODRIVER"] = "dummy"
os.environ["SDL_AUDIODRIVER"] = "dummy"

# Add parent directory to path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from main import (
    Snake, Food, Game, GameState, Direction, Position,
    GRID_WIDTH, GRID_HEIGHT, INITIAL_SNAKE_LENGTH, SCORE_PER_FOOD
)


class TestPosition:
    """Tests for the Position class."""

    def test_position_equality(self):
        """Positions with same coordinates should be equal."""
        p1 = Position(5, 10)
        p2 = Position(5, 10)
        assert p1 == p2

    def test_position_inequality(self):
        """Positions with different coordinates should not be equal."""
        p1 = Position(5, 10)
        p2 = Position(5, 11)
        assert p1 != p2

    def test_position_addition(self):
        """Position addition should work correctly."""
        p1 = Position(3, 4)
        p2 = Position(1, 2)
        result = p1 + p2
        assert result.x == 4
        assert result.y == 6

    def test_position_hashable(self):
        """Positions should be hashable for use in sets/dicts."""
        p1 = Position(5, 10)
        p2 = Position(5, 10)
        s = {p1}
        assert p2 in s


class TestDirection:
    """Tests for the Direction enum."""

    def test_opposite_directions(self):
        """Opposite directions should be correct."""
        assert Direction.UP.opposite == Direction.DOWN
        assert Direction.DOWN.opposite == Direction.UP
        assert Direction.LEFT.opposite == Direction.RIGHT
        assert Direction.RIGHT.opposite == Direction.LEFT

    def test_direction_values(self):
        """Direction values should be correct dx, dy tuples."""
        assert Direction.UP.value == (0, -1)
        assert Direction.DOWN.value == (0, 1)
        assert Direction.LEFT.value == (-1, 0)
        assert Direction.RIGHT.value == (1, 0)


class TestSnake:
    """Tests for the Snake class."""

    def test_snake_initialization(self):
        """Snake should initialize with correct length and position."""
        start = Position(10, 10)
        snake = Snake(start)

        assert len(snake.body) == INITIAL_SNAKE_LENGTH
        assert snake.head == start
        assert snake.direction == Direction.RIGHT

    def test_snake_head_property(self):
        """Snake head should be the first body segment."""
        snake = Snake(Position(10, 10))
        assert snake.head == snake.body[0]

    def test_snake_tail_property(self):
        """Snake tail should be all segments except head."""
        snake = Snake(Position(10, 10))
        assert snake.tail == snake.body[1:]

    def test_snake_move_right(self):
        """Snake should move right correctly."""
        snake = Snake(Position(10, 10))
        initial_head = Position(snake.head.x, snake.head.y)
        snake.move()

        assert snake.head.x == initial_head.x + 1
        assert snake.head.y == initial_head.y

    def test_snake_move_up(self):
        """Snake should move up correctly."""
        snake = Snake(Position(10, 10))
        snake.change_direction(Direction.UP)
        initial_head = Position(snake.head.x, snake.head.y)
        snake.move()

        assert snake.head.x == initial_head.x
        assert snake.head.y == initial_head.y - 1

    def test_snake_move_down(self):
        """Snake should move down correctly."""
        snake = Snake(Position(10, 10))
        snake.change_direction(Direction.DOWN)
        initial_head = Position(snake.head.x, snake.head.y)
        snake.move()

        assert snake.head.x == initial_head.x
        assert snake.head.y == initial_head.y + 1

    def test_snake_move_left(self):
        """Snake should move left correctly."""
        snake = Snake(Position(10, 10))
        # First change to UP to allow LEFT
        snake.change_direction(Direction.UP)
        snake.move()
        snake.change_direction(Direction.LEFT)
        initial_head = Position(snake.head.x, snake.head.y)
        snake.move()

        assert snake.head.x == initial_head.x - 1
        assert snake.head.y == initial_head.y

    def test_snake_cannot_reverse(self):
        """Snake should not be able to reverse direction."""
        snake = Snake(Position(10, 10))
        assert snake.direction == Direction.RIGHT

        # Try to reverse (should be ignored)
        snake.change_direction(Direction.LEFT)
        snake.move()

        # Should still be moving right
        assert snake.direction == Direction.RIGHT

    def test_snake_grow(self):
        """Snake should grow when eating food."""
        snake = Snake(Position(10, 10))
        initial_length = len(snake.body)

        snake.grow(1)
        snake.move()

        assert len(snake.body) == initial_length + 1

    def test_snake_grow_multiple(self):
        """Snake should grow by multiple segments."""
        snake = Snake(Position(10, 10))
        initial_length = len(snake.body)

        snake.grow(3)
        for _ in range(3):
            snake.move()

        assert len(snake.body) == initial_length + 3

    def test_snake_wall_collision_left(self):
        """Snake should detect left wall collision."""
        snake = Snake(Position(0, 10))
        snake.change_direction(Direction.UP)
        snake.move()
        snake.change_direction(Direction.LEFT)
        snake.move()

        assert snake.check_wall_collision()

    def test_snake_wall_collision_right(self):
        """Snake should detect right wall collision."""
        snake = Snake(Position(GRID_WIDTH - 1, 10))
        snake.move()

        assert snake.check_wall_collision()

    def test_snake_wall_collision_top(self):
        """Snake should detect top wall collision."""
        snake = Snake(Position(10, 0))
        snake.change_direction(Direction.UP)
        snake.move()

        assert snake.check_wall_collision()

    def test_snake_wall_collision_bottom(self):
        """Snake should detect bottom wall collision."""
        snake = Snake(Position(10, GRID_HEIGHT - 1))
        snake.change_direction(Direction.DOWN)
        snake.move()

        assert snake.check_wall_collision()

    def test_snake_no_wall_collision(self):
        """Snake should not detect collision when inside grid."""
        snake = Snake(Position(10, 10))
        snake.move()

        assert not snake.check_wall_collision()

    def test_snake_self_collision(self):
        """Snake should detect self collision."""
        snake = Snake(Position(10, 10))

        # Grow snake long enough to collide with itself
        snake.grow(5)
        for _ in range(5):
            snake.move()

        # Create a loop pattern: right, up, left, down
        snake.change_direction(Direction.UP)
        snake.move()
        snake.change_direction(Direction.LEFT)
        snake.move()
        snake.change_direction(Direction.DOWN)
        snake.move()

        assert snake.check_self_collision()

    def test_snake_no_self_collision(self):
        """Snake should not detect self collision when not colliding."""
        snake = Snake(Position(10, 10))
        snake.move()

        assert not snake.check_self_collision()

    def test_snake_check_collision(self):
        """Snake should detect collision with a position."""
        snake = Snake(Position(10, 10))

        # Head position
        assert snake.check_collision(Position(10, 10))
        # Body position
        assert snake.check_collision(Position(9, 10))
        # Empty position
        assert not snake.check_collision(Position(20, 20))

    def test_snake_reset(self):
        """Snake should reset to initial state."""
        snake = Snake(Position(10, 10))
        snake.grow(5)
        for _ in range(5):
            snake.move()
        snake.change_direction(Direction.UP)
        snake.move()

        snake.reset(Position(15, 15))

        assert snake.head == Position(15, 15)
        assert len(snake.body) == INITIAL_SNAKE_LENGTH
        assert snake.direction == Direction.RIGHT


class TestFood:
    """Tests for the Food class."""

    def test_food_initialization(self):
        """Food should initialize with a position."""
        food = Food()
        assert hasattr(food, 'position')

    def test_food_spawn_not_on_snake(self):
        """Food should not spawn on the snake."""
        snake = Snake(Position(10, 10))
        food = Food()

        for _ in range(100):  # Test multiple spawns
            food.spawn(snake)
            assert not snake.check_collision(food.position)

    def test_food_spawn_within_grid(self):
        """Food should spawn within grid boundaries."""
        snake = Snake(Position(10, 10))
        food = Food()

        for _ in range(100):  # Test multiple spawns
            food.spawn(snake)
            assert 0 <= food.position.x < GRID_WIDTH
            assert 0 <= food.position.y < GRID_HEIGHT

    def test_food_check_eaten(self):
        """Food should detect when eaten by snake."""
        snake = Snake(Position(10, 10))
        food = Food()
        food.position = Position(11, 10)  # One cell to the right

        # Move snake to food
        snake.move()

        assert food.check_eaten(snake)

    def test_food_not_eaten(self):
        """Food should not be eaten when snake is elsewhere."""
        snake = Snake(Position(10, 10))
        food = Food()
        food.position = Position(20, 20)

        assert not food.check_eaten(snake)


class TestGame:
    """Tests for the Game class."""

    def test_game_initialization(self):
        """Game should initialize correctly."""
        game = Game(headless=True)

        assert game.state == GameState.MENU
        assert game.running is True
        assert game.score == 0
        assert game.snake is not None
        assert game.food is not None

    def test_game_reset(self):
        """Game should reset correctly."""
        game = Game(headless=True)
        game.score = 100
        game.snake.grow(5)

        game.reset_game()

        assert game.score == 0
        assert len(game.snake.body) == INITIAL_SNAKE_LENGTH

    def test_game_state_transitions(self):
        """Game should transition between states correctly."""
        game = Game(headless=True)

        assert game.state == GameState.MENU

        game.set_state(GameState.PLAYING)
        assert game.state == GameState.PLAYING

        game.set_state(GameState.PAUSED)
        assert game.state == GameState.PAUSED

        game.set_state(GameState.GAME_OVER)
        assert game.state == GameState.GAME_OVER

    def test_game_step_returns_info(self):
        """Game step should return state information."""
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)

        info = game.step()

        assert "state" in info
        assert "score" in info
        assert "snake_length" in info
        assert "snake_head" in info
        assert "food_pos" in info

    def test_game_inject_input(self):
        """Game should accept injected input."""
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)

        game.inject_input(Direction.UP)

        # Direction change takes effect on next move
        assert game.snake.next_direction == Direction.UP

    def test_game_eating_food_increases_score(self):
        """Eating food should increase score."""
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)

        # Position food directly in front of snake
        game.food.position = Position(game.snake.head.x + 1, game.snake.head.y)
        initial_score = game.score

        game.update()

        assert game.score == initial_score + SCORE_PER_FOOD

    def test_game_wall_collision_ends_game(self):
        """Wall collision should end the game."""
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)

        # Move snake to right edge
        game.snake.body[0] = Position(GRID_WIDTH - 1, 10)

        game.update()

        assert game.state == GameState.GAME_OVER

    def test_game_self_collision_ends_game(self):
        """Self collision should end the game."""
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)

        # Create a snake that's about to collide with itself
        game.snake.body = [
            Position(10, 10),  # head
            Position(11, 10),
            Position(11, 11),
            Position(10, 11),
            Position(9, 11),
        ]
        game.snake.direction = Direction.DOWN
        game.snake.next_direction = Direction.DOWN

        game.update()

        assert game.state == GameState.GAME_OVER

    def test_game_high_score_updated(self):
        """High score should be updated on game over."""
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game.score = 100
        game.high_score = 50

        # Trigger game over
        game.snake.body[0] = Position(GRID_WIDTH - 1, 10)
        game.update()

        assert game.high_score == 100

    def test_game_callbacks_called(self):
        """Game callbacks should be called at appropriate times."""
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)

        score_called = []
        state_called = []
        death_called = []

        game.on_score = lambda s: score_called.append(s)
        game.on_state_change = lambda s: state_called.append(s)
        game.on_death = lambda r: death_called.append(r)

        # Trigger score
        game.food.position = Position(game.snake.head.x + 1, game.snake.head.y)
        game.update()
        assert len(score_called) == 1

        # Trigger death
        game.snake.body[0] = Position(GRID_WIDTH - 1, 10)
        game.update()
        assert len(death_called) == 1
        assert death_called[0] == "wall"


class TestGameImports:
    """Basic tests to verify the game module works."""

    def test_game_imports(self):
        """Test that the game module can be imported."""
        from main import Game, GameState, Snake, Food
        assert Game is not None
        assert GameState is not None

    def test_snake_creation(self):
        """Test snake can be created."""
        snake = Snake(Position(10, 10))
        assert snake.head is not None
        assert len(snake.body) > 0

    def test_food_creation(self):
        """Test food can be created."""
        food = Food()
        assert food.position is not None
