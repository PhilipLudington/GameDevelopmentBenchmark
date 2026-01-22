#!/usr/bin/env python3
"""Snake - A classic arcade game implemented in Pygame.

This is a baseline game for the Game Development Benchmark.
It includes proper structure for testing and bug injection.
"""

import argparse
import os
import random
import sys
from enum import Enum, auto
from dataclasses import dataclass
from typing import Callable, List, Tuple, Optional

# Set up headless mode if requested before importing pygame
if "--headless" in sys.argv or os.environ.get("SDL_VIDEODRIVER") == "dummy":
    os.environ.setdefault("SDL_VIDEODRIVER", "dummy")
    os.environ.setdefault("SDL_AUDIODRIVER", "dummy")

import pygame


# Constants
SCREEN_WIDTH = 640
SCREEN_HEIGHT = 480
CELL_SIZE = 20
GRID_WIDTH = SCREEN_WIDTH // CELL_SIZE
GRID_HEIGHT = SCREEN_HEIGHT // CELL_SIZE
FPS = 10  # Snake games typically run at lower FPS

# Colors
BLACK = (0, 0, 0)
WHITE = (255, 255, 255)
GREEN = (0, 255, 0)
DARK_GREEN = (0, 200, 0)
RED = (255, 0, 0)
GRAY = (128, 128, 128)

# Game settings
INITIAL_SNAKE_LENGTH = 3
SCORE_PER_FOOD = 10


class GameState(Enum):
    """Game state enumeration."""
    MENU = auto()
    PLAYING = auto()
    PAUSED = auto()
    GAME_OVER = auto()


class Direction(Enum):
    """Direction enumeration for snake movement."""
    UP = (0, -1)
    DOWN = (0, 1)
    LEFT = (-1, 0)
    RIGHT = (1, 0)

    @property
    def opposite(self) -> "Direction":
        """Get the opposite direction."""
        opposites = {
            Direction.UP: Direction.DOWN,
            Direction.DOWN: Direction.UP,
            Direction.LEFT: Direction.RIGHT,
            Direction.RIGHT: Direction.LEFT,
        }
        return opposites[self]


@dataclass
class Position:
    """Grid position."""
    x: int
    y: int

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, Position):
            return NotImplemented
        return self.x == other.x and self.y == other.y

    def __hash__(self) -> int:
        return hash((self.x, self.y))

    def __add__(self, other: "Position") -> "Position":
        return Position(self.x + other.x, self.y + other.y)

    def to_tuple(self) -> Tuple[int, int]:
        return (self.x, self.y)


class Snake:
    """Snake entity."""

    def __init__(self, start_pos: Position):
        """Initialize the snake.

        Args:
            start_pos: Starting head position
        """
        self.body: List[Position] = []
        self.direction = Direction.RIGHT
        self.next_direction = Direction.RIGHT
        self.grow_pending = 0

        # Initialize snake body
        for i in range(INITIAL_SNAKE_LENGTH):
            self.body.append(Position(start_pos.x - i, start_pos.y))

    @property
    def head(self) -> Position:
        """Get the snake's head position."""
        return self.body[0]

    @property
    def tail(self) -> List[Position]:
        """Get the snake's tail (body minus head)."""
        return self.body[1:]

    def change_direction(self, new_direction: Direction) -> None:
        """Change the snake's direction if valid.

        Args:
            new_direction: The new direction to move in
        """
        # Prevent reversing direction (would cause instant self-collision)
        # Check against next_direction to handle rapid key presses
        if new_direction != self.next_direction.opposite:
            self.next_direction = new_direction

    def move(self) -> None:
        """Move the snake one cell in the current direction."""
        # Apply the pending direction change
        self.direction = self.next_direction

        # Calculate new head position
        dx, dy = self.direction.value
        new_head = Position(self.head.x + dx, self.head.y + dy)

        # Insert new head at the front
        self.body.insert(0, new_head)

        # Remove tail unless growing
        if self.grow_pending > 0:
            self.grow_pending -= 1
        else:
            self.body.pop()

    def grow(self, amount: int = 1) -> None:
        """Schedule the snake to grow.

        Args:
            amount: Number of segments to add
        """
        self.grow_pending += amount

    def check_wall_collision(self) -> bool:
        """Check if the snake has hit a wall.

        Returns:
            True if collision detected
        """
        head = self.head
        return (
            head.x < 0 or
            head.x >= GRID_WIDTH or
            head.y < 0 or
            head.y >= GRID_HEIGHT
        )

    def check_self_collision(self) -> bool:
        """Check if the snake has hit itself.

        Returns:
            True if collision detected
        """
        return self.head in self.tail

    def check_collision(self, position: Position) -> bool:
        """Check if a position collides with any part of the snake.

        Args:
            position: Position to check

        Returns:
            True if collision detected
        """
        return position in self.body

    def reset(self, start_pos: Position) -> None:
        """Reset the snake to its initial state.

        Args:
            start_pos: Starting head position
        """
        self.body = []
        self.direction = Direction.RIGHT
        self.next_direction = Direction.RIGHT
        self.grow_pending = 0

        for i in range(INITIAL_SNAKE_LENGTH):
            self.body.append(Position(start_pos.x - i, start_pos.y))

    def draw(self, surface: pygame.Surface) -> None:
        """Draw the snake.

        Args:
            surface: Surface to draw on
        """
        for i, segment in enumerate(self.body):
            color = DARK_GREEN if i == 0 else GREEN
            rect = pygame.Rect(
                segment.x * CELL_SIZE,
                segment.y * CELL_SIZE,
                CELL_SIZE - 1,
                CELL_SIZE - 1
            )
            pygame.draw.rect(surface, color, rect)


