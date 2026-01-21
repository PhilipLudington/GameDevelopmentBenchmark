# Ball Angle Always Same

## Problem Description

The ball always bounces at the same angle regardless of where it hits the paddle. This makes the game predictable and less fun.

## Task

Fix the ball bounce logic so the angle varies based on where the ball hits the paddle.

## Files

The main game file is `game/main.py`. Focus on the `Ball.check_paddle_collision()` method.

## Expected Behavior

After your fix:
- Ball hitting the center of the paddle should bounce relatively straight
- Ball hitting the top of the paddle should bounce upward
- Ball hitting the bottom of the paddle should bounce downward
- The angle should be proportional to how far from center the ball hits

## Testing

```bash
pytest tests/ -v
```
