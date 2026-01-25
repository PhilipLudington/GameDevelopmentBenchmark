#!/usr/bin/env python3
"""Space Invaders - Classic arcade game implemented in Pygame.

A baseline implementation for the Game Development Benchmark.
Features:
- Player ship controlled by left/right arrow keys or A/D
- Shooting with spacebar
- Multiple rows of alien invaders that move and descend
- Alien shooting
- Shield barriers that can be destroyed
- Scoring system
- Lives system
- Game states (menu, playing, paused, game over, victory)
- Headless mode support for automated testing
"""

import argparse
import math
import os
import random
import sys
from dataclasses import dataclass, field
from enum import Enum, auto
from typing import Callable, List, Optional

# Set up headless mode BEFORE importing pygame
if "--headless" in sys.argv or os.environ.get("SDL_VIDEODRIVER") == "dummy":
    os.environ.setdefault("SDL_VIDEODRIVER", "dummy")
    os.environ.setdefault("SDL_AUDIODRIVER", "dummy")

import pygame

# Screen dimensions
SCREEN_WIDTH = 800
SCREEN_HEIGHT = 600
FPS = 60

# Player settings
PLAYER_WIDTH = 50
PLAYER_HEIGHT = 30
PLAYER_SPEED = 5
PLAYER_Y = SCREEN_HEIGHT - 60

# Bullet settings
BULLET_WIDTH = 4
BULLET_HEIGHT = 15
PLAYER_BULLET_SPEED = 8
ALIEN_BULLET_SPEED = 4

# Alien settings
ALIEN_ROWS = 5
ALIEN_COLS = 11
ALIEN_WIDTH = 40
ALIEN_HEIGHT = 30
ALIEN_PADDING_X = 10
ALIEN_PADDING_Y = 10
ALIEN_START_Y = 80
ALIEN_MOVE_SPEED = 1
ALIEN_DROP_DISTANCE = 20
ALIEN_SHOOT_CHANCE = 0.002  # Per alien per frame

# Shield settings
SHIELD_COUNT = 4
SHIELD_WIDTH = 70
SHIELD_HEIGHT = 50
SHIELD_Y = SCREEN_HEIGHT - 150
SHIELD_BLOCK_SIZE = 5

# Game settings
INITIAL_LIVES = 3
POINTS_ROW_1 = 30  # Top row
POINTS_ROW_2 = 20
POINTS_ROW_3 = 20
POINTS_ROW_4 = 10
POINTS_ROW_5 = 10  # Bottom row

# Colors
WHITE = (255, 255, 255)
BLACK = (0, 0, 0)
GREEN = (0, 255, 0)
RED = (255, 0, 0)
YELLOW = (255, 255, 0)
CYAN = (0, 255, 255)
MAGENTA = (255, 0, 255)
GRAY = (128, 128, 128)

# Alien colors by row (from top to bottom)
ALIEN_COLORS = [MAGENTA, CYAN, CYAN, GREEN, GREEN]
POINTS_PER_ROW = [POINTS_ROW_1, POINTS_ROW_2, POINTS_ROW_3, POINTS_ROW_4, POINTS_ROW_5]


class GameState(Enum):
    """Game state machine states."""
    MENU = auto()
    PLAYING = auto()
    PAUSED = auto()
    GAME_OVER = auto()
    VICTORY = auto()


@dataclass
class Vector2:
    """2D vector for positions and velocities."""
    x: float
    y: float

    def __add__(self, other: "Vector2") -> "Vector2":
        return Vector2(self.x + other.x, self.y + other.y)

    def __sub__(self, other: "Vector2") -> "Vector2":
        return Vector2(self.x - other.x, self.y - other.y)

    def copy(self) -> "Vector2":
        return Vector2(self.x, self.y)