class Food:
    """Food entity."""

    def __init__(self):
        """Initialize food at a random position."""
        self.position = Position(0, 0)

    def spawn(self, snake: Snake) -> None:
        """Spawn food at a random position not occupied by the snake.

        Args:
            snake: The snake to avoid
        """
        while True:
            x = random.randint(0, GRID_WIDTH - 1)
            y = random.randint(0, GRID_HEIGHT - 1)
            self.position = Position(x, y)

            # Make sure food doesn't spawn on snake
            if not snake.check_collision(self.position):
                break

    def check_eaten(self, snake: Snake) -> bool:
        """Check if the food has been eaten.

        Args:
            snake: The snake to check

        Returns:
            True if snake head is at food position
        """
        return snake.head == self.position

    def draw(self, surface: pygame.Surface) -> None:
        """Draw the food.

        Args:
            surface: Surface to draw on
        """
        rect = pygame.Rect(
            self.position.x * CELL_SIZE,
            self.position.y * CELL_SIZE,
            CELL_SIZE - 1,
            CELL_SIZE - 1
        )
        pygame.draw.rect(surface, RED, rect)


class Game:
    """Main game class."""

    def __init__(self, headless: bool = False):
        """Initialize the game.

        Args:
            headless: Run without display
        """
        pygame.init()

        self.headless = headless
        self.screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))
        pygame.display.set_caption("Snake")

        self.clock = pygame.time.Clock()
        self.font = pygame.font.Font(None, 74)
        self.small_font = pygame.font.Font(None, 36)

        self.state = GameState.MENU
        self.running = True
        self.frame_count = 0

        # Create game entities
        start_pos = Position(GRID_WIDTH // 2, GRID_HEIGHT // 2)
        self.snake = Snake(start_pos)
        self.food = Food()
        self.food.spawn(self.snake)

        self.score = 0
        self.high_score = 0

        # Callbacks for testing
        self.on_score: Optional[Callable[[int], None]] = None
        self.on_state_change: Optional[Callable[[GameState], None]] = None
        self.on_death: Optional[Callable[[str], None]] = None

    def reset_game(self) -> None:
        """Reset the game to initial state."""
        start_pos = Position(GRID_WIDTH // 2, GRID_HEIGHT // 2)
        self.snake.reset(start_pos)
        self.food.spawn(self.snake)
        self.score = 0

    def handle_menu_input(self, event: pygame.event.Event) -> None:
        """Handle input in menu state.

        Args:
            event: Pygame event
        """
        if event.type == pygame.KEYDOWN:
            if event.key == pygame.K_SPACE:
                self.reset_game()
                self.set_state(GameState.PLAYING)
            elif event.key == pygame.K_ESCAPE:
                self.running = False

    def handle_playing_input(self, event: pygame.event.Event) -> None:
        """Handle input in playing state.

        Args:
            event: Pygame event
        """
        if event.type == pygame.KEYDOWN:
            if event.key == pygame.K_ESCAPE:
                self.set_state(GameState.PAUSED)
            elif event.key == pygame.K_p:
                self.set_state(GameState.PAUSED)
            elif event.key in (pygame.K_UP, pygame.K_w):
                self.snake.change_direction(Direction.UP)
            elif event.key in (pygame.K_DOWN, pygame.K_s):
                self.snake.change_direction(Direction.DOWN)
            elif event.key in (pygame.K_LEFT, pygame.K_a):
                self.snake.change_direction(Direction.LEFT)
            elif event.key in (pygame.K_RIGHT, pygame.K_d):
                self.snake.change_direction(Direction.RIGHT)

    def handle_paused_input(self, event: pygame.event.Event) -> None:
        """Handle input in paused state.

        Args:
            event: Pygame event
        """
        if event.type == pygame.KEYDOWN:
            if event.key == pygame.K_SPACE or event.key == pygame.K_p:
                self.set_state(GameState.PLAYING)
            elif event.key == pygame.K_ESCAPE:
                self.set_state(GameState.MENU)

    def handle_game_over_input(self, event: pygame.event.Event) -> None:
        """Handle input in game over state.

        Args:
            event: Pygame event

        BUG: Pressing ESC resets the high score when it shouldn't.
        """
        if event.type == pygame.KEYDOWN:
            if event.key == pygame.K_SPACE:
                self.reset_game()
                self.set_state(GameState.PLAYING)
            elif event.key == pygame.K_ESCAPE:
                # BUG: This incorrectly resets the high score
                self.high_score = 0
                self.set_state(GameState.MENU)

    def handle_events(self) -> None:
        """Handle pygame events."""
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                self.running = False
                return

            if self.state == GameState.MENU:
                self.handle_menu_input(event)
            elif self.state == GameState.PLAYING:
                self.handle_playing_input(event)
            elif self.state == GameState.PAUSED:
                self.handle_paused_input(event)
            elif self.state == GameState.GAME_OVER:
                self.handle_game_over_input(event)

    def update(self) -> None:
        """Update game state."""
        if self.state != GameState.PLAYING:
            return

        # Move snake
        self.snake.move()

        # Check for food collision
        if self.food.check_eaten(self.snake):
            self.snake.grow()
            self.score += SCORE_PER_FOOD
            self.food.spawn(self.snake)

            if self.on_score:
                self.on_score(self.score)

        # Check for death conditions
        death_reason = None
        if self.snake.check_wall_collision():
            death_reason = "wall"
        elif self.snake.check_self_collision():
            death_reason = "self"

        if death_reason:
            if self.score > self.high_score:
                self.high_score = self.score

            if self.on_death:
                self.on_death(death_reason)

            self.set_state(GameState.GAME_OVER)

    def set_state(self, new_state: GameState) -> None:
        """Set the game state.

        Args:
            new_state: New state to set
        """
        self.state = new_state
        if self.on_state_change:
            self.on_state_change(new_state)

    def draw_grid(self) -> None:
        """Draw the game grid."""
        for x in range(0, SCREEN_WIDTH, CELL_SIZE):
            pygame.draw.line(self.screen, GRAY, (x, 0), (x, SCREEN_HEIGHT), 1)
        for y in range(0, SCREEN_HEIGHT, CELL_SIZE):
            pygame.draw.line(self.screen, GRAY, (0, y), (SCREEN_WIDTH, y), 1)

    def draw_score(self) -> None:
        """Draw the current score."""
        score_text = self.small_font.render(f"Score: {self.score}", True, WHITE)
        self.screen.blit(score_text, (10, 10))

        high_score_text = self.small_font.render(f"High Score: {self.high_score}", True, WHITE)
        self.screen.blit(high_score_text, (SCREEN_WIDTH - high_score_text.get_width() - 10, 10))

    def draw_menu(self) -> None:
        """Draw the menu screen."""
        title = self.font.render("SNAKE", True, GREEN)
        instruction = self.small_font.render("Press SPACE to start", True, GRAY)
        controls = self.small_font.render("Arrow keys or WASD to move", True, GRAY)

        self.screen.blit(title, (SCREEN_WIDTH // 2 - title.get_width() // 2, SCREEN_HEIGHT // 3))
        self.screen.blit(instruction, (SCREEN_WIDTH // 2 - instruction.get_width() // 2, SCREEN_HEIGHT // 2))
        self.screen.blit(controls, (SCREEN_WIDTH // 2 - controls.get_width() // 2, SCREEN_HEIGHT // 2 + 40))

    def draw_paused(self) -> None:
        """Draw the paused overlay."""
        overlay = pygame.Surface((SCREEN_WIDTH, SCREEN_HEIGHT))
        overlay.fill(BLACK)
        overlay.set_alpha(128)
        self.screen.blit(overlay, (0, 0))

        paused_text = self.font.render("PAUSED", True, WHITE)
        instruction = self.small_font.render("Press SPACE to resume", True, GRAY)

        self.screen.blit(paused_text, (SCREEN_WIDTH // 2 - paused_text.get_width() // 2, SCREEN_HEIGHT // 3))
        self.screen.blit(instruction, (SCREEN_WIDTH // 2 - instruction.get_width() // 2, SCREEN_HEIGHT // 2))

    def draw_game_over(self) -> None:
        """Draw the game over screen."""
        game_over_text = self.font.render("GAME OVER", True, RED)
        score_text = self.small_font.render(f"Final Score: {self.score}", True, WHITE)
        instruction = self.small_font.render("Press SPACE to play again", True, GRAY)

        self.screen.blit(game_over_text, (SCREEN_WIDTH // 2 - game_over_text.get_width() // 2, SCREEN_HEIGHT // 3))
        self.screen.blit(score_text, (SCREEN_WIDTH // 2 - score_text.get_width() // 2, SCREEN_HEIGHT // 2))
        self.screen.blit(instruction, (SCREEN_WIDTH // 2 - instruction.get_width() // 2, SCREEN_HEIGHT // 2 + 40))

    def draw(self) -> None:
        """Draw the game."""
        self.screen.fill(BLACK)

        if self.state == GameState.MENU:
            self.draw_menu()
        elif self.state == GameState.PLAYING:
            self.draw_grid()
            self.draw_score()
            self.snake.draw(self.screen)
            self.food.draw(self.screen)
        elif self.state == GameState.PAUSED:
            self.draw_grid()
            self.draw_score()
            self.snake.draw(self.screen)
            self.food.draw(self.screen)
            self.draw_paused()
        elif self.state == GameState.GAME_OVER:
            self.draw_game_over()

        pygame.display.flip()

    def run(self, max_frames: Optional[int] = None) -> None:
        """Run the game loop.

        Args:
            max_frames: Maximum frames to run (for headless testing)
        """
        while self.running:
            self.handle_events()
            self.update()
            self.draw()

            self.frame_count += 1
            self.clock.tick(FPS)

            if max_frames and self.frame_count >= max_frames:
                break

        pygame.quit()

    def step(self) -> dict:
        """Execute a single game step.

        Returns:
            Current game state information
        """
        self.handle_events()
        self.update()
        self.draw()
        self.frame_count += 1
        self.clock.tick(FPS)

        return {
            "state": self.state.name,
            "score": self.score,
            "snake_length": len(self.snake.body),
            "snake_head": self.snake.head.to_tuple(),
            "snake_direction": self.snake.direction.name,
            "food_pos": self.food.position.to_tuple(),
            "frame": self.frame_count,
        }

    def inject_input(self, direction: Direction) -> None:
        """Inject a direction input (for testing/bots).

        Args:
            direction: Direction to set
        """
        self.snake.change_direction(direction)


def main():
    """Main entry point."""
    parser = argparse.ArgumentParser(description="Snake - Classic arcade game")
    parser.add_argument("--headless", action="store_true", help="Run in headless mode")
    parser.add_argument("--frames", type=int, help="Maximum frames to run (headless only)")
    args = parser.parse_args()

    game = Game(headless=args.headless)

    if args.headless and args.frames:
        game.set_state(GameState.PLAYING)
        game.run(max_frames=args.frames)
    else:
        game.run()


if __name__ == "__main__":
    main()
