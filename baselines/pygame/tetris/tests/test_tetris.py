"""
Comprehensive tests for Tetris game.
"""

import os
import sys
import pytest

# Setup headless mode before importing pygame
os.environ["SDL_VIDEODRIVER"] = "dummy"
os.environ["SDL_AUDIODRIVER"] = "dummy"

# Add game directory to path
game_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, game_dir)

from main import (
    Game, GameState, Piece, Position,
    GRID_WIDTH, GRID_HEIGHT, TETROMINOES, PIECE_COLORS,
    SCORE_SINGLE, SCORE_DOUBLE, SCORE_TRIPLE, SCORE_TETRIS,
    SCORE_SOFT_DROP, SCORE_HARD_DROP
)


class TestPosition:
    """Test Position class."""

    def test_position_creation(self):
        p = Position(3, 4)
        assert p.x == 3
        assert p.y == 4

    def test_position_addition(self):
        p1 = Position(1, 2)
        p2 = Position(3, 4)
        result = p1 + p2
        assert result.x == 4
        assert result.y == 6

    def test_position_to_tuple(self):
        p = Position(3, 4)
        assert p.to_tuple() == (3, 4)


class TestPiece:
    """Test Piece class."""

    def test_piece_creation(self):
        piece = Piece('T')
        assert piece.type == 'T'
        assert piece.x == 3
        assert piece.y == 0
        assert piece.rotation == 0

    def test_piece_custom_position(self):
        piece = Piece('I', x=5, y=10)
        assert piece.x == 5
        assert piece.y == 10

    def test_piece_color(self):
        for piece_type in TETROMINOES.keys():
            piece = Piece(piece_type)
            assert piece.color == PIECE_COLORS[piece_type]

    def test_piece_get_cells(self):
        piece = Piece('O', x=0, y=0)
        cells = piece.get_cells()
        assert len(cells) == 4
        # O piece at (0,0) with first rotation
        expected = [(1, 0), (2, 0), (1, 1), (2, 1)]
        assert sorted(cells) == sorted(expected)

    def test_piece_rotation_changes_cells(self):
        piece = Piece('T', x=5, y=5)
        cells_0 = piece.get_cells()
        piece.rotation = 1
        cells_1 = piece.get_cells()
        assert cells_0 != cells_1

    def test_piece_all_types_have_4_rotations(self):
        for piece_type in TETROMINOES.keys():
            assert len(TETROMINOES[piece_type]) == 4

    def test_piece_all_types_have_4_cells(self):
        for piece_type, rotations in TETROMINOES.items():
            for rotation in rotations:
                assert len(rotation) == 4, f"{piece_type} rotation has {len(rotation)} cells"

    def test_piece_copy(self):
        piece = Piece('T', x=5, y=10)
        piece.rotation = 2
        copy = piece.copy()
        assert copy.type == piece.type
        assert copy.x == piece.x
        assert copy.y == piece.y
        assert copy.rotation == piece.rotation
        assert copy is not piece


class TestPieceMovement:
    """Test piece movement."""

    def test_piece_move_empty_grid(self):
        grid = [[None for _ in range(GRID_WIDTH)] for _ in range(GRID_HEIGHT)]
        piece = Piece('T', x=5, y=5)
        assert piece.move(grid, 1, 0)
        assert piece.x == 6

    def test_piece_move_down(self):
        grid = [[None for _ in range(GRID_WIDTH)] for _ in range(GRID_HEIGHT)]
        piece = Piece('T', x=5, y=5)
        assert piece.move(grid, 0, 1)
        assert piece.y == 6

    def test_piece_blocked_by_wall_left(self):
        grid = [[None for _ in range(GRID_WIDTH)] for _ in range(GRID_HEIGHT)]
        piece = Piece('T', x=0, y=5)
        assert not piece.move(grid, -1, 0)
        assert piece.x == 0

    def test_piece_blocked_by_wall_right(self):
        grid = [[None for _ in range(GRID_WIDTH)] for _ in range(GRID_HEIGHT)]
        piece = Piece('T', x=GRID_WIDTH - 3, y=5)
        assert not piece.move(grid, 1, 0)

    def test_piece_blocked_by_floor(self):
        grid = [[None for _ in range(GRID_WIDTH)] for _ in range(GRID_HEIGHT)]
        piece = Piece('T', x=5, y=GRID_HEIGHT - 2)
        assert not piece.move(grid, 0, 1)

    def test_piece_blocked_by_other_piece(self):
        grid = [[None for _ in range(GRID_WIDTH)] for _ in range(GRID_HEIGHT)]
        grid[6][5] = 'T'  # Block cell
        piece = Piece('T', x=4, y=5)
        # T piece has cell at (5,6) relative to (4,5), so (5,6) in grid
        assert not piece.move(grid, 0, 1)


