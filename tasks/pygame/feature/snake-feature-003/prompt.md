# Add Wrap-Around Walls Mode

## Feature Description

Add a mode where the snake wraps around to the opposite side of the grid instead of dying when hitting a wall. This creates a "toroidal" play area.

## Task

Add a `wrap_around` setting to the Game class and modify the snake movement/collision to support wrapping.

## Files

The main game file is `game/main.py`. Modify the `Game` and `Snake` classes.

## Requirements

1. Add a `wrap_around` attribute to the `Game` class (default: False)
2. Add a `wrap_position()` method to the `Snake` class that wraps coordinates
3. Modify `Snake.move()` to wrap the head position when wrap_around is enabled
4. When wrap_around is True, `check_wall_collision()` should always return False

## Implementation

Add to Snake class:
```python
def wrap_position(self, pos: Position) -> Position:
    """Wrap position to opposite side of grid."""
    return Position(
        pos.x % GRID_WIDTH,
        pos.y % GRID_HEIGHT
    )
```

Modify move() to use wrap_position when enabled.

Add to Game class:
```python
self.wrap_around = False  # In __init__
```

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```
