"""
Tower Defense - Strategic tower placement game.

Features:
- Grid-based tower placement
- Enemy waves with pathfinding (A*)
- Multiple tower types with upgrades
- Targeting AI (nearest, strongest, first)
- Resource management (gold)
- Wave progression system
"""

import os
import sys
import math
import heapq
from enum import Enum, auto
from dataclasses import dataclass, field
from typing import List, Optional, Callable, Tuple, Dict, Set

if "--headless" in sys.argv or os.environ.get("SDL_VIDEODRIVER") == "dummy":
    os.environ.setdefault("SDL_VIDEODRIVER", "dummy")
    os.environ.setdefault("SDL_AUDIODRIVER", "dummy")

import pygame

# Constants
SCREEN_WIDTH = 800
SCREEN_HEIGHT = 600
GRID_SIZE = 40
GRID_COLS = 15
GRID_ROWS = 12
GRID_OFFSET_X = 50
GRID_OFFSET_Y = 50
FPS = 60

# Colors
BLACK = (0, 0, 0)
WHITE = (255, 255, 255)
GRAY = (100, 100, 100)
DARK_GRAY = (50, 50, 50)
GREEN = (50, 200, 50)
RED = (200, 50, 50)
BLUE = (50, 130, 200)
YELLOW = (255, 220, 50)
ORANGE = (255, 150, 50)
PURPLE = (150, 50, 200)
CYAN = (50, 200, 200)
BROWN = (139, 90, 43)

# Tower costs and stats
TOWER_TYPES = {
    "basic": {
        "cost": 50,
        "damage": 10,
        "range": 100,
        "fire_rate": 60,  # frames between shots
        "color": BLUE,
        "upgrade_cost": 30,
    },
    "sniper": {
        "cost": 100,
        "damage": 50,
        "range": 200,
        "fire_rate": 120,
        "color": GREEN,
        "upgrade_cost": 50,
    },
    "rapid": {
        "cost": 75,
        "damage": 5,
        "range": 80,
        "fire_rate": 15,
        "color": ORANGE,
        "upgrade_cost": 40,
    },
    "splash": {
        "cost": 125,
        "damage": 20,
        "range": 100,
        "fire_rate": 90,
        "color": PURPLE,
        "upgrade_cost": 60,
        "splash_radius": 50,
    },
}

# Targeting modes
class TargetMode(Enum):
    FIRST = auto()    # First enemy in path
    NEAREST = auto()  # Nearest to tower
    STRONGEST = auto() # Highest HP
    WEAKEST = auto()  # Lowest HP


class GameState(Enum):
    MENU = auto()
    PLAYING = auto()
    PAUSED = auto()
    GAME_OVER = auto()
    VICTORY = auto()
    BUILDING = auto()  # Placing a tower


@dataclass
class Vector2:
    """2D vector for positions."""
    x: float = 0.0
    y: float = 0.0

    def __add__(self, other: "Vector2") -> "Vector2":
        return Vector2(self.x + other.x, self.y + other.y)

    def __sub__(self, other: "Vector2") -> "Vector2":
        return Vector2(self.x - other.x, self.y - other.y)

    def __mul__(self, scalar: float) -> "Vector2":
        return Vector2(self.x * scalar, self.y * scalar)

    def magnitude(self) -> float:
        return math.sqrt(self.x * self.x + self.y * self.y)

    def normalize(self) -> "Vector2":
        mag = self.magnitude()
        if mag == 0:
            return Vector2(0, 0)
        return Vector2(self.x / mag, self.y / mag)

    def distance_to(self, other: "Vector2") -> float:
        return (self - other).magnitude()


@dataclass
class GridPos:
    """Grid position."""
    x: int
    y: int

    def __hash__(self):
        return hash((self.x, self.y))

    def __eq__(self, other):
        if isinstance(other, GridPos):
            return self.x == other.x and self.y == other.y
        return False

    def __lt__(self, other):
        """Comparison for heapq tie-breaking."""
        if isinstance(other, GridPos):
            return (self.x, self.y) < (other.x, other.y)
        return NotImplemented

    def to_world(self) -> Vector2:
        """Convert to world coordinates (center of cell)."""
        return Vector2(
            GRID_OFFSET_X + self.x * GRID_SIZE + GRID_SIZE // 2,
            GRID_OFFSET_Y + self.y * GRID_SIZE + GRID_SIZE // 2
        )