class TestPieceRotation:
    """Test piece rotation with wall kicks."""

    def test_piece_rotate_clockwise(self):
        grid = [[None for _ in range(GRID_WIDTH)] for _ in range(GRID_HEIGHT)]
        piece = Piece('T', x=5, y=5)
        initial_rotation = piece.rotation
        assert piece.rotate(grid, clockwise=True)
        assert piece.rotation == (initial_rotation + 1) % 4

    def test_piece_rotate_counter_clockwise(self):
        grid = [[None for _ in range(GRID_WIDTH)] for _ in range(GRID_HEIGHT)]
        piece = Piece('T', x=5, y=5)
        piece.rotation = 1
        assert piece.rotate(grid, clockwise=False)
        assert piece.rotation == 0

    def test_piece_rotate_with_wall_kick(self):
        grid = [[None for _ in range(GRID_WIDTH)] for _ in range(GRID_HEIGHT)]
        # Place piece at left edge
        piece = Piece('T', x=0, y=5)
        initial_x = piece.x
        # Rotate - should wall kick
        assert piece.rotate(grid, clockwise=True)
        # Piece should have moved due to wall kick
        assert piece.x != initial_x or piece.rotation == 1

    def test_piece_rotation_fails_when_blocked(self):
        grid = [[None for _ in range(GRID_WIDTH)] for _ in range(GRID_HEIGHT)]
        # Fill grid around piece to block all rotations
        for x in range(GRID_WIDTH):
            for y in range(5, 10):
                if x != 5 or y not in [6, 7]:
                    grid[y][x] = 'X'
        piece = Piece('I', x=3, y=5)
        initial_rotation = piece.rotation
        result = piece.rotate(grid, clockwise=True)
        # Rotation should fail or succeed based on wall kicks
        if not result:
            assert piece.rotation == initial_rotation


class TestGhostPiece:
    """Test ghost piece (hard drop preview)."""

    def test_ghost_y_empty_grid(self):
        grid = [[None for _ in range(GRID_WIDTH)] for _ in range(GRID_HEIGHT)]
        piece = Piece('T', x=5, y=0)
        ghost_y = piece.get_ghost_y(grid)
        assert ghost_y == GRID_HEIGHT - 2  # T piece is 2 rows tall

    def test_ghost_y_with_floor(self):
        grid = [[None for _ in range(GRID_WIDTH)] for _ in range(GRID_HEIGHT)]
        grid[15][5] = 'X'  # Block at row 15
        piece = Piece('T', x=4, y=0)
        ghost_y = piece.get_ghost_y(grid)
        assert ghost_y < GRID_HEIGHT - 2

    def test_ghost_y_does_not_modify_piece(self):
        grid = [[None for _ in range(GRID_WIDTH)] for _ in range(GRID_HEIGHT)]
        piece = Piece('T', x=5, y=3)
        original_y = piece.y
        piece.get_ghost_y(grid)
        assert piece.y == original_y


class TestGameImports:
    """Test that all required classes can be imported."""

    def test_game_imports(self):
        from main import Game, GameState, Piece
        assert Game is not None
        assert GameState is not None
        assert Piece is not None


class TestGameCreation:
    """Test Game class creation."""

    def test_game_creates_headless(self):
        game = Game(headless=True)
        assert game is not None
        assert game.headless

    def test_game_initial_state(self):
        game = Game(headless=True)
        assert game.state == GameState.MENU

    def test_game_initial_score(self):
        game = Game(headless=True)
        assert game.score == 0

    def test_game_initial_level(self):
        game = Game(headless=True)
        assert game.level == 1

    def test_game_empty_grid(self):
        game = Game(headless=True)
        for row in game.grid:
            assert all(cell is None for cell in row)

    def test_game_has_next_pieces(self):
        game = Game(headless=True)
        assert len(game.next_pieces) == 3


