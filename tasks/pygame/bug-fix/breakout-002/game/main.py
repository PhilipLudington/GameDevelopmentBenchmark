#!/usr/bin/env python3
"""Breakout - Classic brick-breaking arcade game.

BUG: The on_score callback receives the total score instead of the
points earned from the brick that was just destroyed.
"""

import argparse
import os
import sys
from dataclasses import dataclass
from enum import Enum
from typing import Callable, Optional

if "--headless" in sys.argv or os.environ.get("SDL_VIDEODRIVER") == "dummy":
    os.environ.setdefault("SDL_VIDEODRIVER", "dummy")
    os.environ.setdefault("SDL_AUDIODRIVER", "dummy")

import pygame

SCREEN_WIDTH = 800
SCREEN_HEIGHT = 600
FPS = 60
PADDLE_WIDTH = 100
PADDLE_HEIGHT = 15
PADDLE_SPEED = 8
PADDLE_Y = SCREEN_HEIGHT - 50
BALL_RADIUS = 8
BALL_SPEED = 5
BALL_START_Y = SCREEN_HEIGHT - 100
BRICK_ROWS = 5
BRICK_COLS = 10
BRICK_WIDTH = 70
BRICK_HEIGHT = 25
BRICK_PADDING = 5
BRICK_TOP_OFFSET = 80
BRICK_LEFT_OFFSET = (SCREEN_WIDTH - (BRICK_COLS * (BRICK_WIDTH + BRICK_PADDING) - BRICK_PADDING)) // 2
INITIAL_LIVES = 3
POINTS_PER_BRICK = 10
WHITE = (255, 255, 255)
BLACK = (0, 0, 0)
GRAY = (128, 128, 128)
RED = (255, 0, 0)
ORANGE = (255, 165, 0)
YELLOW = (255, 255, 0)
GREEN = (0, 255, 0)
BLUE = (0, 100, 255)
BRICK_COLORS = [RED, ORANGE, YELLOW, GREEN, BLUE]


class GameState(Enum):
    MENU = "menu"
    PLAYING = "playing"
    PAUSED = "paused"
    GAME_OVER = "game_over"
    VICTORY = "victory"


@dataclass
class Vector2:
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
    def __init__(self):
        self.width = PADDLE_WIDTH
        self.height = PADDLE_HEIGHT
        self.speed = PADDLE_SPEED
        self.reset()

    def reset(self):
        self.x = (SCREEN_WIDTH - self.width) // 2
        self.y = PADDLE_Y

    def move_left(self):
        self.x = max(0, self.x - self.speed)

    def move_right(self):
        self.x = min(SCREEN_WIDTH - self.width, self.x + self.speed)

    @property
    def rect(self) -> pygame.Rect:
        return pygame.Rect(self.x, self.y, self.width, self.height)

    @property
    def center_x(self) -> float:
        return self.x + self.width / 2


