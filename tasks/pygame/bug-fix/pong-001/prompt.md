# Ball Passes Through Paddle Edge

## Problem Description

Players have reported that sometimes the ball passes right through the paddle, especially when hitting near the corners or edges. This happens because the collision detection only checks if the ball collides with the paddle rectangle but doesn't properly prevent the ball from passing through when moving at high speeds.

## Task

Fix the collision detection in the `Ball.check_paddle_collision()` method to ensure the ball never passes through the paddle, even when moving quickly or hitting at an angle.

## Files

The main game file is `game/main.py`. Focus on the `Ball` class and its `check_paddle_collision` method.

## Expected Behavior

After your fix:
- The ball should never pass through the paddle
- Collision should work correctly at all ball speeds
- The ball should bounce at appropriate angles based on where it hits the paddle

## Hints

- The current collision check uses `colliderect()` which can miss fast-moving objects
- Consider the ball's velocity when determining collision response
- Make sure to clamp the ball position after collision

## Testing

Run the game to verify your fix works correctly:
```bash
python game/main.py --headless --frames 1000
```

Run the automated tests:
```bash
pytest tests/ -v
```
