"""
Platformer - Side-scrolling platform game.

Features:
- Player with run, jump, double jump, and wall slide
- Tile-based level with collision detection
- Moving platforms
- Enemy AI with patrol and chase behaviors
- Collectibles (coins) and hazards (spikes)
- Camera following player
"""

import os
import sys
from enum import Enum, auto
from dataclasses import dataclass, field
from typing import List, Optional, Callable, Tuple, Dict

if "--headless" in sys.argv or os.environ.get("SDL_VIDEODRIVER") == "dummy":
    os.environ.setdefault("SDL_VIDEODRIVER", "dummy")
    os.environ.setdefault("SDL_AUDIODRIVER", "dummy")

import pygame

# Constants
SCREEN_WIDTH = 800
SCREEN_HEIGHT = 600
TILE_SIZE = 32
FPS = 60

# Physics
GRAVITY = 0.8
MAX_FALL_SPEED = 15
PLAYER_SPEED = 5
PLAYER_JUMP_FORCE = -14
PLAYER_DOUBLE_JUMP_FORCE = -12
WALL_SLIDE_SPEED = 2
WALL_JUMP_X_FORCE = 8

# Colors
BLACK = (0, 0, 0)
WHITE = (255, 255, 255)
BLUE = (50, 130, 200)
GREEN = (50, 200, 50)
RED = (200, 50, 50)
YELLOW = (255, 220, 50)
BROWN = (139, 90, 43)
GRAY = (100, 100, 100)
DARK_GRAY = (60, 60, 60)

# Tile types
TILE_AIR = 0
TILE_SOLID = 1
TILE_SPIKE = 2
TILE_PLATFORM = 3  # One-way platform


class GameState(Enum):
    MENU = auto()
    PLAYING = auto()
    PAUSED = auto()
    GAME_OVER = auto()
    LEVEL_COMPLETE = auto()


class EnemyState(Enum):
    PATROL = auto()
    CHASE = auto()
    RETURN = auto()


@dataclass
class Vector2:
    """2D vector for positions and velocities."""
    x: float = 0.0
    y: float = 0.0

    def __add__(self, other: "Vector2") -> "Vector2":
        return Vector2(self.x + other.x, self.y + other.y)

    def __sub__(self, other: "Vector2") -> "Vector2":
        return Vector2(self.x - other.x, self.y - other.y)

    def __mul__(self, scalar: float) -> "Vector2":
        return Vector2(self.x * scalar, self.y * scalar)

    def copy(self) -> "Vector2":
        return Vector2(self.x, self.y)


@dataclass
class Rect:
    """Rectangle for collision detection."""
    x: float
    y: float
    width: float
    height: float

    @property
    def left(self) -> float:
        return self.x

    @property
    def right(self) -> float:
        return self.x + self.width

    @property
    def top(self) -> float:
        return self.y

    @property
    def bottom(self) -> float:
        return self.y + self.height

    @property
    def center_x(self) -> float:
        return self.x + self.width / 2

    @property
    def center_y(self) -> float:
        return self.y + self.height / 2

    def intersects(self, other: "Rect") -> bool:
        return (self.left < other.right and self.right > other.left and
                self.top < other.bottom and self.bottom > other.top)

    def to_pygame(self) -> pygame.Rect:
        return pygame.Rect(int(self.x), int(self.y),
                          int(self.width), int(self.height))


