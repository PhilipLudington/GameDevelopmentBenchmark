"""
Asteroids - Classic asteroid shooting game with vector graphics style.

Features:
- Player ship with rotation and thrust-based movement
- Asteroids that split into smaller pieces when shot
- Screen wrapping for all entities
- Particle effects for explosions and thrust
- UFO enemy with targeting AI
- Score system with increasing difficulty
"""

import os
import sys
import math
import random
from enum import Enum, auto
from dataclasses import dataclass, field
from typing import List, Optional, Callable, Tuple

if "--headless" in sys.argv or os.environ.get("SDL_VIDEODRIVER") == "dummy":
    os.environ.setdefault("SDL_VIDEODRIVER", "dummy")
    os.environ.setdefault("SDL_AUDIODRIVER", "dummy")

import pygame

# Constants
SCREEN_WIDTH = 800
SCREEN_HEIGHT = 600
FPS = 60

# Colors
WHITE = (255, 255, 255)
BLACK = (0, 0, 0)
RED = (255, 0, 0)
GREEN = (0, 255, 0)
YELLOW = (255, 255, 0)
CYAN = (0, 255, 255)

# Ship constants
SHIP_SIZE = 20
SHIP_ROTATION_SPEED = 5
SHIP_THRUST = 0.15
SHIP_MAX_SPEED = 8
SHIP_DRAG = 0.99
SHIP_INVULNERABLE_TIME = 180  # 3 seconds at 60 FPS

# Bullet constants
BULLET_SPEED = 10
BULLET_LIFETIME = 60
MAX_BULLETS = 4

# Asteroid constants
ASTEROID_SPEEDS = {3: 1.5, 2: 2.5, 1: 3.5}  # size -> speed
ASTEROID_RADII = {3: 40, 2: 20, 1: 10}  # size -> radius
ASTEROID_SCORES = {3: 20, 2: 50, 1: 100}  # size -> score
INITIAL_ASTEROIDS = 4

# UFO constants
UFO_SIZE = 25
UFO_SPEED = 3
UFO_SHOOT_INTERVAL = 90  # 1.5 seconds at 60 FPS
UFO_SCORE = 200
UFO_SPAWN_SCORE = 1000


class GameState(Enum):
    MENU = auto()
    PLAYING = auto()
    PAUSED = auto()
    GAME_OVER = auto()


@dataclass
class Vector2:
    """2D vector with math operations."""
    x: float = 0.0
    y: float = 0.0

    def __add__(self, other: "Vector2") -> "Vector2":
        return Vector2(self.x + other.x, self.y + other.y)

    def __sub__(self, other: "Vector2") -> "Vector2":
        return Vector2(self.x - other.x, self.y - other.y)

    def __mul__(self, scalar: float) -> "Vector2":
        return Vector2(self.x * scalar, self.y * scalar)

    def __rmul__(self, scalar: float) -> "Vector2":
        return self.__mul__(scalar)

    def magnitude(self) -> float:
        return math.sqrt(self.x * self.x + self.y * self.y)

    def normalize(self) -> "Vector2":
        mag = self.magnitude()
        if mag == 0:
            return Vector2(0, 0)
        return Vector2(self.x / mag, self.y / mag)

    def limit(self, max_val: float) -> "Vector2":
        mag = self.magnitude()
        if mag > max_val:
            return self.normalize() * max_val
        return Vector2(self.x, self.y)

    def to_tuple(self) -> Tuple[float, float]:
        return (self.x, self.y)

    def distance_to(self, other: "Vector2") -> float:
        return (self - other).magnitude()

    @staticmethod
    def from_angle(angle: float, magnitude: float = 1.0) -> "Vector2":
        """Create vector from angle (degrees) and magnitude."""
        rad = math.radians(angle)
        return Vector2(math.cos(rad) * magnitude, math.sin(rad) * magnitude)