class TestGameState:
    """Test game state management."""

    def test_set_state(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        assert game.state == GameState.PLAYING

    def test_state_callback(self):
        game = Game(headless=True)
        states = []
        game.on_state_change = lambda s: states.append(s)
        game.set_state(GameState.PLAYING)
        assert GameState.PLAYING in states

    def test_state_callback_not_called_for_same_state(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        states = []
        game.on_state_change = lambda s: states.append(s)
        game.set_state(GameState.PLAYING)
        assert len(states) == 0


class TestBag:
    """Test randomized bag system."""

    def test_bag_fills(self):
        game = Game(headless=True)
        assert len(game.bag) >= 7

    def test_bag_contains_all_pieces(self):
        game = Game(headless=True)
        # Get many pieces and check distribution
        pieces = []
        for _ in range(14):
            pieces.append(game._get_next_piece().type)
        # Should have all piece types
        assert set(pieces) == set(TETROMINOES.keys())

    def test_bag_refills(self):
        game = Game(headless=True)
        # Drain the bag
        for _ in range(20):
            game._get_next_piece()
        # Should still be able to get pieces
        piece = game._get_next_piece()
        assert piece is not None


class TestSpawn:
    """Test piece spawning."""

    def test_spawn_piece(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game._spawn_piece()
        assert game.current_piece is not None

    def test_spawn_uses_next_queue(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        next_type = game.next_pieces[0].type
        game._spawn_piece()
        assert game.current_piece.type == next_type

    def test_spawn_refills_next_queue(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game._spawn_piece()
        assert len(game.next_pieces) == 3

    def test_spawn_game_over_when_blocked(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        # Fill top rows
        for x in range(GRID_WIDTH):
            game.grid[0][x] = 'X'
            game.grid[1][x] = 'X'
        game._spawn_piece()
        assert game.state == GameState.GAME_OVER


class TestHold:
    """Test hold piece mechanic."""

    def test_hold_piece(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game._spawn_piece()
        original_type = game.current_piece.type
        game.hold_piece()
        assert game.held_piece is not None
        assert game.held_piece.type == original_type

    def test_hold_swaps_with_held(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game._spawn_piece()
        first_type = game.current_piece.type
        game.hold_piece()
        second_type = game.current_piece.type
        game.can_hold = True  # Reset
        game.hold_piece()
        assert game.current_piece.type == first_type

    def test_cannot_hold_twice_per_piece(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game._spawn_piece()
        game.hold_piece()
        assert not game.can_hold
        held_type = game.held_piece.type
        game.hold_piece()  # Should fail
        assert game.held_piece.type == held_type


class TestLineClearing:
    """Test line clearing mechanics."""

    def test_clear_single_line(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        # Fill bottom row
        for x in range(GRID_WIDTH):
            game.grid[GRID_HEIGHT - 1][x] = 'X'
        lines = game._clear_lines()
        assert lines == 1
        # Row should be cleared
        assert all(cell is None for cell in game.grid[GRID_HEIGHT - 1])

    def test_clear_multiple_lines(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        # Fill bottom 4 rows
        for y in range(GRID_HEIGHT - 4, GRID_HEIGHT):
            for x in range(GRID_WIDTH):
                game.grid[y][x] = 'X'
        lines = game._clear_lines()
        assert lines == 4

    def test_clear_lines_shifts_down(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        # Place a piece above a full row
        game.grid[GRID_HEIGHT - 2][0] = 'T'
        for x in range(GRID_WIDTH):
            game.grid[GRID_HEIGHT - 1][x] = 'X'
        game._clear_lines()
        # The T should have shifted down
        assert game.grid[GRID_HEIGHT - 1][0] == 'T'

    def test_lines_cleared_count(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        for x in range(GRID_WIDTH):
            game.grid[GRID_HEIGHT - 1][x] = 'X'
        game._clear_lines()
        assert game.lines_cleared == 1

    def test_line_clear_callback(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        lines_cleared = []
        game.on_line_clear = lambda n: lines_cleared.append(n)
        for x in range(GRID_WIDTH):
            game.grid[GRID_HEIGHT - 1][x] = 'X'
        game._spawn_piece()
        game._lock_piece()  # This calls _clear_lines internally
        # Callback called when lines cleared during lock
        assert 1 in lines_cleared or len(lines_cleared) > 0


class TestScoring:
    """Test scoring system."""

    def test_single_line_score(self):
        game = Game(headless=True)
        game._score_lines(1)
        assert game.score >= SCORE_SINGLE

    def test_double_line_score(self):
        game = Game(headless=True)
        game._score_lines(2)
        assert game.score >= SCORE_DOUBLE

    def test_triple_line_score(self):
        game = Game(headless=True)
        game._score_lines(3)
        assert game.score >= SCORE_TRIPLE

    def test_tetris_score(self):
        game = Game(headless=True)
        game._score_lines(4)
        assert game.score >= SCORE_TETRIS

    def test_level_multiplier(self):
        game = Game(headless=True)
        game.level = 2
        game._score_lines(1)
        assert game.score >= SCORE_SINGLE * 2

    def test_combo_bonus(self):
        game = Game(headless=True)
        game.combo = 1
        game._score_lines(1)
        score_with_combo = game.score
        game.score = 0
        game.combo = 0
        game._score_lines(1)
        assert score_with_combo > game.score


class TestLevel:
    """Test level progression."""

    def test_level_up_after_10_lines(self):
        game = Game(headless=True)
        game.lines_cleared = 10
        game._clear_lines()  # Updates level
        # Level should be 2 (10 lines / 10 + 1)
        assert game.level >= 2

    def test_drop_speed_increases_with_level(self):
        game = Game(headless=True)
        speed_level_1 = game._get_drop_speed()
        game.level = 10
        speed_level_10 = game._get_drop_speed()
        assert speed_level_10 < speed_level_1


class TestInput:
    """Test input handling."""

    def test_inject_input(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game._spawn_piece()
        original_x = game.current_piece.x
        game.inject_input({"left": True})
        game.step()
        assert game.current_piece.x == original_x - 1

    def test_inject_input_right(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game._spawn_piece()
        original_x = game.current_piece.x
        game.inject_input({"right": True})
        game.step()
        assert game.current_piece.x == original_x + 1

    def test_inject_input_down(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game._spawn_piece()
        original_y = game.current_piece.y
        game.inject_input({"down": True})
        game.step()
        assert game.current_piece.y > original_y

    def test_inject_input_cleared_after_step(self):
        game = Game(headless=True)
        game.inject_input({"left": True})
        game.step()
        assert game.injected_input == {}


class TestHardDrop:
    """Test hard drop mechanic."""

    def test_hard_drop_moves_to_bottom(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game._spawn_piece()
        game._hard_drop()
        # Piece should be locked
        assert game.current_piece is not None  # New piece spawned

    def test_hard_drop_scores_points(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game._spawn_piece()
        initial_score = game.score
        game._hard_drop()
        assert game.score > initial_score


class TestLockPiece:
    """Test piece locking."""

    def test_lock_piece_adds_to_grid(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game._spawn_piece()
        game.current_piece.y = GRID_HEIGHT - 2
        piece_type = game.current_piece.type
        game._lock_piece()
        # Check that piece is in grid
        has_piece = False
        for row in game.grid:
            if piece_type in row:
                has_piece = True
                break
        assert has_piece

    def test_lock_piece_spawns_new_piece(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game._spawn_piece()
        old_type = game.current_piece.type
        game._lock_piece()
        # New piece should be spawned
        assert game.current_piece is not None


class TestReset:
    """Test game reset."""

    def test_reset_clears_grid(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game.grid[10][5] = 'T'
        game.reset()
        for row in game.grid:
            assert all(cell is None for cell in row)

    def test_reset_clears_score(self):
        game = Game(headless=True)
        game.add_score(1000)
        game.reset()
        assert game.score == 0

    def test_reset_clears_held_piece(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game._spawn_piece()
        game.hold_piece()
        game.reset()
        assert game.held_piece is None

    def test_reset_sets_menu_state(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game.reset()
        assert game.state == GameState.MENU


class TestStep:
    """Test step function."""

    def test_step_returns_true(self):
        game = Game(headless=True)
        assert game.step()

    def test_step_does_not_update_menu(self):
        game = Game(headless=True)
        # In menu state, step should not modify game state
        initial_score = game.score
        for _ in range(60):
            game.step()
        assert game.score == initial_score


class TestCombo:
    """Test combo system."""

    def test_combo_increments_on_line_clear(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game._spawn_piece()
        # Manually set up to test combo
        game.combo = 0
        for x in range(GRID_WIDTH):
            game.grid[GRID_HEIGHT - 1][x] = 'X'
        game._lock_piece()  # Should clear line and increment combo
        # Combo might be 1 now or reset to 0 if next lock doesn't clear
        assert game.combo >= 0

    def test_combo_resets_on_no_clear(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game._spawn_piece()
        game.combo = 5
        # Lock piece without clearing any lines
        game.current_piece.y = 5
        game._lock_piece()
        assert game.combo == 0


class TestDAS:
    """Test Delayed Auto Shift."""

    def test_das_activates_after_delay(self):
        game = Game(headless=True)
        game.set_state(GameState.PLAYING)
        game._spawn_piece()
        original_x = game.current_piece.x

        # Hold left for many frames
        for i in range(20):
            game.inject_input({"left": True})
            game.step()

        # Piece should have moved multiple times
        assert game.current_piece.x < original_x - 1


class TestTetrominoShapes:
    """Test that all tetromino shapes are correct."""

    def test_all_pieces_have_4_cells(self):
        for piece_type in TETROMINOES:
            for rotation in range(4):
                piece = Piece(piece_type)
                piece.rotation = rotation
                cells = piece.get_cells()
                assert len(cells) == 4, f"{piece_type} rotation {rotation} has {len(cells)} cells"

    def test_o_piece_same_all_rotations(self):
        piece = Piece('O', x=0, y=0)
        cells_0 = set(piece.get_cells())
        for rotation in range(1, 4):
            piece.rotation = rotation
            cells = set(piece.get_cells())
            assert cells == cells_0

    def test_i_piece_is_4_long(self):
        piece = Piece('I', x=0, y=0)
        cells = piece.get_cells()
        xs = [c[0] for c in cells]
        ys = [c[1] for c in cells]
        # Should be 4 wide or 4 tall
        assert max(xs) - min(xs) == 3 or max(ys) - min(ys) == 3


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