class Player:
    """Player character with platformer controls."""

    def __init__(self, x: float, y: float):
        self.position = Vector2(x, y)
        self.velocity = Vector2(0, 0)
        self.width = 24
        self.height = 32
        self.facing_right = True
        self.on_ground = False
        self.on_wall = False
        self.wall_direction = 0  # -1 left, 1 right
        self.can_double_jump = True
        self.is_wall_sliding = False
        self.coyote_time = 0
        self.jump_buffer = 0
        self.invulnerable_timer = 0

    @property
    def rect(self) -> Rect:
        return Rect(self.position.x, self.position.y, self.width, self.height)

    def apply_gravity(self):
        """Apply gravity to velocity."""
        if self.is_wall_sliding:
            self.velocity.y = min(self.velocity.y + GRAVITY * 0.3, WALL_SLIDE_SPEED)
        else:
            self.velocity.y = min(self.velocity.y + GRAVITY, MAX_FALL_SPEED)

    def jump(self) -> bool:
        """Attempt to jump. Returns True if successful."""
        if self.on_ground or self.coyote_time > 0:
            self.velocity.y = PLAYER_JUMP_FORCE
            self.on_ground = False
            self.coyote_time = 0
            return True
        elif self.is_wall_sliding:
            # Wall jump
            self.velocity.y = PLAYER_JUMP_FORCE * 0.9
            self.velocity.x = WALL_JUMP_X_FORCE * -self.wall_direction
            self.on_wall = False
            self.is_wall_sliding = False
            self.can_double_jump = True
            return True
        elif self.can_double_jump:
            self.velocity.y = PLAYER_DOUBLE_JUMP_FORCE
            self.can_double_jump = False
            return True
        return False

    def move(self, direction: int):
        """Move player horizontally (-1 left, 1 right)."""
        self.velocity.x = direction * PLAYER_SPEED
        if direction != 0:
            self.facing_right = direction > 0

    def update(self):
        """Update player state."""
        # Update timers
        if self.coyote_time > 0:
            self.coyote_time -= 1
        if self.jump_buffer > 0:
            self.jump_buffer -= 1
        if self.invulnerable_timer > 0:
            self.invulnerable_timer -= 1

        # Wall slide detection
        if self.on_wall and not self.on_ground and self.velocity.y > 0:
            self.is_wall_sliding = True
        else:
            self.is_wall_sliding = False

        self.apply_gravity()
        self.position.x += self.velocity.x
        self.position.y += self.velocity.y

        # Reset horizontal velocity (no momentum)
        self.velocity.x = 0
        self.on_wall = False

    def land(self):
        """Called when player lands on ground."""
        self.on_ground = True
        self.can_double_jump = True
        self.coyote_time = 6  # ~100ms

    def leave_ground(self):
        """Called when player leaves ground."""
        if self.on_ground:
            self.on_ground = False
            self.coyote_time = 6

    def make_invulnerable(self, frames: int = 90):
        """Make player invulnerable."""
        self.invulnerable_timer = frames

    def is_invulnerable(self) -> bool:
        return self.invulnerable_timer > 0

    def draw(self, screen: pygame.Surface, camera_offset: Vector2):
        """Draw the player."""
        if self.invulnerable_timer > 0 and (self.invulnerable_timer // 5) % 2 == 0:
            return  # Blink when invulnerable

        x = int(self.position.x - camera_offset.x)
        y = int(self.position.y - camera_offset.y)
        pygame.draw.rect(screen, BLUE, (x, y, self.width, self.height))

        # Draw eyes to show facing direction
        eye_x = x + (self.width - 8 if self.facing_right else 4)
        pygame.draw.rect(screen, WHITE, (eye_x, y + 8, 6, 6))


class Enemy:
    """Enemy with patrol and chase AI."""

    def __init__(self, x: float, y: float, patrol_distance: float = 100):
        self.position = Vector2(x, y)
        self.velocity = Vector2(0, 0)
        self.width = 28
        self.height = 28
        self.start_x = x
        self.patrol_distance = patrol_distance
        self.state = EnemyState.PATROL
        self.facing_right = True
        self.speed = 2
        self.chase_speed = 3.5
        self.detection_range = 150
        self.on_ground = False

    @property
    def rect(self) -> Rect:
        return Rect(self.position.x, self.position.y, self.width, self.height)

    def update(self, player: Player, tilemap: "Tilemap"):
        """Update enemy AI and movement."""
        # Check if player is in detection range
        dist_to_player = abs(player.position.x - self.position.x)
        player_visible = (dist_to_player < self.detection_range and
                         abs(player.position.y - self.position.y) < 80)

        if self.state == EnemyState.PATROL:
            # Patrol back and forth
            if self.facing_right:
                self.velocity.x = self.speed
                if self.position.x > self.start_x + self.patrol_distance:
                    self.facing_right = False
            else:
                self.velocity.x = -self.speed
                if self.position.x < self.start_x - self.patrol_distance:
                    self.facing_right = True

            if player_visible:
                self.state = EnemyState.CHASE

        elif self.state == EnemyState.CHASE:
            # Chase player
            if player.position.x > self.position.x:
                self.velocity.x = self.chase_speed
                self.facing_right = True
            else:
                self.velocity.x = -self.chase_speed
                self.facing_right = False

            if not player_visible:
                self.state = EnemyState.RETURN

        elif self.state == EnemyState.RETURN:
            # Return to patrol area
            if abs(self.position.x - self.start_x) < 5:
                self.state = EnemyState.PATROL
            elif self.position.x > self.start_x:
                self.velocity.x = -self.speed
            else:
                self.velocity.x = self.speed

            if player_visible:
                self.state = EnemyState.CHASE

        # Apply gravity
        self.velocity.y = min(self.velocity.y + GRAVITY, MAX_FALL_SPEED)

        # Move and collide
        self.position.x += self.velocity.x
        self.position.y += self.velocity.y

    def draw(self, screen: pygame.Surface, camera_offset: Vector2):
        """Draw the enemy."""
        x = int(self.position.x - camera_offset.x)
        y = int(self.position.y - camera_offset.y)
        color = RED if self.state == EnemyState.CHASE else (200, 100, 100)
        pygame.draw.rect(screen, color, (x, y, self.width, self.height))

        # Draw eyes
        eye_x = x + (self.width - 10 if self.facing_right else 4)
        pygame.draw.rect(screen, WHITE, (eye_x, y + 6, 6, 6))


class MovingPlatform:
    """Platform that moves between two points."""

    def __init__(self, x: float, y: float, width: float,
                 move_x: float = 0, move_y: float = 0, speed: float = 2):
        self.start_pos = Vector2(x, y)
        self.position = Vector2(x, y)
        self.width = width
        self.height = TILE_SIZE // 2
        self.move_x = move_x
        self.move_y = move_y
        self.speed = speed
        self.progress = 0.0  # 0 to 1
        self.direction = 1
        self.velocity = Vector2(0, 0)

    @property
    def rect(self) -> Rect:
        return Rect(self.position.x, self.position.y, self.width, self.height)

    def update(self):
        """Update platform position."""
        old_x, old_y = self.position.x, self.position.y

        self.progress += self.speed / 100 * self.direction
        if self.progress >= 1:
            self.progress = 1
            self.direction = -1
        elif self.progress <= 0:
            self.progress = 0
            self.direction = 1

        self.position.x = self.start_pos.x + self.move_x * self.progress
        self.position.y = self.start_pos.y + self.move_y * self.progress

        self.velocity.x = self.position.x - old_x
        self.velocity.y = self.position.y - old_y

    def draw(self, screen: pygame.Surface, camera_offset: Vector2):
        """Draw the platform."""
        x = int(self.position.x - camera_offset.x)
        y = int(self.position.y - camera_offset.y)
        pygame.draw.rect(screen, BROWN, (x, y, int(self.width), int(self.height)))
        pygame.draw.rect(screen, DARK_GRAY, (x, y, int(self.width), int(self.height)), 2)


class Coin:
    """Collectible coin."""

    def __init__(self, x: float, y: float):
        self.position = Vector2(x, y)
        self.width = 16
        self.height = 16
        self.collected = False
        self.animation_offset = 0

    @property
    def rect(self) -> Rect:
        return Rect(self.position.x, self.position.y, self.width, self.height)

    def update(self):
        """Update animation."""
        self.animation_offset = (self.animation_offset + 0.1) % 6.28

    def draw(self, screen: pygame.Surface, camera_offset: Vector2):
        """Draw the coin."""
        if self.collected:
            return
        x = int(self.position.x - camera_offset.x)
        y = int(self.position.y - camera_offset.y + 2 * (1 + 0.5 * (self.animation_offset % 3.14 > 1.57)))
        pygame.draw.ellipse(screen, YELLOW, (x, y, self.width, self.height))
        pygame.draw.ellipse(screen, (200, 180, 0), (x, y, self.width, self.height), 2)


class Tilemap:
    """Tile-based level."""

    def __init__(self, width: int, height: int):
        self.width = width
        self.height = height
        self.tiles: List[List[int]] = [
            [TILE_AIR for _ in range(width)]
            for _ in range(height)
        ]

    def get_tile(self, x: int, y: int) -> int:
        """Get tile at grid position."""
        if 0 <= x < self.width and 0 <= y < self.height:
            return self.tiles[y][x]
        return TILE_SOLID  # Out of bounds is solid

    def set_tile(self, x: int, y: int, tile_type: int):
        """Set tile at grid position."""
        if 0 <= x < self.width and 0 <= y < self.height:
            self.tiles[y][x] = tile_type

    def get_tile_rect(self, gx: int, gy: int) -> Rect:
        """Get rectangle for tile at grid position."""
        return Rect(gx * TILE_SIZE, gy * TILE_SIZE, TILE_SIZE, TILE_SIZE)

    def collide_rect(self, rect: Rect) -> List[Tuple[int, int, int]]:
        """Get all tile collisions for a rectangle."""
        collisions = []
        start_x = max(0, int(rect.left // TILE_SIZE))
        end_x = min(self.width, int(rect.right // TILE_SIZE) + 1)
        start_y = max(0, int(rect.top // TILE_SIZE))
        end_y = min(self.height, int(rect.bottom // TILE_SIZE) + 1)

        for gy in range(start_y, end_y):
            for gx in range(start_x, end_x):
                tile = self.tiles[gy][gx]
                if tile != TILE_AIR:
                    tile_rect = self.get_tile_rect(gx, gy)
                    if rect.intersects(tile_rect):
                        collisions.append((gx, gy, tile))

        return collisions

    def draw(self, screen: pygame.Surface, camera_offset: Vector2):
        """Draw visible tiles."""
        start_x = max(0, int(camera_offset.x // TILE_SIZE))
        end_x = min(self.width, int((camera_offset.x + SCREEN_WIDTH) // TILE_SIZE) + 1)
        start_y = max(0, int(camera_offset.y // TILE_SIZE))
        end_y = min(self.height, int((camera_offset.y + SCREEN_HEIGHT) // TILE_SIZE) + 1)

        for gy in range(start_y, end_y):
            for gx in range(start_x, end_x):
                tile = self.tiles[gy][gx]
                if tile == TILE_AIR:
                    continue

                x = gx * TILE_SIZE - int(camera_offset.x)
                y = gy * TILE_SIZE - int(camera_offset.y)

                if tile == TILE_SOLID:
                    pygame.draw.rect(screen, GREEN, (x, y, TILE_SIZE, TILE_SIZE))
                    pygame.draw.rect(screen, DARK_GRAY, (x, y, TILE_SIZE, TILE_SIZE), 1)
                elif tile == TILE_SPIKE:
                    # Draw spike as triangle
                    points = [(x + TILE_SIZE // 2, y),
                             (x, y + TILE_SIZE),
                             (x + TILE_SIZE, y + TILE_SIZE)]
                    pygame.draw.polygon(screen, GRAY, points)
                elif tile == TILE_PLATFORM:
                    pygame.draw.rect(screen, BROWN, (x, y, TILE_SIZE, TILE_SIZE // 3))


class Game:
    """Main platformer game class."""

    def __init__(self, headless: bool = False):
        pygame.init()
        self.headless = headless

        if headless:
            self.screen = pygame.Surface((SCREEN_WIDTH, SCREEN_HEIGHT))
        else:
            self.screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))
            pygame.display.set_caption("Platformer")

        self.clock = pygame.time.Clock()
        self.font = pygame.font.Font(None, 36)
        self.small_font = pygame.font.Font(None, 24)

        # Callbacks
        self.on_score: Optional[Callable[[int], None]] = None
        self.on_state_change: Optional[Callable[[GameState], None]] = None
        self.on_death: Optional[Callable[[], None]] = None
        self.on_coin_collect: Optional[Callable[[], None]] = None

        self.reset()

    def reset(self):
        """Reset game to initial state."""
        self.state = GameState.MENU
        self.score = 0
        self.lives = 3
        self.coins_collected = 0
        self.total_coins = 0
        self.camera_offset = Vector2(0, 0)
        self.injected_input: dict = {}

        self._create_level()

    def _create_level(self):
        """Create the game level."""
        # Create tilemap (25 tiles wide, 20 tiles tall)
        self.tilemap = Tilemap(50, 20)

        # Ground
        for x in range(50):
            self.tilemap.set_tile(x, 18, TILE_SOLID)
            self.tilemap.set_tile(x, 19, TILE_SOLID)

        # Platforms
        for x in range(5, 9):
            self.tilemap.set_tile(x, 14, TILE_SOLID)
        for x in range(12, 16):
            self.tilemap.set_tile(x, 12, TILE_SOLID)
        for x in range(18, 22):
            self.tilemap.set_tile(x, 15, TILE_PLATFORM)
        for x in range(25, 30):
            self.tilemap.set_tile(x, 13, TILE_SOLID)

        # Walls for wall jumping
        for y in range(10, 18):
            self.tilemap.set_tile(34, y, TILE_SOLID)
            self.tilemap.set_tile(38, y, TILE_SOLID)

        # Spikes
        for x in range(40, 43):
            self.tilemap.set_tile(x, 17, TILE_SPIKE)

        # Create player
        self.player = Player(100, 500)

        # Create enemies
        self.enemies: List[Enemy] = [
            Enemy(300, 500, patrol_distance=80),
            Enemy(500, 500, patrol_distance=60),
        ]

        # Create moving platforms
        self.platforms: List[MovingPlatform] = [
            MovingPlatform(600, 400, 96, move_y=100, speed=1),
            MovingPlatform(750, 300, 96, move_x=100, speed=2),
        ]

        # Create coins
        self.coins: List[Coin] = [
            Coin(200, 490),
            Coin(250, 490),
            Coin(350, 350),
            Coin(450, 350),
            Coin(700, 300),
        ]
        self.total_coins = len(self.coins)

    def set_state(self, state: GameState):
        """Change game state."""
        old_state = self.state
        self.state = state
        if self.on_state_change and old_state != state:
            self.on_state_change(state)

    def add_score(self, points: int):
        """Add points to score."""
        self.score += points
        if self.on_score:
            self.on_score(points)

    def inject_input(self, inputs: dict):
        """Inject input for testing."""
        self.injected_input = inputs

    def _get_input(self) -> dict:
        """Get current input state."""
        if self.injected_input:
            return self.injected_input

        keys = pygame.key.get_pressed()
        return {
            "left": keys[pygame.K_LEFT] or keys[pygame.K_a],
            "right": keys[pygame.K_RIGHT] or keys[pygame.K_d],
            "jump": keys[pygame.K_SPACE] or keys[pygame.K_UP] or keys[pygame.K_w],
        }

    def _handle_events(self) -> bool:
        """Handle pygame events. Returns False to quit."""
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                return False
            if event.type == pygame.KEYDOWN:
                if event.key == pygame.K_ESCAPE:
                    if self.state == GameState.PLAYING:
                        self.set_state(GameState.PAUSED)
                    elif self.state == GameState.PAUSED:
                        self.set_state(GameState.PLAYING)
                elif event.key == pygame.K_RETURN:
                    if self.state in (GameState.MENU, GameState.GAME_OVER):
                        self.reset()
                        self.set_state(GameState.PLAYING)
                elif event.key in (pygame.K_SPACE, pygame.K_UP, pygame.K_w):
                    if self.state == GameState.PLAYING:
                        self.player.jump()
        return True

    def _resolve_player_collisions(self):
        """Resolve player collisions with tilemap and platforms."""
        player = self.player

        # Horizontal collision
        player_rect = player.rect
        collisions = self.tilemap.collide_rect(player_rect)
        for gx, gy, tile in collisions:
            if tile == TILE_SOLID:
                tile_rect = self.tilemap.get_tile_rect(gx, gy)
                if player.velocity.x > 0:
                    player.position.x = tile_rect.left - player.width
                    player.on_wall = True
                    player.wall_direction = 1
                elif player.velocity.x < 0:
                    player.position.x = tile_rect.right
                    player.on_wall = True
                    player.wall_direction = -1

        # Vertical collision
        player_rect = player.rect
        collisions = self.tilemap.collide_rect(player_rect)
        was_on_ground = player.on_ground
        player.on_ground = False

        for gx, gy, tile in collisions:
            tile_rect = self.tilemap.get_tile_rect(gx, gy)

            if tile == TILE_SOLID:
                if player.velocity.y > 0:
                    player.position.y = tile_rect.top - player.height
                    player.velocity.y = 0
                    player.land()
                elif player.velocity.y < 0:
                    player.position.y = tile_rect.bottom
                    player.velocity.y = 0

            elif tile == TILE_PLATFORM:
                # One-way platform - only collide from above
                if (player.velocity.y > 0 and
                    player_rect.bottom - player.velocity.y <= tile_rect.top + 2):
                    player.position.y = tile_rect.top - player.height
                    player.velocity.y = 0
                    player.land()

            elif tile == TILE_SPIKE:
                if not player.is_invulnerable():
                    self._player_hit()

        if was_on_ground and not player.on_ground:
            player.leave_ground()

        # Moving platform collision
        for platform in self.platforms:
            plat_rect = platform.rect
            player_rect = player.rect

            # Check if player is standing on platform
            if (player.velocity.y >= 0 and
                player_rect.bottom >= plat_rect.top and
                player_rect.bottom <= plat_rect.top + 10 and
                player_rect.right > plat_rect.left and
                player_rect.left < plat_rect.right):
                player.position.y = plat_rect.top - player.height
                player.position.x += platform.velocity.x
                player.velocity.y = 0
                player.land()

    def _player_hit(self):
        """Handle player getting hit."""
        self.lives -= 1
        if self.on_death:
            self.on_death()

        if self.lives <= 0:
            self.set_state(GameState.GAME_OVER)
        else:
            self.player.make_invulnerable()

    def _update_camera(self):
        """Update camera to follow player."""
        target_x = self.player.position.x - SCREEN_WIDTH // 3
        target_y = self.player.position.y - SCREEN_HEIGHT // 2

        # Smooth follow
        self.camera_offset.x += (target_x - self.camera_offset.x) * 0.1
        self.camera_offset.y += (target_y - self.camera_offset.y) * 0.1

        # Clamp to level bounds
        max_x = self.tilemap.width * TILE_SIZE - SCREEN_WIDTH
        max_y = self.tilemap.height * TILE_SIZE - SCREEN_HEIGHT
        self.camera_offset.x = max(0, min(self.camera_offset.x, max_x))
        self.camera_offset.y = max(0, min(self.camera_offset.y, max_y))

    def _update_playing(self):
        """Update game during PLAYING state."""
        inputs = self._get_input()

        # Player movement
        direction = 0
        if inputs.get("left"):
            direction -= 1
        if inputs.get("right"):
            direction += 1
        self.player.move(direction)

        # Jump from injected input
        if inputs.get("jump"):
            self.player.jump()

        self.player.update()
        self._resolve_player_collisions()

        # Update enemies
        for enemy in self.enemies:
            enemy.update(self.player, self.tilemap)
            # Simple enemy collision
            if self.player.rect.intersects(enemy.rect):
                if not self.player.is_invulnerable():
                    # Check if player is stomping
                    if (self.player.velocity.y > 0 and
                        self.player.position.y + self.player.height < enemy.position.y + 10):
                        self.enemies.remove(enemy)
                        self.add_score(100)
                        self.player.velocity.y = PLAYER_JUMP_FORCE * 0.5
                    else:
                        self._player_hit()

        # Update platforms
        for platform in self.platforms:
            platform.update()

        # Update and collect coins
        for coin in self.coins:
            if coin.collected:
                continue
            coin.update()
            if self.player.rect.intersects(coin.rect):
                coin.collected = True
                self.coins_collected += 1
                self.add_score(50)
                if self.on_coin_collect:
                    self.on_coin_collect()

        # Check level complete
        if self.coins_collected >= self.total_coins:
            self.set_state(GameState.LEVEL_COMPLETE)

        # Check fall death
        if self.player.position.y > self.tilemap.height * TILE_SIZE:
            self._player_hit()
            if self.state == GameState.PLAYING:
                self.player.position = Vector2(100, 500)

        self._update_camera()

    def step(self) -> bool:
        """Execute one game frame. Returns False to quit."""
        if not self._handle_events():
            return False

        if self.state == GameState.PLAYING:
            self._update_playing()

        self._draw()
        self.clock.tick(FPS)
        self.injected_input = {}
        return True

    def _draw(self):
        """Draw the current frame."""
        self.screen.fill((100, 150, 200))  # Sky blue

        if self.state == GameState.MENU:
            self._draw_menu()
        elif self.state == GameState.PLAYING:
            self._draw_game()
            self._draw_hud()
        elif self.state == GameState.PAUSED:
            self._draw_game()
            self._draw_hud()
            self._draw_paused()
        elif self.state == GameState.GAME_OVER:
            self._draw_game()
            self._draw_game_over()
        elif self.state == GameState.LEVEL_COMPLETE:
            self._draw_game()
            self._draw_level_complete()

        if not self.headless:
            pygame.display.flip()

    def _draw_menu(self):
        """Draw menu screen."""
        title = self.font.render("PLATFORMER", True, WHITE)
        self.screen.blit(title,
                        (SCREEN_WIDTH // 2 - title.get_width() // 2, 200))

        start = self.font.render("Press ENTER to Start", True, WHITE)
        self.screen.blit(start,
                        (SCREEN_WIDTH // 2 - start.get_width() // 2, 300))

        controls = self.small_font.render("Arrows/WASD + Space", True, WHITE)
        self.screen.blit(controls,
                        (SCREEN_WIDTH // 2 - controls.get_width() // 2, 350))

    def _draw_game(self):
        """Draw game entities."""
        self.tilemap.draw(self.screen, self.camera_offset)

        for platform in self.platforms:
            platform.draw(self.screen, self.camera_offset)

        for coin in self.coins:
            coin.draw(self.screen, self.camera_offset)

        for enemy in self.enemies:
            enemy.draw(self.screen, self.camera_offset)

        self.player.draw(self.screen, self.camera_offset)

    def _draw_hud(self):
        """Draw HUD elements."""
        # Score
        score_text = self.font.render(f"Score: {self.score}", True, WHITE)
        self.screen.blit(score_text, (10, 10))

        # Lives
        lives_text = self.font.render(f"Lives: {self.lives}", True, WHITE)
        self.screen.blit(lives_text, (10, 45))

        # Coins
        coins_text = self.font.render(
            f"Coins: {self.coins_collected}/{self.total_coins}", True, YELLOW
        )
        self.screen.blit(coins_text, (SCREEN_WIDTH - 150, 10))

    def _draw_paused(self):
        """Draw pause overlay."""
        overlay = pygame.Surface((SCREEN_WIDTH, SCREEN_HEIGHT))
        overlay.fill(BLACK)
        overlay.set_alpha(128)
        self.screen.blit(overlay, (0, 0))

        paused = self.font.render("PAUSED", True, WHITE)
        self.screen.blit(paused,
                        (SCREEN_WIDTH // 2 - paused.get_width() // 2,
                         SCREEN_HEIGHT // 2))

    def _draw_game_over(self):
        """Draw game over overlay."""
        overlay = pygame.Surface((SCREEN_WIDTH, SCREEN_HEIGHT))
        overlay.fill(BLACK)
        overlay.set_alpha(128)
        self.screen.blit(overlay, (0, 0))

        game_over = self.font.render("GAME OVER", True, RED)
        self.screen.blit(game_over,
                        (SCREEN_WIDTH // 2 - game_over.get_width() // 2, 250))

        final_score = self.font.render(f"Score: {self.score}", True, WHITE)
        self.screen.blit(final_score,
                        (SCREEN_WIDTH // 2 - final_score.get_width() // 2, 300))

        restart = self.small_font.render("Press ENTER to Restart", True, WHITE)
        self.screen.blit(restart,
                        (SCREEN_WIDTH // 2 - restart.get_width() // 2, 350))

    def _draw_level_complete(self):
        """Draw level complete overlay."""
        overlay = pygame.Surface((SCREEN_WIDTH, SCREEN_HEIGHT))
        overlay.fill(BLACK)
        overlay.set_alpha(128)
        self.screen.blit(overlay, (0, 0))

        complete = self.font.render("LEVEL COMPLETE!", True, GREEN)
        self.screen.blit(complete,
                        (SCREEN_WIDTH // 2 - complete.get_width() // 2, 250))

        final_score = self.font.render(f"Final Score: {self.score}", True, WHITE)
        self.screen.blit(final_score,
                        (SCREEN_WIDTH // 2 - final_score.get_width() // 2, 300))

    def run(self):
        """Run the main game loop."""
        running = True
        while running:
            running = self.step()
        pygame.quit()


def main():
    """Entry point."""
    headless = "--headless" in sys.argv
    game = Game(headless=headless)
    game.run()


if __name__ == "__main__":
    main()
