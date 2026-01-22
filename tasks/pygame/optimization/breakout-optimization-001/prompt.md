# Optimize Brick Collision Detection

## Problem Description

The current collision detection checks every brick every frame, even though most bricks are far from the ball. With 50 bricks, this is O(n) per frame.

## Task

Optimize collision detection using spatial partitioning or early-exit strategies.

## Files

The main game file is `game/main.py`. Modify the `Game` class.

## Requirements

Implement one of these optimization strategies:

### Option 1: Grid-based spatial partitioning
- Divide brick area into grid cells
- Only check bricks in cells near the ball
- Update cell membership when bricks destroyed

### Option 2: Row-based early exit
- Since bricks are in rows, check which row the ball is near
- Only check bricks in that row ± 1

### Option 3: Bounding box pre-check
- Skip bricks far from ball using quick distance check
- Use ball velocity to predict which bricks to check

## Suggested Implementation (Row-based)

```python
def _get_nearby_bricks(self):
    """Get bricks near the ball's y position."""
    ball_y = self.ball.position.y
    nearby = []
    for brick in self.bricks:
        if not brick.alive:
            continue
        # Check if ball is within 2 rows of this brick
        if abs(brick.y + BRICK_HEIGHT/2 - ball_y) < BRICK_HEIGHT * 2 + self.ball.radius:
            nearby.append(brick)
    return nearby
```

## Expected Behavior

- Collision detection still works correctly
- Fewer collision checks per frame
- Performance improvement measurable with many bricks

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```

Tests should verify:
- All collisions still detected
- Game behavior unchanged
- Performance metric (optional)
