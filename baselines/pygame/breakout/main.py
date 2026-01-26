#!/usr/bin/env python3
"""Breakout - Classic brick-breaking arcade game.

A baseline implementation for the Game Development Benchmark.
Features:
- Paddle controlled by left/right arrow keys or A/D
- Ball physics with angle-based reflections
- Multiple rows of colored bricks
- Scoring system
- Lives system
- Game states (menu, playing, paused, game over, victory)
- Headless mode support for automated testing
- Sound effects for game events
"""

import argparse
import os
import sys
from dataclasses import dataclass
from enum import Enum
from typing import Callable, Optional

# Set up headless mode BEFORE importing pygame
if "--headless" in sys.argv or os.environ.get("SDL_VIDEODRIVER") == "dummy":
    os.environ.setdefault("SDL_VIDEODRIVER", "dummy")
    os.environ.setdefault("SDL_AUDIODRIVER", "dummy")

import pygame

# Screen dimensions
SCREEN_WIDTH = 800
SCREEN_HEIGHT = 600
FPS = 60

# Paddle settings
PADDLE_WIDTH = 100
PADDLE_HEIGHT = 15
PADDLE_SPEED = 8
PADDLE_Y = SCREEN_HEIGHT - 50

# Ball settings
BALL_RADIUS = 8
BALL_SPEED = 5
BALL_START_Y = SCREEN_HEIGHT - 100

# Brick settings
BRICK_ROWS = 5
BRICK_COLS = 10
BRICK_WIDTH = 70
BRICK_HEIGHT = 25
BRICK_PADDING = 5
BRICK_TOP_OFFSET = 80
BRICK_LEFT_OFFSET = (SCREEN_WIDTH - (BRICK_COLS * (BRICK_WIDTH + BRICK_PADDING) - BRICK_PADDING)) // 2

# Game settings
INITIAL_LIVES = 3
POINTS_PER_BRICK = 10

# Colors
WHITE = (255, 255, 255)
BLACK = (0, 0, 0)
GRAY = (128, 128, 128)
RED = (255, 0, 0)
ORANGE = (255, 165, 0)
YELLOW = (255, 255, 0)
GREEN = (0, 255, 0)
BLUE = (0, 100, 255)

# Brick colors by row (from top to bottom)
BRICK_COLORS = [RED, ORANGE, YELLOW, GREEN, BLUE]


def create_beep_sound(frequency: float, duration_ms: int) -> Optional[pygame.mixer.Sound]:
    """Create a simple beep sound using sine wave synthesis.
    
    Args:
        frequency: Frequency in Hz
        duration_ms: Duration in milliseconds
        
    Returns:
        pygame.mixer.Sound object or None if synthesis fails
    """
    try:
        import numpy as np
        
        sample_rate = 44100
        n_samples = int(sample_rate * duration_ms / 1000)
        t = np.linspace(0, duration_ms / 1000, n_samples, False)
        
        # Generate sine wave
        wave = np.sin(2 * np.pi * frequency * t) * 0.3
        wave = (wave * 32767).astype(np.int16)
        
        # Create stereo sound
        stereo = np.column_stack([wave, wave])
        sound = pygame.sndarray.make_sound(stereo)
        return sound
    except (ImportError, ValueError, TypeError):
        return None


def create_sound_library() -> dict:
    """Create a library of sounds for game events.
    
    Returns:
        Dictionary mapping event names to pygame.mixer.Sound objects.
        Returns empty dict if sound creation fails.
    """
    sounds = {}
    
    # Paddle hit: high pitch beep
    paddle_sound = create_beep_sound(800, 100)
    if paddle_sound:
        sounds['paddle'] = paddle_sound
    
    # Brick hit: mid-high pitch beep
    brick_sound = create_beep_sound(600, 80)
    if brick_sound:
        sounds['brick'] = brick_sound
    
    # Wall hit: lower pitch beep
    wall_sound = create_beep_sound(400, 60)
    if wall_sound:
        sounds['wall'] = wall_sound
    
    # Life lost: sad falling tone
    life_lost_sound = create_beep_sound(300, 200)
    if life_lost_sound:
        sounds['life_lost'] = life_lost_sound
    
    # Game over: lower sad tone
    game_over_sound = create_beep_sound(200, 300)
    if game_over_sound:
        sounds['game_over'] = game_over_sound
    
    # Victory: high ascending tone
    victory_sound = create_beep_sound(1000, 300)
    if victory_sound:
        sounds['victory'] = victory_sound
    
    return sounds