class Player:
    """Player-controlled ship at the bottom of the screen."""

    def __init__(self):
        self.width = PLAYER_WIDTH
        self.height = PLAYER_HEIGHT
        self.speed = PLAYER_SPEED
        self.reset()

    def reset(self):
        """Reset player to center position."""
        self.x = (SCREEN_WIDTH - self.width) // 2
        self.y = PLAYER_Y

    def move_left(self):
        """Move player left, clamped to screen bounds."""
        self.x = max(0, self.x - self.speed)

    def move_right(self):
        """Move player right, clamped to screen bounds."""
        self.x = min(SCREEN_WIDTH - self.width, self.x + self.speed)

    @property
    def rect(self) -> pygame.Rect:
        """Get player rectangle for collision detection."""
        return pygame.Rect(self.x, self.y, self.width, self.height)

    @property
    def center_x(self) -> float:
        """Get center x position of player."""
        return self.x + self.width / 2

    def draw(self, surface: pygame.Surface):
        """Draw the player ship."""
        # Draw a simple ship shape
        points = [
            (self.x + self.width // 2, self.y),  # Top center
            (self.x + self.width, self.y + self.height),  # Bottom right
            (self.x, self.y + self.height),  # Bottom left
        ]
        pygame.draw.polygon(surface, GREEN, points)


@dataclass
class Bullet:
    """A bullet fired by player or alien."""
    x: float
    y: float
    speed: float
    is_player_bullet: bool

    @property
    def rect(self) -> pygame.Rect:
        """Get bullet rectangle for collision detection."""
        return pygame.Rect(self.x, self.y, BULLET_WIDTH, BULLET_HEIGHT)

    def update(self):
        """Update bullet position."""
        if self.is_player_bullet:
            self.y -= self.speed
        else:
            self.y += self.speed

    def is_off_screen(self) -> bool:
        """Check if bullet has left the screen."""
        return self.y < -BULLET_HEIGHT or self.y > SCREEN_HEIGHT

    def draw(self, surface: pygame.Surface):
        """Draw the bullet."""
        color = GREEN if self.is_player_bullet else RED
        pygame.draw.rect(surface, color, self.rect)


@dataclass
class Alien:
    """An alien invader."""
    row: int
    col: int
    x: float
    y: float
    alive: bool = True

    @property
    def color(self) -> tuple:
        """Get color based on row."""
        return ALIEN_COLORS[self.row % len(ALIEN_COLORS)]

    @property
    def points(self) -> int:
        """Get points for destroying this alien."""
        return POINTS_PER_ROW[self.row % len(POINTS_PER_ROW)]

    @property
    def rect(self) -> pygame.Rect:
        """Get alien rectangle for collision detection."""
        return pygame.Rect(self.x, self.y, ALIEN_WIDTH, ALIEN_HEIGHT)

    def draw(self, surface: pygame.Surface):
        """Draw the alien."""
        if not self.alive:
            return
        # Draw a simple alien shape (rectangle with antennae)
        pygame.draw.rect(surface, self.color, self.rect)
        # Draw antennae
        pygame.draw.line(
            surface, self.color,
            (self.x + 10, self.y),
            (self.x + 5, self.y - 8), 2
        )
        pygame.draw.line(
            surface, self.color,
            (self.x + ALIEN_WIDTH - 10, self.y),
            (self.x + ALIEN_WIDTH - 5, self.y - 8), 2
        )


class AlienFleet:
    """Manages the fleet of aliens."""

    def __init__(self):
        self.aliens: List[Alien] = []
        self.direction = 1  # 1 = right, -1 = left
        self.move_timer = 0
        self.move_delay = 30  # Frames between moves
        self.reset()

    def reset(self):
        """Reset the fleet to initial positions."""
        self.aliens = []
        self.direction = 1
        self.move_timer = 0
        self.move_delay = 30

        start_x = (SCREEN_WIDTH - (ALIEN_COLS * (ALIEN_WIDTH + ALIEN_PADDING_X) - ALIEN_PADDING_X)) // 2

        for row in range(ALIEN_ROWS):
            for col in range(ALIEN_COLS):
                x = start_x + col * (ALIEN_WIDTH + ALIEN_PADDING_X)
                y = ALIEN_START_Y + row * (ALIEN_HEIGHT + ALIEN_PADDING_Y)
                self.aliens.append(Alien(row=row, col=col, x=x, y=y))

    def update(self) -> bool:
        """Update alien positions.

        Returns:
            True if aliens reached the bottom (game over condition).
        """
        self.move_timer += 1
        if self.move_timer < self.move_delay:
            return False

        self.move_timer = 0

        # Check if any alien hit the edge
        hit_edge = False
        for alien in self.aliens:
            if not alien.alive:
                continue
            if self.direction > 0 and alien.x + ALIEN_WIDTH >= SCREEN_WIDTH - 10:
                hit_edge = True
                break
            if self.direction < 0 and alien.x <= 10:
                hit_edge = True
                break

        # Move aliens
        if hit_edge:
            # Move down and reverse direction
            for alien in self.aliens:
                alien.y += ALIEN_DROP_DISTANCE
            self.direction *= -1
            # Speed up as aliens are destroyed
            alive_count = sum(1 for a in self.aliens if a.alive)
            self.move_delay = max(5, int(30 * alive_count / (ALIEN_ROWS * ALIEN_COLS)))
        else:
            # Move sideways
            for alien in self.aliens:
                alien.x += ALIEN_MOVE_SPEED * self.direction

        # Check if any alien reached the bottom
        for alien in self.aliens:
            if alien.alive and alien.y + ALIEN_HEIGHT >= PLAYER_Y:
                return True

        return False

    def get_shooters(self) -> List[Alien]:
        """Get aliens that can shoot (bottom-most in each column)."""
        shooters = []
        for col in range(ALIEN_COLS):
            column_aliens = [a for a in self.aliens if a.col == col and a.alive]
            if column_aliens:
                bottom_alien = max(column_aliens, key=lambda a: a.y)
                shooters.append(bottom_alien)
        return shooters

    def try_shoot(self) -> Optional[Bullet]:
        """Attempt to have an alien shoot.

        Returns:
            A new bullet if an alien shoots, None otherwise.
        """
        shooters = self.get_shooters()
        for alien in shooters:
            if random.random() < ALIEN_SHOOT_CHANCE:
                return Bullet(
                    x=alien.x + ALIEN_WIDTH // 2 - BULLET_WIDTH // 2,
                    y=alien.y + ALIEN_HEIGHT,
                    speed=ALIEN_BULLET_SPEED,
                    is_player_bullet=False
                )
        return None

    @property
    def alive_count(self) -> int:
        """Get count of living aliens."""
        return sum(1 for a in self.aliens if a.alive)

    @property
    def all_dead(self) -> bool:
        """Check if all aliens are destroyed."""
        return self.alive_count == 0

    def draw(self, surface: pygame.Surface):
        """Draw all alive aliens."""
        for alien in self.aliens:
            if alien.alive:
                alien.draw(surface)


@dataclass
class ShieldBlock:
    """A single block of a shield."""
    x: int
    y: int
    alive: bool = True

    @property
    def rect(self) -> pygame.Rect:
        return pygame.Rect(self.x, self.y, SHIELD_BLOCK_SIZE, SHIELD_BLOCK_SIZE)


class Shield:
    """A destructible shield barrier."""

    def __init__(self, x: int, y: int):
        self.x = x
        self.y = y
        self.blocks: List[ShieldBlock] = []
        self._create_blocks()

    def _create_blocks(self):
        """Create the shield blocks in an arch shape."""
        self.blocks = []
        blocks_wide = SHIELD_WIDTH // SHIELD_BLOCK_SIZE
        blocks_tall = SHIELD_HEIGHT // SHIELD_BLOCK_SIZE

        for row in range(blocks_tall):
            for col in range(blocks_wide):
                # Create arch shape by excluding corners at the bottom
                if row >= blocks_tall - 2:
                    # Bottom rows - create arch
                    center = blocks_wide // 2
                    if abs(col - center) < 3:
                        continue  # Skip center bottom for arch

                block_x = self.x + col * SHIELD_BLOCK_SIZE
                block_y = self.y + row * SHIELD_BLOCK_SIZE
                self.blocks.append(ShieldBlock(x=block_x, y=block_y))

    def check_collision(self, rect: pygame.Rect) -> bool:
        """Check and handle collision with a bullet.

        Args:
            rect: The bullet rectangle.

        Returns:
            True if collision occurred.
        """
        for block in self.blocks:
            if block.alive and block.rect.colliderect(rect):
                block.alive = False
                return True
        return False

    @property
    def is_destroyed(self) -> bool:
        """Check if all blocks are destroyed."""
        return all(not block.alive for block in self.blocks)

    def draw(self, surface: pygame.Surface):
        """Draw the shield."""
        for block in self.blocks:
            if block.alive:
                pygame.draw.rect(surface, GREEN, block.rect)


class Game:
    """Main game class managing all game logic and rendering."""

    def __init__(self, headless: bool = False):
        """Initialize the game.

        Args:
            headless: If True, run without display for testing.
        """
        self.headless = headless
        pygame.init()

        self.screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))
        if not headless:
            pygame.display.set_caption("Space Invaders")

        self.clock = pygame.time.Clock()
        self.font = pygame.font.Font(None, 36)
        self.large_font = pygame.font.Font(None, 72)

        # Game objects
        self.player = Player()
        self.fleet = AlienFleet()
        self.shields: List[Shield] = []
        self.player_bullets: List[Bullet] = []
        self.alien_bullets: List[Bullet] = []

        # Game state
        self.state = GameState.MENU
        self.score = 0
        self.high_score = 0
        self.lives = INITIAL_LIVES
        self.frame_count = 0
        self.running = True
        self.shoot_cooldown = 0
        self.shoot_delay = 15  # Frames between shots

        # Callbacks for testing
        self.on_score: Optional[Callable[[int], None]] = None
        self.on_state_change: Optional[Callable[[GameState, GameState], None]] = None
        self.on_alien_destroyed: Optional[Callable[[Alien], None]] = None
        self.on_life_lost: Optional[Callable[[int], None]] = None

        self._create_shields()

    def _create_shields(self):
        """Create shield barriers."""
        self.shields = []
        spacing = SCREEN_WIDTH // (SHIELD_COUNT + 1)
        for i in range(SHIELD_COUNT):
            x = spacing * (i + 1) - SHIELD_WIDTH // 2
            self.shields.append(Shield(x, SHIELD_Y))

    def reset_game(self):
        """Reset game to initial state."""
        self.player.reset()
        self.fleet.reset()
        self._create_shields()
        self.player_bullets = []
        self.alien_bullets = []
        self.score = 0
        self.lives = INITIAL_LIVES
        self.shoot_cooldown = 0

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
            if old_state in (GameState.MENU, GameState.GAME_OVER, GameState.VICTORY):
                self.reset_game()
        elif new_state == GameState.GAME_OVER:
            if self.score > self.high_score:
                self.high_score = self.score

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
                        self.set_state(GameState.PLAYING)
                    elif self.state in (GameState.MENU, GameState.GAME_OVER, GameState.VICTORY):
                        self.running = False

                elif event.key in (pygame.K_SPACE, pygame.K_RETURN):
                    if self.state == GameState.MENU:
                        self.set_state(GameState.PLAYING)
                    elif self.state == GameState.PAUSED:
                        self.set_state(GameState.PLAYING)
                    elif self.state == GameState.GAME_OVER:
                        self.set_state(GameState.PLAYING)
                    elif self.state == GameState.VICTORY:
                        self.set_state(GameState.PLAYING)

                elif event.key == pygame.K_p:
                    if self.state == GameState.PLAYING:
                        self.set_state(GameState.PAUSED)
                    elif self.state == GameState.PAUSED:
                        self.set_state(GameState.PLAYING)

    def handle_continuous_input(self):
        """Handle continuous key presses."""
        if self.state != GameState.PLAYING:
            return

        keys = pygame.key.get_pressed()

        # Movement
        if keys[pygame.K_LEFT] or keys[pygame.K_a]:
            self.player.move_left()
        if keys[pygame.K_RIGHT] or keys[pygame.K_d]:
            self.player.move_right()

        # Shooting
        if keys[pygame.K_SPACE] and self.shoot_cooldown == 0:
            self.player_bullets.append(Bullet(
                x=self.player.center_x - BULLET_WIDTH // 2,
                y=self.player.y,
                speed=PLAYER_BULLET_SPEED,
                is_player_bullet=True
            ))
            self.shoot_cooldown = self.shoot_delay

    def update(self):
        """Update game logic for one frame."""
        if self.state != GameState.PLAYING:
            return

        # Update shoot cooldown
        if self.shoot_cooldown > 0:
            self.shoot_cooldown -= 1

        # Update player bullets
        for bullet in self.player_bullets[:]:
            bullet.update()
            if bullet.is_off_screen():
                self.player_bullets.remove(bullet)
                continue

            # Check collision with aliens
            for alien in self.fleet.aliens:
                if alien.alive and bullet.rect.colliderect(alien.rect):
                    alien.alive = False
                    if bullet in self.player_bullets:
                        self.player_bullets.remove(bullet)

                    old_score = self.score
                    self.score += alien.points
                    if self.on_score:
                        self.on_score(alien.points)
                    if self.on_alien_destroyed:
                        self.on_alien_destroyed(alien)
                    break

            # Check collision with shields
            for shield in self.shields:
                if bullet in self.player_bullets and shield.check_collision(bullet.rect):
                    self.player_bullets.remove(bullet)
                    break

        # Update alien bullets
        for bullet in self.alien_bullets[:]:
            bullet.update()
            if bullet.is_off_screen():
                self.alien_bullets.remove(bullet)
                continue

            # Check collision with player
            if bullet.rect.colliderect(self.player.rect):
                self.alien_bullets.remove(bullet)
                self.lives -= 1
                if self.on_life_lost:
                    self.on_life_lost(self.lives)

                if self.lives <= 0:
                    self.set_state(GameState.GAME_OVER)
                    return
                continue

            # Check collision with shields
            for shield in self.shields:
                if shield.check_collision(bullet.rect):
                    if bullet in self.alien_bullets:
                        self.alien_bullets.remove(bullet)
                    break

        # Update alien fleet
        reached_bottom = self.fleet.update()
        if reached_bottom:
            self.set_state(GameState.GAME_OVER)
            return

        # Aliens try to shoot
        new_bullet = self.fleet.try_shoot()
        if new_bullet:
            self.alien_bullets.append(new_bullet)

        # Check for victory
        if self.fleet.all_dead:
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
        elif self.state == GameState.GAME_OVER:
            self._draw_game()
            self._draw_game_over_overlay()
        elif self.state == GameState.VICTORY:
            self._draw_game()
            self._draw_victory_overlay()

        pygame.display.flip()

    def _draw_menu(self):
        """Draw the main menu screen."""
        title = self.large_font.render("SPACE INVADERS", True, GREEN)
        title_rect = title.get_rect(center=(SCREEN_WIDTH // 2, SCREEN_HEIGHT // 3))
        self.screen.blit(title, title_rect)

        instructions = [
            "Press SPACE or ENTER to start",
            "Arrow keys or A/D to move",
            "SPACE to shoot",
            "P or ESC to pause",
            "",
            f"High Score: {self.high_score}"
        ]

        for i, text in enumerate(instructions):
            line = self.font.render(text, True, GRAY if i == 4 else WHITE)
            line_rect = line.get_rect(center=(SCREEN_WIDTH // 2, SCREEN_HEIGHT // 2 + i * 35))
            self.screen.blit(line, line_rect)

    def _draw_game(self):
        """Draw the main game elements."""
        # Draw player
        self.player.draw(self.screen)

        # Draw alien fleet
        self.fleet.draw(self.screen)

        # Draw shields
        for shield in self.shields:
            shield.draw(self.screen)

        # Draw bullets
        for bullet in self.player_bullets:
            bullet.draw(self.screen)
        for bullet in self.alien_bullets:
            bullet.draw(self.screen)

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

        victory_text = self.large_font.render("VICTORY!", True, GREEN)
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
            "state": self.state.name,
            "score": self.score,
            "lives": self.lives,
            "player_x": self.player.x,
            "aliens_remaining": self.fleet.alive_count,
            "player_bullets": len(self.player_bullets),
            "alien_bullets": len(self.alien_bullets),
            "frame_count": self.frame_count,
            "running": self.running,
        }

    def inject_input(self, action: str):
        """Inject input for testing/bots.

        Args:
            action: One of 'left', 'right', 'shoot'
        """
        if action == 'left':
            self.player.move_left()
        elif action == 'right':
            self.player.move_right()
        elif action == 'shoot' and self.shoot_cooldown == 0:
            self.player_bullets.append(Bullet(
                x=self.player.center_x - BULLET_WIDTH // 2,
                y=self.player.y,
                speed=PLAYER_BULLET_SPEED,
                is_player_bullet=True
            ))
            self.shoot_cooldown = self.shoot_delay

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
    parser = argparse.ArgumentParser(description="Space Invaders - Classic arcade game")
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
