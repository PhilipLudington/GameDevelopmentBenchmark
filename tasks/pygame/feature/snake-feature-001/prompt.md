# Add Speed Increase as Snake Grows

## Feature Description

The game currently runs at a constant speed regardless of how long the snake is. To make the game more challenging as the player progresses, implement a feature that gradually increases the game speed as the snake grows longer.

## Task

Implement a speed increase mechanism in the `Game` class that increases the FPS based on the snake's length.

## Files

The main game file is `game/main.py`. Modify the `Game` class.

## Requirements

1. Add a `get_current_fps()` method to the `Game` class that returns the current FPS based on snake length
2. The base FPS should be 10 (the starting speed)
3. For every 5 segments the snake grows beyond the initial length, increase FPS by 1
4. Cap the maximum FPS at 20 to keep the game playable
5. Update the game loop to use the dynamic FPS

## Implementation Details

Add this method to the Game class:
```python
def get_current_fps(self) -> int:
    """Calculate current FPS based on snake length."""
    base_fps = 10
    initial_length = 3  # INITIAL_SNAKE_LENGTH
    extra_segments = max(0, len(self.snake.body) - initial_length)
    speed_increase = extra_segments // 5
    return min(base_fps + speed_increase, 20)
```

Then modify the `run()` method to use:
```python
self.clock.tick(self.get_current_fps())
```

Also modify the `step()` method similarly.

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```

Tests will verify:
- Base FPS is 10 when snake is at initial length
- FPS increases by 1 for every 5 extra segments
- FPS is capped at 20