@dataclass
class Particle:
    """Particle for visual effects."""
    position: Vector2
    velocity: Vector2
    lifetime: int
    color: Tuple[int, int, int]
    size: float = 2.0

    def update(self) -> bool:
        """Update particle. Returns False when dead."""
        self.position = self.position + self.velocity
        self.lifetime -= 1
        self.velocity = self.velocity * 0.98  # Drag
        return self.lifetime > 0

    def draw(self, screen: pygame.Surface):
        """Draw the particle."""
        alpha = min(255, self.lifetime * 10)
        color = tuple(min(255, int(c * alpha / 255)) for c in self.color)
        pygame.draw.circle(
            screen, color,
            (int(self.position.x), int(self.position.y)),
            max(1, int(self.size * self.lifetime / 30))
        )


class Ship:
    """Player ship with rotation and thrust movement."""

    def __init__(self, x: float, y: float):
        self.position = Vector2(x, y)
        self.velocity = Vector2(0, 0)
        self.angle = -90  # Pointing up
        self.thrusting = False
        self.invulnerable_timer = 0
        self.visible = True

    def rotate_left(self):
        """Rotate ship counter-clockwise."""
        self.angle -= SHIP_ROTATION_SPEED

    def rotate_right(self):
        """Rotate ship clockwise."""
        self.angle += SHIP_ROTATION_SPEED

    def thrust(self):
        """Apply thrust in facing direction."""
        self.thrusting = True
        thrust_vector = Vector2.from_angle(self.angle, SHIP_THRUST)
        self.velocity = self.velocity + thrust_vector
        self.velocity = self.velocity.limit(SHIP_MAX_SPEED)

    def update(self):
        """Update ship position and state."""
        self.velocity = self.velocity * SHIP_DRAG
        self.position = self.position + self.velocity
        self.wrap_position()

        if self.invulnerable_timer > 0:
            self.invulnerable_timer -= 1
            # Blink effect
            self.visible = (self.invulnerable_timer // 10) % 2 == 0

        self.thrusting = False

    def wrap_position(self):
        """Wrap position around screen edges."""
        if self.position.x < 0:
            self.position.x = SCREEN_WIDTH
        elif self.position.x > SCREEN_WIDTH:
            self.position.x = 0
        if self.position.y < 0:
            self.position.y = SCREEN_HEIGHT
        elif self.position.y > SCREEN_HEIGHT:
            self.position.y = 0

    def make_invulnerable(self):
        """Make ship invulnerable for a period."""
        self.invulnerable_timer = SHIP_INVULNERABLE_TIME
        self.visible = True

    def is_invulnerable(self) -> bool:
        """Check if ship is currently invulnerable."""
        return self.invulnerable_timer > 0

    def get_points(self) -> List[Tuple[int, int]]:
        """Get ship vertices for drawing."""
        points = []
        # Triangle pointing in direction of angle
        for offset in [0, 140, 220]:  # Front, back-left, back-right
            angle = self.angle + offset
            rad = math.radians(angle)
            x = self.position.x + math.cos(rad) * SHIP_SIZE
            y = self.position.y + math.sin(rad) * SHIP_SIZE
            points.append((int(x), int(y)))
        return points

    def get_thrust_particles(self) -> List[Particle]:
        """Generate thrust exhaust particles."""
        if not self.thrusting:
            return []
        particles = []
        for _ in range(2):
            # Spawn behind ship
            back_angle = self.angle + 180
            offset = Vector2.from_angle(back_angle, SHIP_SIZE)
            pos = self.position + offset
            # Random spread
            vel = Vector2.from_angle(
                back_angle + random.uniform(-20, 20),
                random.uniform(2, 4)
            )
            particles.append(Particle(
                position=pos,
                velocity=vel,
                lifetime=random.randint(10, 20),
                color=random.choice([RED, YELLOW, (255, 128, 0)])
            ))
        return particles

    def draw(self, screen: pygame.Surface):
        """Draw the ship."""
        if not self.visible:
            return
        points = self.get_points()
        pygame.draw.polygon(screen, WHITE, points, 2)
        # Draw thrust flame
        if self.thrusting:
            back_angle = self.angle + 180
            flame_pos = self.position + Vector2.from_angle(back_angle, SHIP_SIZE)
            flame_tip = flame_pos + Vector2.from_angle(
                back_angle, random.uniform(5, 15)
            )
            pygame.draw.line(
                screen, YELLOW,
                (int(flame_pos.x), int(flame_pos.y)),
                (int(flame_tip.x), int(flame_tip.y)), 2
            )


class Bullet:
    """Projectile fired by ship or UFO."""

    def __init__(self, x: float, y: float, angle: float, is_player: bool = True):
        self.position = Vector2(x, y)
        self.velocity = Vector2.from_angle(angle, BULLET_SPEED)
        self.lifetime = BULLET_LIFETIME
        self.is_player = is_player
        self.radius = 2

    def update(self) -> bool:
        """Update bullet. Returns False when expired."""
        self.position = self.position + self.velocity
        self.wrap_position()
        self.lifetime -= 1
        return self.lifetime > 0

    def wrap_position(self):
        """Wrap position around screen edges."""
        if self.position.x < 0:
            self.position.x = SCREEN_WIDTH
        elif self.position.x > SCREEN_WIDTH:
            self.position.x = 0
        if self.position.y < 0:
            self.position.y = SCREEN_HEIGHT
        elif self.position.y > SCREEN_HEIGHT:
            self.position.y = 0

    def draw(self, screen: pygame.Surface):
        """Draw the bullet."""
        color = WHITE if self.is_player else RED
        pygame.draw.circle(
            screen, color,
            (int(self.position.x), int(self.position.y)),
            self.radius
        )


class Asteroid:
    """Asteroid that splits when destroyed."""

    def __init__(self, x: float, y: float, size: int = 3, velocity: Optional[Vector2] = None):
        self.position = Vector2(x, y)
        self.size = size  # 3=large, 2=medium, 1=small
        self.radius = ASTEROID_RADII[size]
        self.score = ASTEROID_SCORES[size]

        if velocity:
            self.velocity = velocity
        else:
            angle = random.uniform(0, 360)
            speed = ASTEROID_SPEEDS[size] * random.uniform(0.8, 1.2)
            self.velocity = Vector2.from_angle(angle, speed)

        # Generate random shape
        self.vertices = self._generate_vertices()

    def _generate_vertices(self) -> List[Tuple[float, float]]:
        """Generate irregular polygon vertices."""
        vertices = []
        num_vertices = random.randint(8, 12)
        for i in range(num_vertices):
            angle = (360 / num_vertices) * i
            # Vary radius for irregular shape
            r = self.radius * random.uniform(0.7, 1.0)
            rad = math.radians(angle)
            vertices.append((math.cos(rad) * r, math.sin(rad) * r))
        return vertices

    def update(self):
        """Update asteroid position."""
        self.position = self.position + self.velocity
        self.wrap_position()

    def wrap_position(self):
        """Wrap position around screen edges."""
        if self.position.x < -self.radius:
            self.position.x = SCREEN_WIDTH + self.radius
        elif self.position.x > SCREEN_WIDTH + self.radius:
            self.position.x = -self.radius
        if self.position.y < -self.radius:
            self.position.y = SCREEN_HEIGHT + self.radius
        elif self.position.y > SCREEN_HEIGHT + self.radius:
            self.position.y = -self.radius

    def split(self) -> List["Asteroid"]:
        """Split asteroid into smaller pieces. Returns empty list if smallest."""
        if self.size <= 1:
            return []

        new_size = self.size - 1
        pieces = []
        for _ in range(2):
            # Random velocity perpendicular-ish to original
            angle = math.degrees(math.atan2(self.velocity.y, self.velocity.x))
            new_angle = angle + random.uniform(-60, 60) + 90 * random.choice([-1, 1])
            speed = ASTEROID_SPEEDS[new_size] * random.uniform(0.8, 1.2)
            vel = Vector2.from_angle(new_angle, speed)
            pieces.append(Asteroid(self.position.x, self.position.y, new_size, vel))
        return pieces

    def collides_with_point(self, point: Vector2) -> bool:
        """Check collision with a point."""
        return self.position.distance_to(point) < self.radius

    def collides_with_circle(self, pos: Vector2, radius: float) -> bool:
        """Check collision with another circle."""
        return self.position.distance_to(pos) < self.radius + radius

    def draw(self, screen: pygame.Surface):
        """Draw the asteroid."""
        points = [
            (int(self.position.x + v[0]), int(self.position.y + v[1]))
            for v in self.vertices
        ]
        pygame.draw.polygon(screen, WHITE, points, 2)


class UFO:
    """UFO enemy that shoots at player."""

    def __init__(self, target_ship: Ship):
        self.target = target_ship
        # Spawn from left or right edge
        if random.random() < 0.5:
            x = -UFO_SIZE
            self.direction = 1
        else:
            x = SCREEN_WIDTH + UFO_SIZE
            self.direction = -1
        y = random.uniform(100, SCREEN_HEIGHT - 100)
        self.position = Vector2(x, y)
        self.velocity = Vector2(UFO_SPEED * self.direction, 0)
        self.shoot_timer = UFO_SHOOT_INTERVAL
        self.radius = UFO_SIZE // 2

    def update(self) -> Optional[Bullet]:
        """Update UFO. Returns a bullet if shooting, None otherwise."""
        # Move horizontally with slight vertical wobble
        self.velocity.y = math.sin(pygame.time.get_ticks() / 300) * 2
        self.position = self.position + self.velocity

        # Shoot at player
        self.shoot_timer -= 1
        bullet = None
        if self.shoot_timer <= 0:
            self.shoot_timer = UFO_SHOOT_INTERVAL
            bullet = self._shoot()

        return bullet

    def _shoot(self) -> Bullet:
        """Shoot at player with some inaccuracy."""
        angle = math.degrees(math.atan2(
            self.target.position.y - self.position.y,
            self.target.position.x - self.position.x
        ))
        angle += random.uniform(-15, 15)  # Add inaccuracy
        return Bullet(self.position.x, self.position.y, angle, is_player=False)

    def is_offscreen(self) -> bool:
        """Check if UFO has left the screen."""
        return (self.position.x < -UFO_SIZE * 2 or
                self.position.x > SCREEN_WIDTH + UFO_SIZE * 2)

    def collides_with_point(self, point: Vector2) -> bool:
        """Check collision with a point."""
        return self.position.distance_to(point) < self.radius

    def collides_with_circle(self, pos: Vector2, radius: float) -> bool:
        """Check collision with another circle."""
        return self.position.distance_to(pos) < self.radius + radius

    def draw(self, screen: pygame.Surface):
        """Draw the UFO."""
        x, y = int(self.position.x), int(self.position.y)
        # Draw saucer shape
        pygame.draw.ellipse(screen, WHITE,
                           (x - UFO_SIZE, y - UFO_SIZE//3,
                            UFO_SIZE * 2, UFO_SIZE * 2//3), 2)
        pygame.draw.ellipse(screen, WHITE,
                           (x - UFO_SIZE//2, y - UFO_SIZE//2,
                            UFO_SIZE, UFO_SIZE//2), 2)


class Game:
    """Main game class managing all game logic."""

    def __init__(self, headless: bool = False):
        pygame.init()
        self.headless = headless

        if headless:
            self.screen = pygame.Surface((SCREEN_WIDTH, SCREEN_HEIGHT))
        else:
            self.screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))
            pygame.display.set_caption("Asteroids")

        self.clock = pygame.time.Clock()
        self.font = pygame.font.Font(None, 36)

        # Callbacks
        self.on_score: Optional[Callable[[int], None]] = None
        self.on_state_change: Optional[Callable[[GameState], None]] = None
        self.on_death: Optional[Callable[[], None]] = None

        self.reset()

    def reset(self):
        """Reset game to initial state."""
        self.state = GameState.MENU
        self.ship = Ship(SCREEN_WIDTH // 2, SCREEN_HEIGHT // 2)
        self.bullets: List[Bullet] = []
        self.asteroids: List[Asteroid] = []
        self.particles: List[Particle] = []
        self.ufo: Optional[UFO] = None
        self.score = 0
        self.lives = 3
        self.level = 1
        self.ufo_spawn_threshold = UFO_SPAWN_SCORE
        self.injected_input: dict = {}

        self._spawn_asteroids(INITIAL_ASTEROIDS)

    def _spawn_asteroids(self, count: int):
        """Spawn asteroids away from ship."""
        for _ in range(count):
            while True:
                x = random.uniform(0, SCREEN_WIDTH)
                y = random.uniform(0, SCREEN_HEIGHT)
                pos = Vector2(x, y)
                # Don't spawn too close to ship
                if pos.distance_to(self.ship.position) > 150:
                    self.asteroids.append(Asteroid(x, y, 3))
                    break

    def set_state(self, state: GameState):
        """Change game state."""
        old_state = self.state
        self.state = state
        if self.on_state_change and old_state != state:
            self.on_state_change(state)

    def add_score(self, points: int):
        """Add points to score."""
        old_score = self.score
        self.score += points
        if self.on_score:
            self.on_score(points)

        # Check UFO spawn
        if old_score < self.ufo_spawn_threshold <= self.score:
            self._spawn_ufo()
            self.ufo_spawn_threshold += UFO_SPAWN_SCORE

    def _spawn_ufo(self):
        """Spawn UFO if none exists."""
        if self.ufo is None:
            self.ufo = UFO(self.ship)

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
            "thrust": keys[pygame.K_UP] or keys[pygame.K_w],
            "shoot": keys[pygame.K_SPACE],
        }

    def _spawn_explosion(self, position: Vector2, count: int = 20,
                         color: Tuple[int, int, int] = WHITE):
        """Spawn explosion particles."""
        for _ in range(count):
            angle = random.uniform(0, 360)
            speed = random.uniform(1, 5)
            vel = Vector2.from_angle(angle, speed)
            self.particles.append(Particle(
                position=Vector2(position.x, position.y),
                velocity=vel,
                lifetime=random.randint(20, 40),
                color=color
            ))

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
                elif event.key == pygame.K_SPACE:
                    if self.state == GameState.PLAYING:
                        self._shoot()
        return True

    def _shoot(self):
        """Fire a bullet from the ship."""
        if len(self.bullets) < MAX_BULLETS:
            # Spawn bullet at ship front
            front = self.ship.position + Vector2.from_angle(
                self.ship.angle, SHIP_SIZE
            )
            self.bullets.append(Bullet(
                front.x, front.y, self.ship.angle, is_player=True
            ))

    def _update_playing(self):
        """Update game during PLAYING state."""
        inputs = self._get_input()

        # Ship controls
        if inputs.get("left"):
            self.ship.rotate_left()
        if inputs.get("right"):
            self.ship.rotate_right()
        if inputs.get("thrust"):
            self.ship.thrust()
            self.particles.extend(self.ship.get_thrust_particles())

        self.ship.update()

        # Update bullets
        self.bullets = [b for b in self.bullets if b.update()]

        # Update asteroids
        for asteroid in self.asteroids:
            asteroid.update()

        # Update UFO
        if self.ufo:
            ufo_bullet = self.ufo.update()
            if ufo_bullet:
                self.bullets.append(ufo_bullet)
            if self.ufo.is_offscreen():
                self.ufo = None

        # Update particles
        self.particles = [p for p in self.particles if p.update()]

        # Check collisions
        self._check_collisions()

        # Check level complete
        if not self.asteroids:
            self._next_level()

    def _check_collisions(self):
        """Check all collisions."""
        # Player bullets vs asteroids
        bullets_to_remove = set()
        asteroids_to_remove = set()
        new_asteroids = []

        for i, bullet in enumerate(self.bullets):
            if not bullet.is_player:
                continue
            for j, asteroid in enumerate(self.asteroids):
                if asteroid.collides_with_point(bullet.position):
                    bullets_to_remove.add(i)
                    asteroids_to_remove.add(j)
                    new_asteroids.extend(asteroid.split())
                    self.add_score(asteroid.score)
                    self._spawn_explosion(asteroid.position)
                    break

        # Player bullets vs UFO
        if self.ufo:
            for i, bullet in enumerate(self.bullets):
                if bullet.is_player and self.ufo.collides_with_point(bullet.position):
                    bullets_to_remove.add(i)
                    self.add_score(UFO_SCORE)
                    self._spawn_explosion(self.ufo.position, 30, CYAN)
                    self.ufo = None
                    break

        # Ship vs asteroids (if not invulnerable)
        if not self.ship.is_invulnerable():
            for asteroid in self.asteroids:
                if asteroid.collides_with_circle(self.ship.position, SHIP_SIZE * 0.6):
                    self._player_death()
                    break

        # Ship vs UFO bullets
        if not self.ship.is_invulnerable():
            for i, bullet in enumerate(self.bullets):
                if not bullet.is_player:
                    dist = self.ship.position.distance_to(bullet.position)
                    if dist < SHIP_SIZE * 0.6:
                        bullets_to_remove.add(i)
                        self._player_death()
                        break

        # Ship vs UFO
        if self.ufo and not self.ship.is_invulnerable():
            if self.ufo.collides_with_circle(self.ship.position, SHIP_SIZE * 0.6):
                self._player_death()
                self.ufo = None

        # Remove destroyed objects
        self.bullets = [b for i, b in enumerate(self.bullets)
                        if i not in bullets_to_remove]
        self.asteroids = [a for i, a in enumerate(self.asteroids)
                         if i not in asteroids_to_remove]
        self.asteroids.extend(new_asteroids)

    def _player_death(self):
        """Handle player death."""
        self._spawn_explosion(self.ship.position, 40, YELLOW)
        self.lives -= 1

        if self.on_death:
            self.on_death()

        if self.lives <= 0:
            self.set_state(GameState.GAME_OVER)
        else:
            # Respawn
            self.ship = Ship(SCREEN_WIDTH // 2, SCREEN_HEIGHT // 2)
            self.ship.make_invulnerable()

    def _next_level(self):
        """Advance to next level."""
        self.level += 1
        asteroid_count = INITIAL_ASTEROIDS + self.level - 1
        self._spawn_asteroids(min(asteroid_count, 12))

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
        self.screen.fill(BLACK)

        if self.state == GameState.MENU:
            self._draw_menu()
        elif self.state == GameState.PLAYING:
            self._draw_game()
        elif self.state == GameState.PAUSED:
            self._draw_game()
            self._draw_paused()
        elif self.state == GameState.GAME_OVER:
            self._draw_game()
            self._draw_game_over()

        if not self.headless:
            pygame.display.flip()

    def _draw_menu(self):
        """Draw menu screen."""
        title = self.font.render("ASTEROIDS", True, WHITE)
        self.screen.blit(title,
                        (SCREEN_WIDTH // 2 - title.get_width() // 2, 200))

        start = self.font.render("Press ENTER to Start", True, WHITE)
        self.screen.blit(start,
                        (SCREEN_WIDTH // 2 - start.get_width() // 2, 300))

        controls = self.font.render("Arrow Keys + Space", True, WHITE)
        self.screen.blit(controls,
                        (SCREEN_WIDTH // 2 - controls.get_width() // 2, 350))

    def _draw_game(self):
        """Draw game entities."""
        self.ship.draw(self.screen)

        for bullet in self.bullets:
            bullet.draw(self.screen)

        for asteroid in self.asteroids:
            asteroid.draw(self.screen)

        if self.ufo:
            self.ufo.draw(self.screen)

        for particle in self.particles:
            particle.draw(self.screen)

        self._draw_hud()

    def _draw_hud(self):
        """Draw score and lives."""
        score_text = self.font.render(f"Score: {self.score}", True, WHITE)
        self.screen.blit(score_text, (10, 10))

        level_text = self.font.render(f"Level: {self.level}", True, WHITE)
        self.screen.blit(level_text, (10, 45))

        # Draw lives as small ships
        for i in range(self.lives):
            x = SCREEN_WIDTH - 30 - i * 25
            points = [
                (x, 25),
                (x - 8, 35),
                (x + 8, 35)
            ]
            pygame.draw.polygon(self.screen, WHITE, points, 2)

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

        final_score = self.font.render(f"Final Score: {self.score}", True, WHITE)
        self.screen.blit(final_score,
                        (SCREEN_WIDTH // 2 - final_score.get_width() // 2, 300))

        restart = self.font.render("Press ENTER to Restart", True, WHITE)
        self.screen.blit(restart,
                        (SCREEN_WIDTH // 2 - restart.get_width() // 2, 350))

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