class Projectile:
    """Projectile fired by towers."""

    def __init__(self, start: Vector2, target: "Enemy", damage: int,
                 speed: float = 8, splash_radius: int = 0):
        self.position = Vector2(start.x, start.y)
        self.target = target
        self.damage = damage
        self.speed = speed
        self.splash_radius = splash_radius
        self.active = True

    def update(self, enemies: List["Enemy"]) -> List["Enemy"]:
        """Update projectile. Returns list of hit enemies."""
        if not self.active:
            return []

        # Move toward target
        if self.target.health <= 0:
            # Target died, find new target or disappear
            self.active = False
            return []

        direction = Vector2(
            self.target.position.x - self.position.x,
            self.target.position.y - self.position.y
        ).normalize()

        self.position.x += direction.x * self.speed
        self.position.y += direction.y * self.speed

        # Check if reached target
        if self.position.distance_to(self.target.position) < 10:
            self.active = False
            hit_enemies = []

            if self.splash_radius > 0:
                # Splash damage
                for enemy in enemies:
                    if enemy.position.distance_to(self.position) < self.splash_radius:
                        enemy.take_damage(self.damage)
                        hit_enemies.append(enemy)
            else:
                # Single target
                self.target.take_damage(self.damage)
                hit_enemies.append(self.target)

            return hit_enemies

        return []

    def draw(self, screen: pygame.Surface):
        """Draw the projectile."""
        if self.active:
            color = ORANGE if self.splash_radius > 0 else YELLOW
            pygame.draw.circle(
                screen, color,
                (int(self.position.x), int(self.position.y)), 4
            )


class Tower:
    """Defense tower."""

    def __init__(self, grid_pos: GridPos, tower_type: str):
        self.grid_pos = grid_pos
        self.position = grid_pos.to_world()
        self.tower_type = tower_type
        self.level = 1
        self.cooldown = 0
        self.target_mode = TargetMode.FIRST

        stats = TOWER_TYPES[tower_type]
        self.damage = stats["damage"]
        self.range = stats["range"]
        self.fire_rate = stats["fire_rate"]
        self.color = stats["color"]
        self.splash_radius = stats.get("splash_radius", 0)

    def upgrade(self) -> int:
        """Upgrade tower. Returns cost."""
        cost = TOWER_TYPES[self.tower_type]["upgrade_cost"] * self.level
        self.level += 1
        self.damage = int(self.damage * 1.5)
        self.range = int(self.range * 1.1)
        self.fire_rate = max(5, int(self.fire_rate * 0.9))
        return cost

    def get_upgrade_cost(self) -> int:
        """Get cost for next upgrade."""
        return TOWER_TYPES[self.tower_type]["upgrade_cost"] * self.level

    def can_hit(self, enemy: "Enemy") -> bool:
        """Check if enemy is in range."""
        return self.position.distance_to(enemy.position) <= self.range

    def select_target(self, enemies: List["Enemy"], path: List[GridPos]) -> Optional["Enemy"]:
        """Select target based on targeting mode."""
        in_range = [e for e in enemies if self.can_hit(e) and e.health > 0]
        if not in_range:
            return None

        if self.target_mode == TargetMode.NEAREST:
            return min(in_range, key=lambda e: self.position.distance_to(e.position))
        elif self.target_mode == TargetMode.STRONGEST:
            return max(in_range, key=lambda e: e.health)
        elif self.target_mode == TargetMode.WEAKEST:
            return min(in_range, key=lambda e: e.health)
        else:  # FIRST
            # Find enemy furthest along path
            return max(in_range, key=lambda e: e.path_index)

    def update(self, enemies: List["Enemy"], path: List[GridPos]) -> Optional[Projectile]:
        """Update tower. Returns projectile if fired."""
        if self.cooldown > 0:
            self.cooldown -= 1
            return None

        target = self.select_target(enemies, path)
        if target:
            self.cooldown = self.fire_rate
            return Projectile(
                Vector2(self.position.x, self.position.y),
                target,
                self.damage,
                splash_radius=self.splash_radius
            )
        return None

    def draw(self, screen: pygame.Surface, selected: bool = False):
        """Draw the tower."""
        x, y = int(self.position.x), int(self.position.y)
        size = GRID_SIZE // 2 - 2

        # Draw base
        pygame.draw.rect(
            screen, self.color,
            (x - size, y - size, size * 2, size * 2)
        )
        pygame.draw.rect(
            screen, WHITE,
            (x - size, y - size, size * 2, size * 2), 2
        )

        # Draw level indicator
        if self.level > 1:
            for i in range(min(self.level - 1, 3)):
                pygame.draw.circle(
                    screen, YELLOW,
                    (x - size + 5 + i * 8, y - size + 5), 3
                )

        # Draw range when selected
        if selected:
            pygame.draw.circle(
                screen, (*self.color, 50),
                (x, y), int(self.range), 1
            )