class Ball:
    def __init__(self):
        self.radius = BALL_RADIUS
        self.speed = BALL_SPEED
        self.reset()

    def reset(self):
        self.position = Vector2(SCREEN_WIDTH // 2, BALL_START_Y)
        self.velocity = Vector2(self.speed * 0.7, self.speed * 0.7)

    @property
    def rect(self) -> pygame.Rect:
        return pygame.Rect(
            self.position.x - self.radius,
            self.position.y - self.radius,
            self.radius * 2,
            self.radius * 2
        )

    def update(self):
        self.position = self.position + self.velocity

    def bounce_horizontal(self):
        self.velocity.x = -self.velocity.x

    def bounce_vertical(self):
        self.velocity.y = -self.velocity.y

    def bounce_off_paddle(self, paddle: Paddle):
        import math
        relative_x = (self.position.x - paddle.center_x) / (paddle.width / 2)
        relative_x = max(-1, min(1, relative_x))
        max_angle = math.pi / 3
        angle = relative_x * max_angle
        self.velocity.x = self.speed * math.sin(angle)
        self.velocity.y = -self.speed * math.cos(angle)


@dataclass
class Brick:
    row: int
    col: int
    color: tuple
    alive: bool = True

    @property
    def x(self) -> int:
        return BRICK_LEFT_OFFSET + self.col * (BRICK_WIDTH + BRICK_PADDING)

    @property
    def y(self) -> int:
        return BRICK_TOP_OFFSET + self.row * (BRICK_HEIGHT + BRICK_PADDING)

    @property
    def rect(self) -> pygame.Rect:
        return pygame.Rect(self.x, self.y, BRICK_WIDTH, BRICK_HEIGHT)

    @property
    def points(self) -> int:
        return POINTS_PER_BRICK * (BRICK_ROWS - self.row)


class Game:
    def __init__(self, headless: bool = False):
        self.headless = headless
        pygame.init()
        self.screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))
        if not headless:
            pygame.display.set_caption("Breakout")
        self.clock = pygame.time.Clock()
        self.font = pygame.font.Font(None, 36)
        self.large_font = pygame.font.Font(None, 72)
        self.paddle = Paddle()
        self.ball = Ball()
        self.bricks: list[Brick] = []
        self.state = GameState.MENU
        self.score = 0
        self.high_score = 0
        self.lives = INITIAL_LIVES
        self.frame_count = 0
        self.running = True
        self.on_score: Optional[Callable[[int], None]] = None
        self.on_state_change: Optional[Callable[[GameState, GameState], None]] = None
        self.on_brick_destroyed: Optional[Callable[[Brick], None]] = None
        self.on_life_lost: Optional[Callable[[int], None]] = None
        self._create_bricks()

    def _create_bricks(self):
        self.bricks = []
        for row in range(BRICK_ROWS):
            color = BRICK_COLORS[row % len(BRICK_COLORS)]
            for col in range(BRICK_COLS):
                self.bricks.append(Brick(row=row, col=col, color=color))

    def reset_game(self):
        self.paddle.reset()
        self.ball.reset()
        self._create_bricks()
        self.score = 0
        self.lives = INITIAL_LIVES

    def reset_ball(self):
        self.ball.reset()
        self.paddle.reset()

    def set_state(self, new_state: GameState):
        old_state = self.state
        if old_state == new_state:
            return
        self.state = new_state
        if self.on_state_change:
            self.on_state_change(old_state, new_state)
        if new_state == GameState.PLAYING:
            if old_state in (GameState.MENU, GameState.GAME_OVER, GameState.VICTORY):
                self.reset_game()
        elif new_state == GameState.GAME_OVER:
            if self.score > self.high_score:
                self.high_score = self.score

    def handle_events(self):
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                self.running = False
                return
            if event.type == pygame.KEYDOWN:
                if event.key == pygame.K_ESCAPE:
                    if self.state == GameState.PLAYING:
                        self.set_state(GameState.PAUSED)
                    elif self.state == GameState.PAUSED:
                        self.set_state(GameState.PLAYING)
                    elif self.state in (GameState.MENU, GameState.GAME_OVER, GameState.VICTORY):
                        self.running = False
                elif event.key in (pygame.K_SPACE, pygame.K_RETURN):
                    if self.state in (GameState.MENU, GameState.PAUSED, GameState.GAME_OVER, GameState.VICTORY):
                        self.set_state(GameState.PLAYING)
                elif event.key == pygame.K_p:
                    if self.state == GameState.PLAYING:
                        self.set_state(GameState.PAUSED)
                    elif self.state == GameState.PAUSED:
                        self.set_state(GameState.PLAYING)

    def handle_continuous_input(self):
        if self.state != GameState.PLAYING:
            return
        keys = pygame.key.get_pressed()
        if keys[pygame.K_LEFT] or keys[pygame.K_a]:
            self.paddle.move_left()
        if keys[pygame.K_RIGHT] or keys[pygame.K_d]:
            self.paddle.move_right()

    def update(self):
        if self.state != GameState.PLAYING:
            return
        self.ball.update()
        if self.ball.position.x - self.ball.radius <= 0:
            self.ball.position.x = self.ball.radius
            self.ball.bounce_horizontal()
        elif self.ball.position.x + self.ball.radius >= SCREEN_WIDTH:
            self.ball.position.x = SCREEN_WIDTH - self.ball.radius
            self.ball.bounce_horizontal()
        if self.ball.position.y - self.ball.radius <= 0:
            self.ball.position.y = self.ball.radius
            self.ball.bounce_vertical()
        if self.ball.position.y + self.ball.radius >= SCREEN_HEIGHT:
            self.lives -= 1
            if self.on_life_lost:
                self.on_life_lost(self.lives)
            if self.lives <= 0:
                self.set_state(GameState.GAME_OVER)
            else:
                self.reset_ball()
            return
        if self.ball.rect.colliderect(self.paddle.rect):
            if self.ball.velocity.y > 0:
                self.ball.position.y = self.paddle.y - self.ball.radius
                self.ball.bounce_off_paddle(self.paddle)
        for brick in self.bricks:
            if not brick.alive:
                continue
            if self.ball.rect.colliderect(brick.rect):
                brick.alive = False
                ball_center_x = self.ball.position.x
                ball_center_y = self.ball.position.y
                brick_center_x = brick.x + BRICK_WIDTH / 2
                brick_center_y = brick.y + BRICK_HEIGHT / 2
                dx = ball_center_x - brick_center_x
                dy = ball_center_y - brick_center_y
                if abs(dx) / BRICK_WIDTH > abs(dy) / BRICK_HEIGHT:
                    self.ball.bounce_horizontal()
                else:
                    self.ball.bounce_vertical()
                self.score += brick.points
                # BUG: Callback receives total score instead of points earned
                if self.on_score:
                    self.on_score(self.score)  # Should be brick.points!
                if self.on_brick_destroyed:
                    self.on_brick_destroyed(brick)
                break
        if all(not brick.alive for brick in self.bricks):
            self.set_state(GameState.VICTORY)

    def draw(self):
        self.screen.fill(BLACK)
        if self.state == GameState.MENU:
            self._draw_menu()
        elif self.state == GameState.PLAYING:
            self._draw_game()
        elif self.state == GameState.PAUSED:
            self._draw_game()
            self._draw_overlay("PAUSED", WHITE)
        elif self.state == GameState.GAME_OVER:
            self._draw_game()
            self._draw_overlay("GAME OVER", RED)
        elif self.state == GameState.VICTORY:
            self._draw_game()
            self._draw_overlay("YOU WIN!", GREEN)
        pygame.display.flip()

    def _draw_menu(self):
        title = self.large_font.render("BREAKOUT", True, WHITE)
        self.screen.blit(title, title.get_rect(center=(SCREEN_WIDTH // 2, SCREEN_HEIGHT // 3)))

    def _draw_game(self):
        pygame.draw.rect(self.screen, WHITE, self.paddle.rect)
        pygame.draw.circle(self.screen, WHITE,
                          (int(self.ball.position.x), int(self.ball.position.y)), self.ball.radius)
        for brick in self.bricks:
            if brick.alive:
                pygame.draw.rect(self.screen, brick.color, brick.rect)
                pygame.draw.rect(self.screen, WHITE, brick.rect, 1)
        score_text = self.font.render(f"Score: {self.score}", True, WHITE)
        self.screen.blit(score_text, (10, 10))
        lives_text = self.font.render(f"Lives: {self.lives}", True, WHITE)
        self.screen.blit(lives_text, lives_text.get_rect(topright=(SCREEN_WIDTH - 10, 10)))

    def _draw_overlay(self, text, color):
        overlay = pygame.Surface((SCREEN_WIDTH, SCREEN_HEIGHT), pygame.SRCALPHA)
        overlay.fill((0, 0, 0, 128))
        self.screen.blit(overlay, (0, 0))
        text_surf = self.large_font.render(text, True, color)
        self.screen.blit(text_surf, text_surf.get_rect(center=(SCREEN_WIDTH // 2, SCREEN_HEIGHT // 2)))

    def step(self) -> dict:
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
        while self.running:
            self.step()
            if max_frames is not None and self.frame_count >= max_frames:
                break
        pygame.quit()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--headless", action="store_true")
    parser.add_argument("--frames", type=int)
    args = parser.parse_args()
    game = Game(headless=args.headless)
    if args.headless and args.frames:
        game.set_state(GameState.PLAYING)
        game.run(max_frames=args.frames)
    else:
        game.run()


if __name__ == "__main__":
    main()
