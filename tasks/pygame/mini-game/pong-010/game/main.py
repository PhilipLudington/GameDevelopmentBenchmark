#!/usr/bin/env python3
"""Pong - A classic arcade game implemented in Pygame.

This is a baseline game for the Game Development Benchmark.
It includes proper structure for testing and bug injection.
"""

import argparse
import os
import sys
from enum import Enum, auto
from dataclasses import dataclass
from typing import Callable

# Set up headless mode if requested before importing pygame
if "--headless" in sys.argv or os.environ.get("SDL_VIDEODRIVER") == "dummy":
    os.environ.setdefault("SDL_VIDEODRIVER", "dummy")
    os.environ.setdefault("SDL_AUDIODRIVER", "dummy")

import pygame


# Constants
SCREEN_WIDTH = 800
SCREEN_HEIGHT = 600
FPS = 60

# Colors
BLACK = (0, 0, 0)
WHITE = (255, 255, 255)
GRAY = (128, 128, 128)

# Game settings
PADDLE_WIDTH = 15
PADDLE_HEIGHT = 90
PADDLE_SPEED = 7
BALL_SIZE = 15
BALL_SPEED = 5
WINNING_SCORE = 11


class GameState(Enum):
    """Game state enumeration."""
    MENU = auto()
    PLAYING = auto()
    PAUSED = auto()
    GAME_OVER = auto()


@dataclass
class Vector2:
    """Simple 2D vector."""
    x: float
    y: float

    def __add__(self, other: "Vector2") -> "Vector2":
        return Vector2(self.x + other.x, self.y + other.y)

    def __mul__(self, scalar: float) -> "Vector2":
        return Vector2(self.x * scalar, self.y * scalar)


class Paddle:
    """Paddle entity."""

    def __init__(self, x: int, y: int, is_ai: bool = False):
        """Initialize a paddle.

        Args:
            x: Initial x position
            y: Initial y position
            is_ai: Whether this paddle is AI-controlled
        """
        self.rect = pygame.Rect(x, y, PADDLE_WIDTH, PADDLE_HEIGHT)
        self.speed = PADDLE_SPEED
        self.is_ai = is_ai
        self.score = 0

    def move_up(self) -> None:
        """Move paddle up."""
        self.rect.y = max(0, self.rect.y - self.speed)

    def move_down(self) -> None:
        """Move paddle down."""
        self.rect.y = min(SCREEN_HEIGHT - PADDLE_HEIGHT, self.rect.y + self.speed)

    def update_ai(self, ball: "Ball") -> None:
        """Update AI paddle to track the ball.

        Args:
            ball: The ball to track
        """
        if not self.is_ai:
            return

        # Simple AI: move towards ball's y position
        paddle_center = self.rect.centery
        ball_center = ball.rect.centery

        # Add some delay/imperfection to make AI beatable
        if ball_center < paddle_center - 10:
            self.move_up()
        elif ball_center > paddle_center + 10:
            self.move_down()

    def reset(self) -> None:
        """Reset paddle to starting position."""
        self.rect.centery = SCREEN_HEIGHT // 2

    def draw(self, surface: pygame.Surface) -> None:
        """Draw the paddle.

        Args:
            surface: Surface to draw on
        """
        pygame.draw.rect(surface, WHITE, self.rect)


