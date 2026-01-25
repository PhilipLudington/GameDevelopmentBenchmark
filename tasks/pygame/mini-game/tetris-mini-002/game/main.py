"""
Tetris - Classic falling block puzzle game.

Features:
- 7 standard tetrominoes with SRS rotation
- Line clearing with cascade animation
- Ghost piece preview
- Hold piece mechanic
- Scoring with combos and level progression
- Hard drop and soft drop
"""

import os
import sys
import random
from enum import Enum, auto
from dataclasses import dataclass, field
from typing import List, Optional, Callable, Tuple, Set

if "--headless" in sys.argv or os.environ.get("SDL_VIDEODRIVER") == "dummy":
    os.environ.setdefault("SDL_VIDEODRIVER", "dummy")
    os.environ.setdefault("SDL_AUDIODRIVER", "dummy")

import pygame

# Constants
SCREEN_WIDTH = 500
SCREEN_HEIGHT = 600
GRID_WIDTH = 10
GRID_HEIGHT = 20
CELL_SIZE = 28
GRID_OFFSET_X = 50
GRID_OFFSET_Y = 30
FPS = 60

# Colors
BLACK = (0, 0, 0)
WHITE = (255, 255, 255)
GRAY = (128, 128, 128)
DARK_GRAY = (50, 50, 50)

# Piece colors (Tetris Guideline colors)
PIECE_COLORS = {
    'I': (0, 255, 255),    # Cyan
    'O': (255, 255, 0),    # Yellow
    'T': (128, 0, 128),    # Purple
    'S': (0, 255, 0),      # Green
    'Z': (255, 0, 0),      # Red
    'J': (0, 0, 255),      # Blue
    'L': (255, 165, 0),    # Orange
}

# Tetromino shapes (each rotation state)
TETROMINOES = {
    'I': [
        [(0, 1), (1, 1), (2, 1), (3, 1)],
        [(2, 0), (2, 1), (2, 2), (2, 3)],
        [(0, 2), (1, 2), (2, 2), (3, 2)],
        [(1, 0), (1, 1), (1, 2), (1, 3)],
    ],
    'O': [
        [(1, 0), (2, 0), (1, 1), (2, 1)],
        [(1, 0), (2, 0), (1, 1), (2, 1)],
        [(1, 0), (2, 0), (1, 1), (2, 1)],
        [(1, 0), (2, 0), (1, 1), (2, 1)],
    ],
    'T': [
        [(1, 0), (0, 1), (1, 1), (2, 1)],
        [(1, 0), (1, 1), (2, 1), (1, 2)],
        [(0, 1), (1, 1), (2, 1), (1, 2)],
        [(1, 0), (0, 1), (1, 1), (1, 2)],
    ],
    'S': [
        [(1, 0), (2, 0), (0, 1), (1, 1)],
        [(1, 0), (1, 1), (2, 1), (2, 2)],
        [(1, 1), (2, 1), (0, 2), (1, 2)],
        [(0, 0), (0, 1), (1, 1), (1, 2)],
    ],
    'Z': [
        [(0, 0), (1, 0), (1, 1), (2, 1)],
        [(2, 0), (1, 1), (2, 1), (1, 2)],
        [(0, 1), (1, 1), (1, 2), (2, 2)],
        [(1, 0), (0, 1), (1, 1), (0, 2)],
    ],
    'J': [
        [(0, 0), (0, 1), (1, 1), (2, 1)],
        [(1, 0), (2, 0), (1, 1), (1, 2)],
        [(0, 1), (1, 1), (2, 1), (2, 2)],
        [(1, 0), (1, 1), (0, 2), (1, 2)],
    ],
    'L': [
        [(2, 0), (0, 1), (1, 1), (2, 1)],
        [(1, 0), (1, 1), (1, 2), (2, 2)],
        [(0, 1), (1, 1), (2, 1), (0, 2)],
        [(0, 0), (1, 0), (1, 1), (1, 2)],
    ],
}