class Enemy:
    """Enemy that follows path."""

    def __init__(self, path: List[GridPos], health: int, speed: float,
                 reward: int, enemy_type: str = "basic"):
        self.path = path
        self.path_index = 0
        self.health = health
        self.max_health = health
        self.speed = speed
        self.reward = reward
        self.enemy_type = enemy_type

        # Start at first path point
        start = path[0].to_world()
        self.position = Vector2(start.x, start.y)

    def take_damage(self, damage: int):
        """Take damage from tower."""
        self.health -= damage

    def update(self) -> bool:
        """Update enemy position. Returns True if reached end."""
        if self.path_index >= len(self.path) - 1:
            return True

        target = self.path[self.path_index + 1].to_world()
        direction = Vector2(
            target.x - self.position.x,
            target.y - self.position.y
        )

        dist = direction.magnitude()
        if dist < self.speed:
            self.path_index += 1
            if self.path_index >= len(self.path) - 1:
                return True
        else:
            direction = direction.normalize()
            self.position.x += direction.x * self.speed
            self.position.y += direction.y * self.speed

        return False

    def draw(self, screen: pygame.Surface):
        """Draw the enemy."""
        x, y = int(self.position.x), int(self.position.y)

        # Different colors for enemy types
        colors = {
            "basic": RED,
            "fast": ORANGE,
            "tank": PURPLE,
            "boss": (150, 0, 0),
        }
        color = colors.get(self.enemy_type, RED)

        # Size based on type
        size = 12 if self.enemy_type != "boss" else 18

        pygame.draw.circle(screen, color, (x, y), size)
        pygame.draw.circle(screen, WHITE, (x, y), size, 2)

        # Health bar
        bar_width = size * 2
        bar_height = 4
        health_pct = self.health / self.max_health
        pygame.draw.rect(
            screen, DARK_GRAY,
            (x - bar_width // 2, y - size - 8, bar_width, bar_height)
        )
        pygame.draw.rect(
            screen, GREEN if health_pct > 0.5 else (YELLOW if health_pct > 0.25 else RED),
            (x - bar_width // 2, y - size - 8, int(bar_width * health_pct), bar_height)
        )


class Grid:
    """Game grid for tower placement and pathfinding."""

    def __init__(self, cols: int, rows: int):
        self.cols = cols
        self.rows = rows
        self.cells: List[List[int]] = [
            [0 for _ in range(cols)] for _ in range(rows)
        ]
        # 0 = empty, 1 = path, 2 = tower, 3 = blocked

    def is_valid(self, pos: GridPos) -> bool:
        """Check if position is within grid."""
        return 0 <= pos.x < self.cols and 0 <= pos.y < self.rows

    def is_buildable(self, pos: GridPos) -> bool:
        """Check if tower can be placed."""
        if not self.is_valid(pos):
            return False
        return self.cells[pos.y][pos.x] == 0

    def set_cell(self, pos: GridPos, value: int):
        """Set cell value."""
        if self.is_valid(pos):
            self.cells[pos.y][pos.x] = value

    def get_cell(self, pos: GridPos) -> int:
        """Get cell value."""
        if self.is_valid(pos):
            return self.cells[pos.y][pos.x]
        return 3  # Blocked

    def find_path(self, start: GridPos, end: GridPos) -> List[GridPos]:
        """Find path using A* algorithm."""
        if not self.is_valid(start) or not self.is_valid(end):
            return []

        def heuristic(a: GridPos, b: GridPos) -> float:
            return abs(a.x - b.x) + abs(a.y - b.y)

        open_set = [(0, start)]
        came_from: Dict[GridPos, GridPos] = {}
        g_score: Dict[GridPos, float] = {start: 0}
        f_score: Dict[GridPos, float] = {start: heuristic(start, end)}

        while open_set:
            _, current = heapq.heappop(open_set)

            if current == end:
                # Reconstruct path
                path = [current]
                while current in came_from:
                    current = came_from[current]
                    path.append(current)
                return path[::-1]

            for dx, dy in [(0, 1), (1, 0), (0, -1), (-1, 0)]:
                neighbor = GridPos(current.x + dx, current.y + dy)

                if not self.is_valid(neighbor):
                    continue

                cell = self.cells[neighbor.y][neighbor.x]
                if cell == 2 or cell == 3:  # Tower or blocked
                    continue

                tentative_g = g_score[current] + 1

                if neighbor not in g_score or tentative_g < g_score[neighbor]:
                    came_from[neighbor] = current
                    g_score[neighbor] = tentative_g
                    f_score[neighbor] = tentative_g + heuristic(neighbor, end)
                    heapq.heappush(open_set, (f_score[neighbor], neighbor))

        return []  # No path found

    def draw(self, screen: pygame.Surface, path: List[GridPos]):
        """Draw the grid."""
        for y in range(self.rows):
            for x in range(self.cols):
                rect = pygame.Rect(
                    GRID_OFFSET_X + x * GRID_SIZE,
                    GRID_OFFSET_Y + y * GRID_SIZE,
                    GRID_SIZE, GRID_SIZE
                )

                cell = self.cells[y][x]
                if GridPos(x, y) in path:
                    color = BROWN
                elif cell == 0:
                    color = DARK_GRAY
                elif cell == 3:
                    color = (30, 30, 30)
                else:
                    color = DARK_GRAY

                pygame.draw.rect(screen, color, rect)
                pygame.draw.rect(screen, GRAY, rect, 1)


class WaveManager:
    """Manages enemy waves."""

    def __init__(self):
        self.wave = 0
        self.enemies_to_spawn: List[Tuple[str, int, float, int]] = []
        self.spawn_timer = 0
        self.spawn_delay = 30
        self.wave_delay = 180
        self.wave_timer = 0
        self.wave_active = False

    def start_wave(self, wave: int):
        """Start a new wave."""
        self.wave = wave
        self.wave_active = True
        self.enemies_to_spawn = self._generate_wave(wave)
        self.spawn_timer = 0

    def _generate_wave(self, wave: int) -> List[Tuple[str, int, float, int]]:
        """Generate enemies for wave. Returns [(type, health, speed, reward), ...]"""
        enemies = []

        # Basic enemies
        basic_count = 5 + wave * 2
        basic_health = 50 + wave * 10
        for _ in range(basic_count):
            enemies.append(("basic", basic_health, 1.5, 10))

        # Fast enemies (from wave 3)
        if wave >= 3:
            fast_count = wave - 2
            fast_health = 30 + wave * 5
            for _ in range(fast_count):
                enemies.append(("fast", fast_health, 3.0, 15))

        # Tank enemies (from wave 5)
        if wave >= 5:
            tank_count = (wave - 4) // 2 + 1
            tank_health = 200 + wave * 30
            for _ in range(tank_count):
                enemies.append(("tank", tank_health, 0.8, 30))

        # Boss every 5 waves
        if wave % 5 == 0:
            boss_health = 500 + wave * 100
            enemies.append(("boss", boss_health, 0.5, 100))

        return enemies

    def update(self, path: List[GridPos]) -> Optional[Enemy]:
        """Update wave. Returns new enemy if spawning."""
        if not self.wave_active:
            return None

        if not self.enemies_to_spawn:
            self.wave_active = False
            return None

        self.spawn_timer += 1
        if self.spawn_timer >= self.spawn_delay:
            self.spawn_timer = 0
            enemy_data = self.enemies_to_spawn.pop(0)
            enemy_type, health, speed, reward = enemy_data
            return Enemy(path, health, speed, reward, enemy_type)

        return None

    def is_wave_complete(self, enemies: List[Enemy]) -> bool:
        """Check if wave is complete."""
        return not self.wave_active and len(enemies) == 0


class Game:
    """Main tower defense game class."""

    def __init__(self, headless: bool = False):
        pygame.init()
        self.headless = headless

        if headless:
            self.screen = pygame.Surface((SCREEN_WIDTH, SCREEN_HEIGHT))
        else:
            self.screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))
            pygame.display.set_caption("Tower Defense")

        self.clock = pygame.time.Clock()
        self.font = pygame.font.Font(None, 36)
        self.small_font = pygame.font.Font(None, 24)

        # Callbacks
        self.on_score: Optional[Callable[[int], None]] = None
        self.on_state_change: Optional[Callable[[GameState], None]] = None
        self.on_wave_complete: Optional[Callable[[int], None]] = None
        self.on_enemy_killed: Optional[Callable[["Enemy"], None]] = None

        self.reset()

    def reset(self):
        """Reset game to initial state."""
        self.state = GameState.MENU
        self.gold = 200
        self.lives = 20
        self.score = 0
        self.selected_tower_type: Optional[str] = None
        self.selected_tower: Optional[Tower] = None
        self.injected_input: dict = {}

        self._create_level()

    def _create_level(self):
        """Create the game level."""
        self.grid = Grid(GRID_COLS, GRID_ROWS)

        # Define start and end points
        self.start_pos = GridPos(0, 5)
        self.end_pos = GridPos(GRID_COLS - 1, 5)

        # Create initial path
        self.path = self.grid.find_path(self.start_pos, self.end_pos)
        for pos in self.path:
            self.grid.set_cell(pos, 1)

        # Game objects
        self.towers: List[Tower] = []
        self.enemies: List[Enemy] = []
        self.projectiles: List[Projectile] = []

        self.wave_manager = WaveManager()

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

    def _get_grid_pos(self, screen_x: int, screen_y: int) -> Optional[GridPos]:
        """Convert screen position to grid position."""
        gx = (screen_x - GRID_OFFSET_X) // GRID_SIZE
        gy = (screen_y - GRID_OFFSET_Y) // GRID_SIZE
        pos = GridPos(gx, gy)
        if self.grid.is_valid(pos):
            return pos
        return None

    def place_tower(self, pos: GridPos, tower_type: str) -> bool:
        """Place a tower at grid position."""
        cost = TOWER_TYPES[tower_type]["cost"]
        if self.gold < cost:
            return False
        if not self.grid.is_buildable(pos):
            return False

        # Check if placing would block path
        self.grid.set_cell(pos, 2)
        new_path = self.grid.find_path(self.start_pos, self.end_pos)
        if not new_path:
            self.grid.set_cell(pos, 0)  # Revert
            return False

        self.gold -= cost
        tower = Tower(pos, tower_type)
        self.towers.append(tower)
        self.path = new_path
        return True

    def upgrade_tower(self, tower: Tower) -> bool:
        """Upgrade a tower."""
        cost = tower.get_upgrade_cost()
        if self.gold < cost:
            return False
        self.gold -= tower.upgrade()
        return True

    def sell_tower(self, tower: Tower) -> int:
        """Sell a tower. Returns gold gained."""
        self.towers.remove(tower)
        self.grid.set_cell(tower.grid_pos, 0)
        self.path = self.grid.find_path(self.start_pos, self.end_pos)
        refund = TOWER_TYPES[tower.tower_type]["cost"] // 2
        self.gold += refund
        return refund

    def start_wave(self):
        """Start next wave."""
        self.wave_manager.start_wave(self.wave_manager.wave + 1)

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
                    elif self.state == GameState.BUILDING:
                        self.selected_tower_type = None
                        self.set_state(GameState.PLAYING)
                elif event.key == pygame.K_RETURN:
                    if self.state in (GameState.MENU, GameState.GAME_OVER, GameState.VICTORY):
                        self.reset()
                        self.set_state(GameState.PLAYING)
                elif self.state == GameState.PLAYING:
                    self._handle_game_key(event.key)
            elif event.type == pygame.MOUSEBUTTONDOWN:
                if self.state in (GameState.PLAYING, GameState.BUILDING):
                    self._handle_click(event.pos, event.button)
        return True

    def _handle_game_key(self, key):
        """Handle key during gameplay."""
        # Tower hotkeys
        tower_keys = {
            pygame.K_1: "basic",
            pygame.K_2: "sniper",
            pygame.K_3: "rapid",
            pygame.K_4: "splash",
        }
        if key in tower_keys:
            self.selected_tower_type = tower_keys[key]
            self.selected_tower = None
            self.set_state(GameState.BUILDING)
        elif key == pygame.K_SPACE:
            if not self.wave_manager.wave_active:
                self.start_wave()
        elif key == pygame.K_u:
            if self.selected_tower:
                self.upgrade_tower(self.selected_tower)
        elif key == pygame.K_s:
            if self.selected_tower:
                self.sell_tower(self.selected_tower)
                self.selected_tower = None
        elif key == pygame.K_t:
            if self.selected_tower:
                # Cycle target mode
                modes = list(TargetMode)
                idx = modes.index(self.selected_tower.target_mode)
                self.selected_tower.target_mode = modes[(idx + 1) % len(modes)]

    def _handle_click(self, pos: Tuple[int, int], button: int):
        """Handle mouse click."""
        grid_pos = self._get_grid_pos(pos[0], pos[1])

        if self.state == GameState.BUILDING and grid_pos:
            if button == 1 and self.selected_tower_type:  # Left click
                if self.place_tower(grid_pos, self.selected_tower_type):
                    self.selected_tower_type = None
                    self.set_state(GameState.PLAYING)

        elif self.state == GameState.PLAYING and grid_pos:
            if button == 1:  # Left click
                # Select tower
                self.selected_tower = None
                for tower in self.towers:
                    if tower.grid_pos == grid_pos:
                        self.selected_tower = tower
                        break

    def _update_playing(self):
        """Update game during PLAYING state."""
        # Handle injected input
        if self.injected_input:
            inp = self.injected_input
            if inp.get("start_wave"):
                self.start_wave()
            if inp.get("place_tower"):
                pos, tower_type = inp["place_tower"]
                self.place_tower(pos, tower_type)

        # Spawn enemies
        new_enemy = self.wave_manager.update(self.path)
        if new_enemy:
            self.enemies.append(new_enemy)

        # Update enemies
        enemies_to_remove = []
        for enemy in self.enemies:
            if enemy.health <= 0:
                self.gold += enemy.reward
                self.add_score(enemy.reward)
                if self.on_enemy_killed:
                    self.on_enemy_killed(enemy)
                enemies_to_remove.append(enemy)
            elif enemy.update():  # Reached end
                self.lives -= 1
                enemies_to_remove.append(enemy)
                if self.lives <= 0:
                    self.set_state(GameState.GAME_OVER)
                    return

        for enemy in enemies_to_remove:
            if enemy in self.enemies:
                self.enemies.remove(enemy)

        # Update towers
        for tower in self.towers:
            projectile = tower.update(self.enemies, self.path)
            if projectile:
                self.projectiles.append(projectile)

        # Update projectiles
        projectiles_to_remove = []
        for projectile in self.projectiles:
            projectile.update(self.enemies)
            if not projectile.active:
                projectiles_to_remove.append(projectile)

        for proj in projectiles_to_remove:
            self.projectiles.remove(proj)

        # Check wave complete
        if self.wave_manager.is_wave_complete(self.enemies):
            if self.on_wave_complete:
                self.on_wave_complete(self.wave_manager.wave)
            # Victory after wave 10
            if self.wave_manager.wave >= 10:
                self.set_state(GameState.VICTORY)

    def step(self) -> bool:
        """Execute one game frame. Returns False to quit."""
        if not self._handle_events():
            return False

        if self.state in (GameState.PLAYING, GameState.BUILDING):
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
        elif self.state in (GameState.PLAYING, GameState.BUILDING):
            self._draw_game()
            self._draw_hud()
            if self.state == GameState.BUILDING:
                self._draw_building_preview()
        elif self.state == GameState.PAUSED:
            self._draw_game()
            self._draw_hud()
            self._draw_paused()
        elif self.state == GameState.GAME_OVER:
            self._draw_game()
            self._draw_game_over()
        elif self.state == GameState.VICTORY:
            self._draw_game()
            self._draw_victory()

        if not self.headless:
            pygame.display.flip()

    def _draw_menu(self):
        """Draw menu screen."""
        title = self.font.render("TOWER DEFENSE", True, WHITE)
        self.screen.blit(title,
                        (SCREEN_WIDTH // 2 - title.get_width() // 2, 200))

        start = self.font.render("Press ENTER to Start", True, WHITE)
        self.screen.blit(start,
                        (SCREEN_WIDTH // 2 - start.get_width() // 2, 300))

        controls = self.small_font.render("1-4: Towers | Space: Start Wave | Click: Place/Select", True, GRAY)
        self.screen.blit(controls,
                        (SCREEN_WIDTH // 2 - controls.get_width() // 2, 350))

    def _draw_game(self):
        """Draw game entities."""
        self.grid.draw(self.screen, self.path)

        # Draw path direction
        for i in range(len(self.path) - 1):
            p1 = self.path[i].to_world()
            p2 = self.path[i + 1].to_world()
            pygame.draw.line(
                self.screen, (80, 60, 40),
                (int(p1.x), int(p1.y)),
                (int(p2.x), int(p2.y)), 3
            )

        for tower in self.towers:
            tower.draw(self.screen, tower == self.selected_tower)

        for enemy in self.enemies:
            enemy.draw(self.screen)

        for projectile in self.projectiles:
            projectile.draw(self.screen)

        # Draw start/end markers
        start_world = self.start_pos.to_world()
        end_world = self.end_pos.to_world()
        pygame.draw.circle(self.screen, GREEN,
                          (int(start_world.x), int(start_world.y)), 15, 3)
        pygame.draw.circle(self.screen, RED,
                          (int(end_world.x), int(end_world.y)), 15, 3)

    def _draw_hud(self):
        """Draw HUD elements."""
        # Gold
        gold_text = self.font.render(f"Gold: {self.gold}", True, YELLOW)
        self.screen.blit(gold_text, (10, 10))

        # Lives
        lives_text = self.font.render(f"Lives: {self.lives}", True, RED)
        self.screen.blit(lives_text, (150, 10))

        # Wave
        wave_text = self.font.render(f"Wave: {self.wave_manager.wave}/10", True, WHITE)
        self.screen.blit(wave_text, (300, 10))

        # Score
        score_text = self.font.render(f"Score: {self.score}", True, WHITE)
        self.screen.blit(score_text, (SCREEN_WIDTH - 150, 10))

        # Tower info panel
        if self.selected_tower:
            self._draw_tower_info()

        # Wave start hint
        if not self.wave_manager.wave_active:
            hint = self.small_font.render("Press SPACE to start wave", True, CYAN)
            self.screen.blit(hint, (SCREEN_WIDTH // 2 - hint.get_width() // 2,
                                   SCREEN_HEIGHT - 30))

    def _draw_tower_info(self):
        """Draw selected tower info."""
        tower = self.selected_tower
        if not tower:
            return

        panel_x = SCREEN_WIDTH - 180
        panel_y = 50

        pygame.draw.rect(self.screen, DARK_GRAY,
                        (panel_x, panel_y, 170, 150))
        pygame.draw.rect(self.screen, WHITE,
                        (panel_x, panel_y, 170, 150), 2)

        info = [
            f"{tower.tower_type.title()} Lv.{tower.level}",
            f"Damage: {tower.damage}",
            f"Range: {tower.range}",
            f"Target: {tower.target_mode.name}",
            f"Upgrade: {tower.get_upgrade_cost()}g (U)",
            f"Sell (S)",
            f"Target Mode (T)",
        ]

        for i, text in enumerate(info):
            color = YELLOW if i == 0 else WHITE
            rendered = self.small_font.render(text, True, color)
            self.screen.blit(rendered, (panel_x + 10, panel_y + 10 + i * 20))

    def _draw_building_preview(self):
        """Draw tower placement preview."""
        mouse_pos = pygame.mouse.get_pos()
        grid_pos = self._get_grid_pos(mouse_pos[0], mouse_pos[1])

        if grid_pos and self.selected_tower_type:
            world_pos = grid_pos.to_world()
            can_build = self.grid.is_buildable(grid_pos)

            color = GREEN if can_build else RED
            size = GRID_SIZE // 2 - 2
            pygame.draw.rect(
                self.screen, color,
                (int(world_pos.x) - size, int(world_pos.y) - size,
                 size * 2, size * 2), 2
            )

            # Show range
            tower_stats = TOWER_TYPES[self.selected_tower_type]
            pygame.draw.circle(
                self.screen, color,
                (int(world_pos.x), int(world_pos.y)),
                tower_stats["range"], 1
            )

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

    def _draw_victory(self):
        """Draw victory overlay."""
        overlay = pygame.Surface((SCREEN_WIDTH, SCREEN_HEIGHT))
        overlay.fill(BLACK)
        overlay.set_alpha(128)
        self.screen.blit(overlay, (0, 0))

        victory = self.font.render("VICTORY!", True, GREEN)
        self.screen.blit(victory,
                        (SCREEN_WIDTH // 2 - victory.get_width() // 2, 250))

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