class Ball:
    """Ball entity."""

    def __init__(self):
        """Initialize the ball."""
        self.rect = pygame.Rect(
            SCREEN_WIDTH // 2 - BALL_SIZE // 2,
            SCREEN_HEIGHT // 2 - BALL_SIZE // 2,
            BALL_SIZE,
            BALL_SIZE,
        )
        self.velocity = Vector2(BALL_SPEED, BALL_SPEED)
        self.speed = BALL_SPEED
        self.speed_increment = 0.2  # Speed increase per hit

    def reset(self, direction: int = 1) -> None:
        """Reset ball to center.

        Args:
            direction: Initial x direction (1 or -1)
        """
        self.rect.center = (SCREEN_WIDTH // 2, SCREEN_HEIGHT // 2)
        self.velocity = Vector2(self.speed * direction, self.speed)
        self.speed = BALL_SPEED

    def update(self) -> int:
        """Update ball position and handle wall collisions.

        Returns:
            -1 if ball went off left side (player 2 scores)
            1 if ball went off right side (player 1 scores)
            0 otherwise
        """
        # Move ball
        self.rect.x += int(self.velocity.x)
        self.rect.y += int(self.velocity.y)

        # Top/bottom wall collision
        if self.rect.top <= 0:
            self.rect.top = 0
            self.velocity.y = abs(self.velocity.y)
        elif self.rect.bottom >= SCREEN_HEIGHT:
            self.rect.bottom = SCREEN_HEIGHT
            self.velocity.y = -abs(self.velocity.y)

        # Check if ball went off screen (scoring)
        if self.rect.right < 0:
            return 1  # Player 2 scores
        elif self.rect.left > SCREEN_WIDTH:
            return -1  # Player 1 scores

        return 0

    def check_paddle_collision(self, paddle: Paddle) -> bool:
        """Check and handle collision with a paddle.

        Args:
            paddle: Paddle to check collision with

        Returns:
            True if collision occurred
        """
        if self.rect.colliderect(paddle.rect):
            # Determine which side of paddle was hit
            if self.velocity.x > 0:  # Ball moving right
                self.rect.right = paddle.rect.left
                self.velocity.x = -abs(self.velocity.x)
            else:  # Ball moving left
                self.rect.left = paddle.rect.right
                self.velocity.x = abs(self.velocity.x)

            # Adjust angle based on where ball hit paddle
            relative_y = (self.rect.centery - paddle.rect.centery) / (PADDLE_HEIGHT / 2)
            self.velocity.y = relative_y * self.speed

            # Increase speed slightly
            self.speed += self.speed_increment
            speed_ratio = self.speed / BALL_SPEED
            self.velocity.x = self.velocity.x / abs(self.velocity.x) * self.speed if self.velocity.x != 0 else self.speed

            return True

        return False

    def draw(self, surface: pygame.Surface) -> None:
        """Draw the ball.

        Args:
            surface: Surface to draw on
        """
        pygame.draw.rect(surface, WHITE, self.rect)


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
        pygame.display.set_caption("Pong")

        self.clock = pygame.time.Clock()
        self.font = pygame.font.Font(None, 74)
        self.small_font = pygame.font.Font(None, 36)

        self.state = GameState.MENU
        self.running = True
        self.frame_count = 0

        # Create game objects
        self.player1 = Paddle(50, SCREEN_HEIGHT // 2 - PADDLE_HEIGHT // 2)
        self.player2 = Paddle(
            SCREEN_WIDTH - 50 - PADDLE_WIDTH,
            SCREEN_HEIGHT // 2 - PADDLE_HEIGHT // 2,
            is_ai=True,
        )
        self.ball = Ball()

        # Callbacks for testing/automation
        self.on_score: Callable[[int, int], None] | None = None
        self.on_state_change: Callable[[GameState], None] | None = None

    def reset_game(self) -> None:
        """Reset game state for a new game."""
        self.player1.score = 0
        self.player2.score = 0
        self.player1.reset()
        self.player2.reset()
        self.ball.reset()

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
        """Handle input while playing.

        Args:
            event: Pygame event
        """
        if event.type == pygame.KEYDOWN:
            if event.key == pygame.K_ESCAPE:
                self.set_state(GameState.PAUSED)
            elif event.key == pygame.K_p:
                self.set_state(GameState.PAUSED)

    def handle_paused_input(self, event: pygame.event.Event) -> None:
        """Handle input while paused.

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
        """
        if event.type == pygame.KEYDOWN:
            if event.key == pygame.K_SPACE:
                self.reset_game()
                self.set_state(GameState.PLAYING)
            elif event.key == pygame.K_ESCAPE:
                self.set_state(GameState.MENU)

    def handle_events(self) -> None:
        """Handle all pygame events."""
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

    def handle_continuous_input(self) -> None:
        """Handle continuous key presses for movement."""
        if self.state != GameState.PLAYING:
            return

        keys = pygame.key.get_pressed()

        # Player 1 controls (W/S or Up/Down)
        if keys[pygame.K_w] or keys[pygame.K_UP]:
            self.player1.move_up()
        if keys[pygame.K_s] or keys[pygame.K_DOWN]:
            self.player1.move_down()

    def update(self) -> None:
        """Update game state."""
        if self.state != GameState.PLAYING:
            return

        # Update AI
        self.player2.update_ai(self.ball)

        # Update ball and check for scoring
        score_result = self.ball.update()

        if score_result != 0:
            if score_result == 1:
                self.player2.score += 1
            else:
                self.player1.score += 1

            if self.on_score:
                self.on_score(self.player1.score, self.player2.score)

            # Check for game over
            if self.player1.score >= WINNING_SCORE or self.player2.score >= WINNING_SCORE:
                self.set_state(GameState.GAME_OVER)
            else:
                # Reset ball, serve towards scorer
                self.ball.reset(direction=score_result)

        # Check paddle collisions
        self.ball.check_paddle_collision(self.player1)
        self.ball.check_paddle_collision(self.player2)

    def set_state(self, new_state: GameState) -> None:
        """Set the game state.

        Args:
            new_state: New state to transition to
        """
        old_state = self.state
        self.state = new_state
        if self.on_state_change:
            self.on_state_change(new_state)

    def draw_center_line(self) -> None:
        """Draw the center dashed line."""
        for y in range(0, SCREEN_HEIGHT, 30):
            pygame.draw.rect(self.screen, GRAY, (SCREEN_WIDTH // 2 - 2, y, 4, 15))

    def draw_scores(self) -> None:
        """Draw the score display."""
        score1_text = self.font.render(str(self.player1.score), True, WHITE)
        score2_text = self.font.render(str(self.player2.score), True, WHITE)

        self.screen.blit(score1_text, (SCREEN_WIDTH // 4 - score1_text.get_width() // 2, 30))
        self.screen.blit(score2_text, (3 * SCREEN_WIDTH // 4 - score2_text.get_width() // 2, 30))

    def draw_menu(self) -> None:
        """Draw the menu screen."""
        title = self.font.render("PONG", True, WHITE)
        instruction = self.small_font.render("Press SPACE to start", True, GRAY)

        self.screen.blit(title, (SCREEN_WIDTH // 2 - title.get_width() // 2, SCREEN_HEIGHT // 3))
        self.screen.blit(instruction, (SCREEN_WIDTH // 2 - instruction.get_width() // 2, SCREEN_HEIGHT // 2))

    def draw_paused(self) -> None:
        """Draw the paused overlay."""
        # Semi-transparent overlay
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
        winner = "Player 1" if self.player1.score >= WINNING_SCORE else "Player 2"
        winner_text = self.font.render(f"{winner} Wins!", True, WHITE)
        instruction = self.small_font.render("Press SPACE to play again", True, GRAY)

        self.screen.blit(winner_text, (SCREEN_WIDTH // 2 - winner_text.get_width() // 2, SCREEN_HEIGHT // 3))
        self.screen.blit(instruction, (SCREEN_WIDTH // 2 - instruction.get_width() // 2, SCREEN_HEIGHT // 2))

    def draw(self) -> None:
        """Draw the current frame."""
        self.screen.fill(BLACK)

        if self.state == GameState.MENU:
            self.draw_menu()
        elif self.state == GameState.PLAYING:
            self.draw_center_line()
            self.draw_scores()
            self.player1.draw(self.screen)
            self.player2.draw(self.screen)
            self.ball.draw(self.screen)
        elif self.state == GameState.PAUSED:
            # Draw game state behind pause overlay
            self.draw_center_line()
            self.draw_scores()
            self.player1.draw(self.screen)
            self.player2.draw(self.screen)
            self.ball.draw(self.screen)
            self.draw_paused()
        elif self.state == GameState.GAME_OVER:
            self.draw_scores()
            self.draw_game_over()

        pygame.display.flip()

    def run(self, max_frames: int | None = None) -> None:
        """Run the game loop.

        Args:
            max_frames: Maximum frames to run (for headless testing)
        """
        while self.running:
            self.handle_events()
            self.handle_continuous_input()
            self.update()
            self.draw()

            self.frame_count += 1
            self.clock.tick(FPS)

            if max_frames and self.frame_count >= max_frames:
                break

        pygame.quit()

    def step(self) -> dict:
        """Execute one frame (for testing/automation).

        Returns:
            Current game state as dictionary
        """
        self.handle_events()
        self.handle_continuous_input()
        self.update()
        self.draw()
        self.frame_count += 1
        self.clock.tick(FPS)

        return {
            "state": self.state.name,
            "player1_score": self.player1.score,
            "player2_score": self.player2.score,
            "ball_pos": (self.ball.rect.centerx, self.ball.rect.centery),
            "player1_pos": self.player1.rect.centery,
            "player2_pos": self.player2.rect.centery,
            "frame": self.frame_count,
        }


def main():
    """Entry point."""
    parser = argparse.ArgumentParser(description="Pong - Classic arcade game")
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
