# Ball Escapes Through Screen Corners

## Problem Description

At high speeds, the ball can escape through the corners of the screen. This happens because horizontal and vertical boundary checks use `elif`, so when the ball hits a corner, only one axis is corrected.

## Task

Fix the wall collision logic so both horizontal and vertical corrections can happen in the same frame.

## Files

The main game file is `game/main.py`. Focus on the wall collision section in the `Game.update()` method.

## The Core Issue

The current code uses:
```python
if self.ball.position.x - self.ball.radius <= 0:
    # fix x
elif self.ball.position.x + self.ball.radius >= SCREEN_WIDTH:
    # fix x

if self.ball.position.y - self.ball.radius <= 0:
    # fix y
```

But for the horizontal check, the right side uses `elif` which means only left OR right is checked, not both (which is correct). The issue is actually that the vertical check is separate but the bottom check is missing from being a complete boundary.

Actually, the bug is simpler: when the ball is moving diagonally fast, it might need BOTH horizontal and vertical correction. The horizontal uses if/elif correctly. The issue is that the return statement when ball falls below happens before checking if ball also needs horizontal correction.

The fix is to ensure all wall checks happen before the ball-below-paddle check:

```python
# Wall collisions - check all walls
if self.ball.position.x - self.ball.radius <= 0:
    self.ball.position.x = self.ball.radius
    self.ball.bounce_horizontal()
elif self.ball.position.x + self.ball.radius >= SCREEN_WIDTH:
    self.ball.position.x = SCREEN_WIDTH - self.ball.radius
    self.ball.bounce_horizontal()

if self.ball.position.y - self.ball.radius <= 0:
    self.ball.position.y = self.ball.radius
    self.ball.bounce_vertical()

# Now check ball fell (not elif!)
if self.ball.position.y + self.ball.radius >= SCREEN_HEIGHT:
    # ... life lost logic
```

## Expected Behavior

After your fix:
- Ball correctly bounces off corners
- Both horizontal and vertical corrections can happen simultaneously

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```
