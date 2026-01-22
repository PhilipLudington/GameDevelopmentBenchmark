# Wall Collision Check Off by One

## Problem Description

Players have noticed that the snake can sometimes appear to go one cell outside the visible grid before the game ends. This is due to an off-by-one error in the wall collision detection.

## Task

Fix the `Snake.check_wall_collision()` method to correctly detect when the snake head is outside the grid boundaries.

## Files

The main game file is `game/main.py`. Focus on the `Snake` class and its `check_wall_collision` method.

## Expected Behavior

After your fix:
- The snake should die immediately when its head reaches coordinates outside [0, GRID_WIDTH-1] for x
- The snake should die immediately when its head reaches coordinates outside [0, GRID_HEIGHT-1] for y
- The collision should be detected using `<` and `>=` comparisons, not `<=` and `>`

## The Bug

The current code uses incorrect comparison operators:
```python
# BUG: Uses <= and > instead of < and >=
head.x <= 0  # Wrong - should be head.x < 0
head.x > GRID_WIDTH  # Wrong - should be head.x >= GRID_WIDTH
```

## How to Fix

Change the comparisons to:
```python
head.x < 0 or head.x >= GRID_WIDTH or head.y < 0 or head.y >= GRID_HEIGHT
```

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```