# SRS wall kick data
WALL_KICKS = {
    'default': [
        [(0, 0), (-1, 0), (-1, -1), (0, 2), (-1, 2)],   # 0->1
        [(0, 0), (1, 0), (1, 1), (0, -2), (1, -2)],     # 1->2
        [(0, 0), (1, 0), (1, -1), (0, 2), (1, 2)],      # 2->3
        [(0, 0), (-1, 0), (-1, 1), (0, -2), (-1, -2)],  # 3->0
    ],
    'I': [
        [(0, 0), (-2, 0), (1, 0), (-2, 1), (1, -2)],
        [(0, 0), (-1, 0), (2, 0), (-1, -2), (2, 1)],
        [(0, 0), (2, 0), (-1, 0), (2, -1), (-1, 2)],
        [(0, 0), (1, 0), (-2, 0), (1, 2), (-2, -1)],
    ],
}

# Scoring
SCORE_SINGLE = 100
SCORE_DOUBLE = 300
SCORE_TRIPLE = 500
SCORE_TETRIS = 800
SCORE_SOFT_DROP = 1
SCORE_HARD_DROP = 2

# Timing
LOCK_DELAY = 30  # frames
DAS_DELAY = 10   # frames before auto-repeat
DAS_RATE = 2     # frames between auto-repeat


class GameState(Enum):
    MENU = auto()
    PLAYING = auto()
    PAUSED = auto()
    GAME_OVER = auto()


@dataclass
class Position:
    """Grid position."""
    x: int
    y: int

    def __add__(self, other: "Position") -> "Position":
        return Position(self.x + other.x, self.y + other.y)

    def to_tuple(self) -> Tuple[int, int]:
        return (self.x, self.y)


class Piece:
    """A tetromino piece."""

    def __init__(self, piece_type: str, x: int = 3, y: int = 0):
        self.type = piece_type
        self.x = x
        self.y = y
        self.rotation = 0
        self.color = PIECE_COLORS[piece_type]

    def get_cells(self) -> List[Tuple[int, int]]:
        """Get absolute cell positions for current rotation."""
        shape = TETROMINOES[self.type][self.rotation]
        return [(self.x + dx, self.y + dy) for dx, dy in shape]

    def get_ghost_y(self, grid: List[List[Optional[str]]]) -> int:
        """Get Y position for ghost piece (hard drop position)."""
        original_y = self.y
        while self._can_move(grid, 0, 1):
            self.y += 1
        ghost_y = self.y
        self.y = original_y
        return ghost_y

    def _can_move(self, grid: List[List[Optional[str]]], dx: int, dy: int) -> bool:
        """Check if piece can move by offset."""
        for cx, cy in self.get_cells():
            nx, ny = cx + dx, cy + dy
            if nx < 0 or nx >= GRID_WIDTH or ny >= GRID_HEIGHT:
                return False
            if ny >= 0 and grid[ny][nx] is not None:
                return False
        return True

    def move(self, grid: List[List[Optional[str]]], dx: int, dy: int) -> bool:
        """Move piece by offset. Returns True if successful."""
        if self._can_move(grid, dx, dy):
            self.x += dx
            self.y += dy
            return True
        return False

    def rotate(self, grid: List[List[Optional[str]]], clockwise: bool = True) -> bool:
        """Rotate piece with wall kicks. Returns True if successful."""
        old_rotation = self.rotation
        if clockwise:
            self.rotation = (self.rotation + 1) % 4
        else:
            self.rotation = (self.rotation - 1) % 4

        # Get wall kick data
        kick_data = WALL_KICKS.get(self.type, WALL_KICKS['default'])
        kick_index = old_rotation if clockwise else self.rotation

        # Try each wall kick
        for dx, dy in kick_data[kick_index]:
            if clockwise:
                test_dx, test_dy = dx, -dy
            else:
                test_dx, test_dy = -dx, dy

            if self._can_move(grid, test_dx, test_dy):
                self.x += test_dx
                self.y += test_dy
                return True

        # Rotation failed, revert
        self.rotation = old_rotation
        return False

    def copy(self) -> "Piece":
        """Create a copy of this piece."""
        p = Piece(self.type, self.x, self.y)
        p.rotation = self.rotation
        return p


