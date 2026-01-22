# Score Displays Wrong Value After Brick Destruction

## Problem Description

The score callback receives the total score instead of the points earned for the brick that was just destroyed. This makes it impossible to show "+50 points" notifications because the callback doesn't know how many points were just earned.

## Task

Fix the `on_score` callback to receive the correct points value (the points earned for the brick, not the total score).

## Files

The main game file is `game/main.py`. Focus on the brick collision section in the `Game.update()` method.

## The Core Issue

The current code calculates the delta incorrectly. After updating `self.score`, it passes `self.score - old_score` which should work, but the bug is that `old_score` is captured *after* some operations that might have already modified the score.

Actually, the real bug is simpler: the callback is called with the total score instead of the delta.

## How to Fix

In the brick collision handling, the callback should receive only the points for this brick:
```python
if self.on_score:
    self.on_score(brick.points)  # Not self.score!
```

## Expected Behavior

After your fix:
- `on_score` callback receives the points earned for the brick (10-50 depending on row)
- The total score still updates correctly internally

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```