class GameState(Enum):
    """Game state machine states."""
    MENU = "menu"
    PLAYING = "playing"
    PAUSED = "paused"
    COUNTDOWN = "countdown"
    GAME_OVER = "game_over"
    VICTORY = "victory"


@dataclass
class Vector2:
    """2D vector for positions and velocities."""
    x: float
    y: float

    def __add__(self, other: "Vector2") -> "Vector2":
        return Vector2(self.x + other.x, self.y + other.y)

    def __sub__(self, other: "Vector2") -> "Vector2":
        return Vector2(self.x - other.x, self.y - other.y)

    def __mul__(self, scalar: float) -> "Vector2":
        return Vector2(self.x * scalar, self.y * scalar)

    def copy(self) -> "Vector2":
        return Vector2(self.x, self.y)


class Paddle:
    """Player-controlled paddle at the bottom of the screen."""

    def __init__(self):
        self.width = PADDLE_WIDTH
        self.height = PADDLE_HEIGHT
        self.speed = PADDLE_SPEED
        self.reset()

    def reset(self):
        """Reset paddle to center position."""
        self.x = (SCREEN_WIDTH - self.width) // 2
        self.y = PADDLE_Y

    def move_left(self):
        """Move paddle left, clamped to screen bounds."""
        self.x = max(0, self.x - self.speed)

    def move_right(self):
        """Move paddle right, clamped to screen bounds."""
        self.x = min(SCREEN_WIDTH - self.width, self.x + self.speed)

    @property
    def rect(self) -> pygame.Rect:
        """Get paddle rectangle for collision detection."""
        return pygame.Rect(self.x, self.y, self.width, self.height)

    @property
    def center_x(self) -> float:
        """Get center x position of paddle."""
        return self.x + self.width / 2