class Game:
    """Main Tetris game class."""

    def __init__(self, headless: bool = False):
        pygame.init()
        self.headless = headless

        if headless:
            self.screen = pygame.Surface((SCREEN_WIDTH, SCREEN_HEIGHT))
        else:
            self.screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))
            pygame.display.set_caption("Tetris")

        self.clock = pygame.time.Clock()
        self.font = pygame.font.Font(None, 36)
        self.small_font = pygame.font.Font(None, 24)

        # Callbacks
        self.on_score: Optional[Callable[[int], None]] = None
        self.on_state_change: Optional[Callable[[GameState], None]] = None
        self.on_line_clear: Optional[Callable[[int], None]] = None

        self.reset()

    def reset(self):
        """Reset game to initial state."""
        self.state = GameState.MENU
        self.grid: List[List[Optional[str]]] = [
            [None for _ in range(GRID_WIDTH)]
            for _ in range(GRID_HEIGHT)
        ]
        self.score = 0
        self.level = 1
        self.lines_cleared = 0
        self.combo = 0

        # Piece management
        self.bag = []
        self.current_piece: Optional[Piece] = None
        self.held_piece: Optional[Piece] = None
        self.can_hold = True
        self.next_pieces: List[Piece] = []

        # Timing
        self.drop_timer = 0
        self.lock_timer = 0
        self.lock_moves = 0

        # Input handling
        self.das_timer = 0
        self.das_direction = 0
        self.injected_input: dict = {}
        self.last_input: dict = {}

        # Fill bag and get pieces
        self._fill_bag()
        for _ in range(3):
            self.next_pieces.append(self._get_next_piece())

    def _fill_bag(self):
        """Fill piece bag with randomized tetromino set."""
        pieces = list(TETROMINOES.keys())
        random.shuffle(pieces)
        self.bag.extend(pieces)

    def _get_next_piece(self) -> Piece:
        """Get next piece from bag."""
        if len(self.bag) < 7:
            self._fill_bag()
        return Piece(self.bag.pop(0))

    def _spawn_piece(self):
        """Spawn a new piece at top of grid."""
        self.current_piece = self.next_pieces.pop(0)
        self.next_pieces.append(self._get_next_piece())
        self.can_hold = True
        self.lock_timer = 0
        self.lock_moves = 0

        # Check for game over
        if not self._is_valid_position():
            self.set_state(GameState.GAME_OVER)

    def _is_valid_position(self) -> bool:
        """Check if current piece position is valid."""
        if not self.current_piece:
            return True
        for cx, cy in self.current_piece.get_cells():
            if cx < 0 or cx >= GRID_WIDTH or cy >= GRID_HEIGHT:
                return False
            if cy >= 0 and self.grid[cy][cx] is not None:
                return False
        return True

    def set_state(self, state: GameState):
        """Change game state."""
        old_state = self.state
        self.state = state
        if self.on_state_change and old_state != state:
            self.on_state_change(state)

    def add_score(self, points: int):
        """Add points to score."""
        self.score += points * self.level
        if self.on_score:
            self.on_score(points * self.level)

    def inject_input(self, inputs: dict):
        """Inject input for testing."""
        self.injected_input = inputs

    def _get_input(self) -> dict:
        """Get current input state."""
        if self.injected_input:
            return self.injected_input

        keys = pygame.key.get_pressed()
        return {
            "left": keys[pygame.K_LEFT],
            "right": keys[pygame.K_RIGHT],
            "down": keys[pygame.K_DOWN],
            "rotate_cw": keys[pygame.K_UP] or keys[pygame.K_x],
            "rotate_ccw": keys[pygame.K_z],
            "hard_drop": keys[pygame.K_SPACE],
            "hold": keys[pygame.K_c] or keys[pygame.K_LSHIFT],
        }

    def hold_piece(self):
        """Hold current piece."""
        if not self.can_hold or not self.current_piece:
            return

        self.can_hold = False
        current_type = self.current_piece.type

        if self.held_piece:
            # Swap with held piece
            self.current_piece = Piece(self.held_piece.type)
        else:
            # Get next piece
            self.current_piece = self.next_pieces.pop(0)
            self.next_pieces.append(self._get_next_piece())

        self.held_piece = Piece(current_type)
        self.lock_timer = 0
        self.lock_moves = 0

    def _get_drop_speed(self) -> int:
        """Get frames per drop for current level."""
        # Speed increases with level
        speeds = [48, 43, 38, 33, 28, 23, 18, 13, 8, 6, 5, 5, 5, 4, 4, 4, 3, 3, 3, 2]
        index = min(self.level - 1, len(speeds) - 1)
        return speeds[index]

    def _lock_piece(self):
        """Lock current piece into grid."""
        if not self.current_piece:
            return

        for cx, cy in self.current_piece.get_cells():
            if 0 <= cy < GRID_HEIGHT:
                self.grid[cy][cx] = self.current_piece.type

        # Check for line clears
        lines = self._clear_lines()
        if lines > 0:
            self.combo += 1
            self._score_lines(lines)
            if self.on_line_clear:
                self.on_line_clear(lines)
        else:
            self.combo = 0

        self.current_piece = None
        self._spawn_piece()

    def _clear_lines(self) -> int:
        """Clear completed lines. Returns number cleared."""
        lines_to_clear = []
        for y in range(GRID_HEIGHT):
            if all(self.grid[y][x] is not None for x in range(GRID_WIDTH)):
                lines_to_clear.append(y)

        # Clear lines and shift down
        for y in lines_to_clear:
            del self.grid[y]
            self.grid.insert(0, [None for _ in range(GRID_WIDTH)])

        self.lines_cleared += len(lines_to_clear)

        # Level up every 10 lines
        new_level = (self.lines_cleared // 10) + 1
        if new_level > self.level:
            self.level = new_level

        return len(lines_to_clear)

    def _score_lines(self, lines: int):
        """Score line clears."""
        base_scores = {1: SCORE_SINGLE, 2: SCORE_DOUBLE, 3: SCORE_TRIPLE, 4: SCORE_TETRIS}
        base = base_scores.get(lines, SCORE_TETRIS)
        combo_bonus = 50 * self.combo
        self.add_score(base + combo_bonus)

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
                        self._spawn_piece()
                elif self.state == GameState.PLAYING:
                    self._handle_game_keydown(event.key)
        return True

    def _handle_game_keydown(self, key):
        """Handle key press during gameplay."""
        if not self.current_piece:
            return

        if key == pygame.K_SPACE:
            self._hard_drop()
        elif key == pygame.K_UP or key == pygame.K_x:
            if self.current_piece.rotate(self.grid, clockwise=True):
                self._on_piece_moved()
        elif key == pygame.K_z:
            if self.current_piece.rotate(self.grid, clockwise=False):
                self._on_piece_moved()
        elif key in (pygame.K_c, pygame.K_LSHIFT):
            self.hold_piece()

    def _hard_drop(self):
        """Hard drop current piece."""
        if not self.current_piece:
            return
        cells_dropped = 0
        while self.current_piece.move(self.grid, 0, 1):
            cells_dropped += 1
        self.add_score(cells_dropped * SCORE_HARD_DROP)
        self._lock_piece()

    def _on_piece_moved(self):
        """Called when piece moves or rotates."""
        if self.lock_timer > 0:
            self.lock_moves += 1
            if self.lock_moves < 15:  # Max lock moves
                self.lock_timer = 0

    def _update_playing(self):
        """Update game during PLAYING state."""
        if not self.current_piece:
            return

        inputs = self._get_input()

        # Handle horizontal movement with DAS
        if inputs.get("left") and not self.last_input.get("left"):
            if self.current_piece.move(self.grid, -1, 0):
                self._on_piece_moved()
            self.das_timer = 0
            self.das_direction = -1
        elif inputs.get("right") and not self.last_input.get("right"):
            if self.current_piece.move(self.grid, 1, 0):
                self._on_piece_moved()
            self.das_timer = 0
            self.das_direction = 1
        elif inputs.get("left") or inputs.get("right"):
            self.das_timer += 1
            if self.das_timer > DAS_DELAY and self.das_timer % DAS_RATE == 0:
                if self.current_piece.move(self.grid, self.das_direction, 0):
                    self._on_piece_moved()
        else:
            self.das_timer = 0
            self.das_direction = 0

        # Handle soft drop
        if inputs.get("down"):
            if self.current_piece.move(self.grid, 0, 1):
                self.add_score(SCORE_SOFT_DROP)
                self.drop_timer = 0
            else:
                self.lock_timer += 2  # Speed up lock when holding down

        # Handle rotation (from injected input)
        if inputs.get("rotate_cw") and not self.last_input.get("rotate_cw"):
            if self.current_piece.rotate(self.grid, clockwise=True):
                self._on_piece_moved()
        if inputs.get("rotate_ccw") and not self.last_input.get("rotate_ccw"):
            if self.current_piece.rotate(self.grid, clockwise=False):
                self._on_piece_moved()

        # Handle hard drop (from injected input)
        if inputs.get("hard_drop") and not self.last_input.get("hard_drop"):
            self._hard_drop()
            return

        # Handle hold (from injected input)
        if inputs.get("hold") and not self.last_input.get("hold"):
            self.hold_piece()

        # Gravity
        self.drop_timer += 1
        if self.drop_timer >= self._get_drop_speed():
            self.drop_timer = 0
            if not self.current_piece.move(self.grid, 0, 1):
                self.lock_timer += 1

        # Lock delay
        if self.lock_timer > 0:
            # Check if piece is still grounded
            if self.current_piece.move(self.grid, 0, 1):
                # Piece can fall, reset lock
                self.current_piece.move(self.grid, 0, -1)  # Move back
            else:
                self.lock_timer += 1
                if self.lock_timer >= LOCK_DELAY:
                    self._lock_piece()

        self.last_input = inputs.copy()

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
        title = self.font.render("TETRIS", True, WHITE)
        self.screen.blit(title,
                        (SCREEN_WIDTH // 2 - title.get_width() // 2, 200))

        start = self.font.render("Press ENTER to Start", True, WHITE)
        self.screen.blit(start,
                        (SCREEN_WIDTH // 2 - start.get_width() // 2, 300))

        controls = self.small_font.render("Arrows + Space + C", True, GRAY)
        self.screen.blit(controls,
                        (SCREEN_WIDTH // 2 - controls.get_width() // 2, 350))

    def _draw_game(self):
        """Draw game state."""
        # Draw grid border
        border_rect = pygame.Rect(
            GRID_OFFSET_X - 2, GRID_OFFSET_Y - 2,
            GRID_WIDTH * CELL_SIZE + 4, GRID_HEIGHT * CELL_SIZE + 4
        )
        pygame.draw.rect(self.screen, WHITE, border_rect, 2)

        # Draw grid cells
        for y in range(GRID_HEIGHT):
            for x in range(GRID_WIDTH):
                cell = self.grid[y][x]
                rect = pygame.Rect(
                    GRID_OFFSET_X + x * CELL_SIZE,
                    GRID_OFFSET_Y + y * CELL_SIZE,
                    CELL_SIZE - 1, CELL_SIZE - 1
                )
                if cell:
                    pygame.draw.rect(self.screen, PIECE_COLORS[cell], rect)
                    pygame.draw.rect(self.screen, WHITE, rect, 1)
                else:
                    pygame.draw.rect(self.screen, DARK_GRAY, rect, 1)

        # Draw ghost piece
        if self.current_piece:
            ghost_y = self.current_piece.get_ghost_y(self.grid)
            ghost = self.current_piece.copy()
            ghost.y = ghost_y
            for cx, cy in ghost.get_cells():
                if cy >= 0:
                    rect = pygame.Rect(
                        GRID_OFFSET_X + cx * CELL_SIZE,
                        GRID_OFFSET_Y + cy * CELL_SIZE,
                        CELL_SIZE - 1, CELL_SIZE - 1
                    )
                    pygame.draw.rect(self.screen, GRAY, rect, 2)

        # Draw current piece
        if self.current_piece:
            for cx, cy in self.current_piece.get_cells():
                if cy >= 0:
                    rect = pygame.Rect(
                        GRID_OFFSET_X + cx * CELL_SIZE,
                        GRID_OFFSET_Y + cy * CELL_SIZE,
                        CELL_SIZE - 1, CELL_SIZE - 1
                    )
                    pygame.draw.rect(self.screen, self.current_piece.color, rect)
                    pygame.draw.rect(self.screen, WHITE, rect, 1)

        # Draw sidebar
        self._draw_sidebar()

    def _draw_sidebar(self):
        """Draw score, next pieces, and held piece."""
        sidebar_x = GRID_OFFSET_X + GRID_WIDTH * CELL_SIZE + 30

        # Score
        score_text = self.small_font.render("SCORE", True, WHITE)
        self.screen.blit(score_text, (sidebar_x, 30))
        score_val = self.font.render(str(self.score), True, WHITE)
        self.screen.blit(score_val, (sidebar_x, 55))

        # Level
        level_text = self.small_font.render("LEVEL", True, WHITE)
        self.screen.blit(level_text, (sidebar_x, 100))
        level_val = self.font.render(str(self.level), True, WHITE)
        self.screen.blit(level_val, (sidebar_x, 125))

        # Lines
        lines_text = self.small_font.render("LINES", True, WHITE)
        self.screen.blit(lines_text, (sidebar_x, 170))
        lines_val = self.font.render(str(self.lines_cleared), True, WHITE)
        self.screen.blit(lines_val, (sidebar_x, 195))

        # Next pieces
        next_text = self.small_font.render("NEXT", True, WHITE)
        self.screen.blit(next_text, (sidebar_x, 260))

        for i, piece in enumerate(self.next_pieces[:3]):
            self._draw_mini_piece(piece, sidebar_x, 290 + i * 60)

        # Hold piece
        hold_text = self.small_font.render("HOLD", True, WHITE)
        self.screen.blit(hold_text, (sidebar_x, 480))

        if self.held_piece:
            color = self.held_piece.color if self.can_hold else GRAY
            self._draw_mini_piece(self.held_piece, sidebar_x, 510, color)

    def _draw_mini_piece(self, piece: Piece, x: int, y: int,
                         color: Optional[Tuple[int, int, int]] = None):
        """Draw a small preview of a piece."""
        if color is None:
            color = piece.color
        mini_size = 12
        shape = TETROMINOES[piece.type][0]  # First rotation

        # Center the piece
        min_x = min(dx for dx, _ in shape)
        max_x = max(dx for dx, _ in shape)
        offset_x = (4 - (max_x - min_x + 1)) * mini_size // 2

        for dx, dy in shape:
            rect = pygame.Rect(
                x + (dx - min_x) * mini_size + offset_x,
                y + dy * mini_size,
                mini_size - 1, mini_size - 1
            )
            pygame.draw.rect(self.screen, color, rect)

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

        game_over = self.font.render("GAME OVER", True, (255, 0, 0))
        self.screen.blit(game_over,
                        (SCREEN_WIDTH // 2 - game_over.get_width() // 2, 250))

        final_score = self.font.render(f"Score: {self.score}", True, WHITE)
        self.screen.blit(final_score,
                        (SCREEN_WIDTH // 2 - final_score.get_width() // 2, 300))

        restart = self.small_font.render("Press ENTER to Restart", True, WHITE)
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
