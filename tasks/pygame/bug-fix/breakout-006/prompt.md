# Paddle Moves Off Screen at Edges

## Problem Description

The paddle can move partially off the left and right edges of the screen. This happens because the boundary clamping uses wrong comparisons.

## Task

Fix the `Paddle.move_left()` and `Paddle.move_right()` methods to properly clamp the paddle within screen bounds.

## Files

The main game file is `game/main.py`. Focus on the `Paddle` class.

## The Core Issue

The move methods clamp the paddle position, but the clamping values are off by one or use incorrect bounds.

For example:
```python
def move_left(self):
    self.x = max(0, self.x - self.speed)  # Correct

def move_right(self):
    self.x = min(SCREEN_WIDTH - self.width, self.x + self.speed)  # Correct
```

The bug is that the current code might use wrong values like:
- `max(-self.width/2, ...)` which allows half the paddle off screen
- `min(SCREEN_WIDTH, ...)` instead of `SCREEN_WIDTH - self.width`

## How to Fix

Ensure the clamping is:
- Left: `max(0, self.x - self.speed)`
- Right: `min(SCREEN_WIDTH - self.width, self.x + self.speed)`

## Expected Behavior

After your fix:
- Paddle left edge never goes below 0
- Paddle right edge never goes beyond SCREEN_WIDTH
- Paddle stays fully visible at all times

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```