class Ball:
    """Ball that bounces around the screen."""

    def __init__(self):
        self.radius = BALL_RADIUS
        self.speed = BALL_SPEED
        self.reset()

    def reset(self):
        """Reset ball to starting position above paddle."""
        self.position = Vector2(SCREEN_WIDTH // 2, BALL_START_Y)
        # Start with a slight angle downward
        self.velocity = Vector2(self.speed * 0.7, self.speed * 0.7)

    @property
    def rect(self) -> pygame.Rect:
        """Get ball bounding rectangle for collision detection."""
        return pygame.Rect(
            self.position.x - self.radius,
            self.position.y - self.radius,
            self.radius * 2,
            self.radius * 2
        )

    def update(self):
        """Update ball position based on velocity."""
        self.position = self.position + self.velocity

    def bounce_horizontal(self):
        """Reverse horizontal direction."""
        self.velocity.x = -self.velocity.x

    def bounce_vertical(self):
        """Reverse vertical direction."""
        self.velocity.y = -self.velocity.y

    def bounce_off_paddle(self, paddle: Paddle):
        """Bounce off paddle with angle based on hit position."""
        # Calculate relative hit position (-1 to 1)
        relative_x = (self.position.x - paddle.center_x) / (paddle.width / 2)
        relative_x = max(-1, min(1, relative_x))  # Clamp to [-1, 1]

        # Calculate new angle based on hit position
        # Hit center: go straight up, hit edges: go at angle
        import math
        max_angle = math.pi / 3  # 60 degrees max
        angle = relative_x * max_angle

        # Set new velocity (always going up after paddle hit)
        self.velocity.x = self.speed * math.sin(angle)
        self.velocity.y = -self.speed * math.cos(angle)


@dataclass
class Brick:
    """A single brick that can be destroyed."""
    row: int
    col: int
    color: tuple
    alive: bool = True

    @property
    def x(self) -> int:
        """Calculate x position based on column."""
        return BRICK_LEFT_OFFSET + self.col * (BRICK_WIDTH + BRICK_PADDING)

    @property
    def y(self) -> int:
        """Calculate y position based on row."""
        return BRICK_TOP_OFFSET + self.row * (BRICK_HEIGHT + BRICK_PADDING)

    @property
    def rect(self) -> pygame.Rect:
        """Get brick rectangle for collision detection."""
        return pygame.Rect(self.x, self.y, BRICK_WIDTH, BRICK_HEIGHT)

    @property
    def points(self) -> int:
        """Points awarded for destroying this brick (higher rows = more points)."""
        return POINTS_PER_BRICK * (BRICK_ROWS - self.row)


class Game:
    """Main game class managing all game logic and rendering."""

    def __init__(self, headless: bool = False):
        """Initialize the game.

        Args:
            headless: If True, run without display for testing.
        """
        self.headless = headless
        pygame.init()

        if headless:
            self.screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))
        else:
            self.screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))
            pygame.display.set_caption("Breakout")

        self.clock = pygame.time.Clock()
        self.font = pygame.font.Font(None, 36)
        self.large_font = pygame.font.Font(None, 72)
        self.countdown_font = pygame.font.Font(None, 150)

        # Initialize sound system (only in non-headless mode)
        self.sounds = {}
        if not headless:
            try:
                pygame.mixer.init()
                self.sounds = create_sound_library()
            except pygame.error:
                # Sound initialization failed, continue without sound
                pass

        # Game objects
        self.paddle = Paddle()
        self.ball = Ball()
        self.bricks: list[Brick] = []

        # Game state
        self.state = GameState.MENU
        self.score = 0
        self.high_score = 0
        self.lives = INITIAL_LIVES
        self.frame_count = 0
        self.running = True

        # Countdown state
        self.countdown_value = 3
        self.countdown_timer = 0

        # Callbacks for testing
        self.on_score: Optional[Callable[[int], None]] = None
        self.on_state_change: Optional[Callable[[GameState, GameState], None]] = None
        self.on_brick_destroyed: Optional[Callable[[Brick], None]] = None
        self.on_life_lost: Optional[Callable[[int], None]] = None

        self._create_bricks()

    def _play_sound(self, sound_name: str):
        """Play a sound effect by name.
        
        Args:
            sound_name: Name of the sound to play.
        """
        if sound_name in self.sounds and not self.headless:
            try:
                self.sounds[sound_name].play()
            except pygame.error:
                # Sound playback failed, continue silently
                pass

    def _create_bricks(self):
        """Create the initial grid of bricks."""
        self.bricks = []
        for row in range(BRICK_ROWS):
            color = BRICK_COLORS[row % len(BRICK_COLORS)]
            for col in range(BRICK_COLS):
                self.bricks.append(Brick(row=row, col=col, color=color))

    def reset_game(self):
        """Reset game to initial state for a new game."""
        self.paddle.reset()
        self.ball.reset()
        self._create_bricks()
        self.score = 0
        self.lives = INITIAL_LIVES

    def reset_ball(self):
        """Reset ball position after losing a life."""
        self.ball.reset()
        self.paddle.reset()

    def set_state(self, new_state: GameState):
        """Transition to a new game state.

        Args:
            new_state: The state to transition to.
        """
        old_state = self.state
        if old_state == new_state:
            return

        self.state = new_state

        if self.on_state_change:
            self.on_state_change(old_state, new_state)

        # Handle state entry actions
        if new_state == GameState.PLAYING:
            if old_state == GameState.MENU or old_state == GameState.GAME_OVER or old_state == GameState.VICTORY:
                self.reset_game()
        elif new_state == GameState.COUNTDOWN:
            self.countdown_value = 3
            self.countdown_timer = 0
        elif new_state == GameState.GAME_OVER:
            if self.score > self.high_score:
                self.high_score = self.score
            self._play_sound('game_over')
        elif new_state == GameState.VICTORY:
            self._play_sound('victory')

    def handle_events(self):
        """Process pygame events."""
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                self.running = False
                return

            if event.type == pygame.KEYDOWN:
                if event.key == pygame.K_ESCAPE:
                    if self.state == GameState.PLAYING:
                        self.set_state(GameState.PAUSED)
                    elif self.state == GameState.PAUSED:
                        self.set_state(GameState.COUNTDOWN)
                    elif self.state in (GameState.MENU, GameState.GAME_OVER, GameState.VICTORY):
                        self.running = False

                elif event.key == pygame.K_SPACE or event.key == pygame.K_RETURN:
                    if self.state == GameState.MENU:
                        self.set_state(GameState.PLAYING)
                    elif self.state == GameState.PAUSED:
                        self.set_state(GameState.COUNTDOWN)
                    elif self.state == GameState.GAME_OVER:
                        self.set_state(GameState.PLAYING)
                    elif self.state == GameState.VICTORY:
                        self.set_state(GameState.PLAYING)

                elif event.key == pygame.K_p:
                    if self.state == GameState.PLAYING:
                        self.set_state(GameState.PAUSED)
                    elif self.state == GameState.PAUSED:
                        self.set_state(GameState.COUNTDOWN)

    def handle_continuous_input(self):
        """Handle continuous key presses for paddle movement."""
        if self.state != GameState.PLAYING:
            return

        keys = pygame.key.get_pressed()
        if keys[pygame.K_LEFT] or keys[pygame.K_a]:
            self.paddle.move_left()
        if keys[pygame.K_RIGHT] or keys[pygame.K_d]:
            self.paddle.move_right()

    def update(self):
        """Update game logic for one frame."""
        # Handle countdown state
        if self.state == GameState.COUNTDOWN:
            self.countdown_timer += 1
            if self.countdown_timer >= FPS:
                self.countdown_timer = 0
                self.countdown_value -= 1
                if self.countdown_value < 0:
                    self.set_state(GameState.PLAYING)
            return

        if self.state != GameState.PLAYING:
            return

        self.ball.update()

        # Wall collisions
        if self.ball.position.x - self.ball.radius <= 0:
            self.ball.position.x = self.ball.radius
            self.ball.bounce_horizontal()
            self._play_sound('wall')
        elif self.ball.position.x + self.ball.radius >= SCREEN_WIDTH:
            self.ball.position.x = SCREEN_WIDTH - self.ball.radius
            self.ball.bounce_horizontal()
            self._play_sound('wall')

        if self.ball.position.y - self.ball.radius <= 0:
            self.ball.position.y = self.ball.radius
            self.ball.bounce_vertical()
            self._play_sound('wall')

        # Ball fell below paddle
        if self.ball.position.y + self.ball.radius >= SCREEN_HEIGHT:
            self.lives -= 1
            if self.on_life_lost:
                self.on_life_lost(self.lives)

            self._play_sound('life_lost')

            if self.lives <= 0:
                self.set_state(GameState.GAME_OVER)
            else:
                self.reset_ball()
            return

        # Paddle collision
        if self.ball.rect.colliderect(self.paddle.rect):
            # Only bounce if ball is moving downward
            if self.ball.velocity.y > 0:
                self.ball.position.y = self.paddle.y - self.ball.radius
                self.ball.bounce_off_paddle(self.paddle)
                self._play_sound('paddle')

        # Brick collisions
        for brick in self.bricks:
            if not brick.alive:
                continue

            if self.ball.rect.colliderect(brick.rect):
                brick.alive = False

                # Determine bounce direction based on collision side
                ball_center_x = self.ball.position.x
                ball_center_y = self.ball.position.y
                brick_center_x = brick.x + BRICK_WIDTH / 2
                brick_center_y = brick.y + BRICK_HEIGHT / 2

                # Calculate overlap on each axis
                dx = ball_center_x - brick_center_x
                dy = ball_center_y - brick_center_y

                # Determine if hit was more horizontal or vertical
                if abs(dx) / BRICK_WIDTH > abs(dy) / BRICK_HEIGHT:
                    self.ball.bounce_horizontal()
                else:
                    self.ball.bounce_vertical()

                self._play_sound('brick')

                # Update score
                old_score = self.score
                self.score += brick.points
                if self.on_score:
                    self.on_score(self.score - old_score)
                if self.on_brick_destroyed:
                    self.on_brick_destroyed(brick)

                # Only destroy one brick per frame
                break

        # Check for victory
        if all(not brick.alive for brick in self.bricks):
            self.set_state(GameState.VICTORY)

    def draw(self):
        """Render the current game state."""
        self.screen.fill(BLACK)

        if self.state == GameState.MENU:
            self._draw_menu()
        elif self.state == GameState.PLAYING:
            self._draw_game()
        elif self.state == GameState.PAUSED:
            self._draw_game()
            self._draw_paused_overlay()
        elif self.state == GameState.COUNTDOWN:
            self._draw_game()
            self._draw_countdown_overlay()
        elif self.state == GameState.GAME_OVER:
            self._draw_game()
            self._draw_game_over_overlay()
        elif self.state == GameState.VICTORY:
            self._draw_game()
            self._draw_victory_overlay()

        pygame.display.flip()

    def _draw_menu(self):
        """Draw the main menu screen."""
        title = self.large_font.render("BREAKOUT", True, WHITE)
        title_rect = title.get_rect(center=(SCREEN_WIDTH // 2, SCREEN_HEIGHT // 3))
        self.screen.blit(title, title_rect)

        instructions = [
            "Press SPACE or ENTER to start",
            "Arrow keys or A/D to move paddle",
            "P or ESC to pause",
            "",
            f"High Score: {self.high_score}"
        ]

        for i, text in enumerate(instructions):
            line = self.font.render(text, True, GRAY if i == 3 else WHITE)
            line_rect = line.get_rect(center=(SCREEN_WIDTH // 2, SCREEN_HEIGHT // 2 + i * 40))
            self.screen.blit(line, line_rect)

    def _draw_game(self):
        """Draw the main game elements."""
        # Draw paddle
        pygame.draw.rect(self.screen, WHITE, self.paddle.rect)

        # Draw ball
        pygame.draw.circle(
            self.screen,
            WHITE,
            (int(self.ball.position.x), int(self.ball.position.y)),
            self.ball.radius
        )

        # Draw bricks
        for brick in self.bricks:
            if brick.alive:
                pygame.draw.rect(self.screen, brick.color, brick.rect)
                # Draw brick border
                pygame.draw.rect(self.screen, WHITE, brick.rect, 1)

        # Draw HUD
        score_text = self.font.render(f"Score: {self.score}", True, WHITE)
        self.screen.blit(score_text, (10, 10))

        lives_text = self.font.render(f"Lives: {self.lives}", True, WHITE)
        lives_rect = lives_text.get_rect(topright=(SCREEN_WIDTH - 10, 10))
        self.screen.blit(lives_text, lives_rect)

        high_score_text = self.font.render(f"High: {self.high_score}", True, GRAY)
        high_score_rect = high_score_text.get_rect(center=(SCREEN_WIDTH // 2, 20))
        self.screen.blit(high_score_text, high_score_rect)

    def _draw_paused_overlay(self):
        """Draw the pause overlay."""
        overlay = pygame.Surface((SCREEN_WIDTH, SCREEN_HEIGHT), pygame.SRCALPHA)
        overlay.fill((0, 0, 0, 128))
        self.screen.blit(overlay, (0, 0))

        paused_text = self.large_font.render("PAUSED", True, WHITE)
        paused_rect = paused_text.get_rect(center=(SCREEN_WIDTH // 2, SCREEN_HEIGHT // 2))
        self.screen.blit(paused_text, paused_rect)

        resume_text = self.font.render("Press SPACE or P to resume", True, GRAY)
        resume_rect = resume_text.get_rect(center=(SCREEN_WIDTH // 2, SCREEN_HEIGHT // 2 + 50))
        self.screen.blit(resume_text, resume_rect)

    def _draw_countdown_overlay(self):
        """Draw the countdown overlay."""
        overlay = pygame.Surface((SCREEN_WIDTH, SCREEN_HEIGHT), pygame.SRCALPHA)
        overlay.fill((0, 0, 0, 128))
        self.screen.blit(overlay, (0, 0))

        countdown_text = self.countdown_font.render(str(self.countdown_value), True, WHITE)
        countdown_rect = countdown_text.get_rect(center=(SCREEN_WIDTH // 2, SCREEN_HEIGHT // 2))
        self.screen.blit(countdown_text, countdown_rect)

    def _draw_game_over_overlay(self):
        """Draw the game over overlay."""
        overlay = pygame.Surface((SCREEN_WIDTH, SCREEN_HEIGHT), pygame.SRCALPHA)
        overlay.fill((0, 0, 0, 128))
        self.screen.blit(overlay, (0, 0))

        game_over_text = self.large_font.render("GAME OVER", True, RED)
        game_over_rect = game_over_text.get_rect(center=(SCREEN_WIDTH // 2, SCREEN_HEIGHT // 2 - 30))
        self.screen.blit(game_over_text, game_over_rect)

        score_text = self.font.render(f"Final Score: {self.score}", True, WHITE)
        score_rect = score_text.get_rect(center=(SCREEN_WIDTH // 2, SCREEN_HEIGHT // 2 + 20))
        self.screen.blit(score_text, score_rect)

        if self.score >= self.high_score and self.score > 0:
            new_high_text = self.font.render("NEW HIGH SCORE!", True, YELLOW)
            new_high_rect = new_high_text.get_rect(center=(SCREEN_WIDTH // 2, SCREEN_HEIGHT // 2 + 55))
            self.screen.blit(new_high_text, new_high_rect)

        restart_text = self.font.render("Press SPACE to play again", True, GRAY)
        restart_rect = restart_text.get_rect(center=(SCREEN_WIDTH // 2, SCREEN_HEIGHT // 2 + 90))
        self.screen.blit(restart_text, restart_rect)

    def _draw_victory_overlay(self):
        """Draw the victory overlay."""
        overlay = pygame.Surface((SCREEN_WIDTH, SCREEN_HEIGHT), pygame.SRCALPHA)
        overlay.fill((0, 0, 0, 128))
        self.screen.blit(overlay, (0, 0))

        victory_text = self.large_font.render("YOU WIN!", True, GREEN)
        victory_rect = victory_text.get_rect(center=(SCREEN_WIDTH // 2, SCREEN_HEIGHT // 2 - 30))
        self.screen.blit(victory_text, victory_rect)

        score_text = self.font.render(f"Final Score: {self.score}", True, WHITE)
        score_rect = score_text.get_rect(center=(SCREEN_WIDTH // 2, SCREEN_HEIGHT // 2 + 20))
        self.screen.blit(score_text, score_rect)

        restart_text = self.font.render("Press SPACE to play again", True, GRAY)
        restart_rect = restart_text.get_rect(center=(SCREEN_WIDTH // 2, SCREEN_HEIGHT // 2 + 60))
        self.screen.blit(restart_text, restart_rect)

    def step(self) -> dict:
        """Execute one frame of the game and return state.

        Returns:
            Dictionary containing current game state information.
        """
        self.handle_events()
        self.handle_continuous_input()
        self.update()
        self.draw()
        self.clock.tick(FPS)
        self.frame_count += 1

        return {
            "state": self.state.value,
            "score": self.score,
            "lives": self.lives,
            "ball_position": (self.ball.position.x, self.ball.position.y),
            "paddle_position": self.paddle.x,
            "bricks_remaining": sum(1 for b in self.bricks if b.alive),
            "frame_count": self.frame_count,
            "running": self.running,
        }

    def run(self, max_frames: Optional[int] = None):
        """Run the main game loop.

        Args:
            max_frames: Maximum number of frames to run (for testing).
        """
        while self.running:
            self.step()

            if max_frames is not None and self.frame_count >= max_frames:
                break

        pygame.quit()


def main():
    """Entry point for the game."""
    parser = argparse.ArgumentParser(description="Breakout - Classic brick-breaking game")
    parser.add_argument("--headless", action="store_true", help="Run in headless mode")
    parser.add_argument("--frames", type=int, help="Maximum frames to run (headless mode)")
    args = parser.parse_args()

    game = Game(headless=args.headless)

    if args.headless and args.frames:
        game.set_state(GameState.PLAYING)
        game.run(max_frames=args.frames)
    else:
        game.run()


if __name__ == "__main__":
    main()