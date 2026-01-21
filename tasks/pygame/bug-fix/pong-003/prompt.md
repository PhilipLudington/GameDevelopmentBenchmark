# Ball Stuck After Scoring

## Problem Description

After a point is scored, the ball freezes in place instead of resetting to the center and launching in a new direction. This makes the game unplayable after the first point.

## Task

Fix the ball reset logic so the ball properly restarts after each score.

## Files

The main game file is `game/main.py`. Focus on the `Ball.reset()` method and how scoring is handled in `Game.update()`.

## Expected Behavior

After your fix:
- Ball should reset to center after each score
- Ball should launch toward the player who was scored on
- Ball velocity should be reset to initial speed

## Testing

```bash
pytest tests/ -v
```
