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
- After collision, the ball must be completely outside the paddle (no overlap)

## The Core Issue

The current code only reverses the ball's velocity when a collision is detected, but this is insufficient. When the ball moves at high speeds (e.g., 10-15 pixels per frame), it can penetrate several pixels into the paddle in a single frame. Simply reversing velocity leaves the ball embedded inside the paddle, causing erratic behavior or pass-through on subsequent frames.

**You must fix TWO things:**
1. **Position correction**: After collision, physically move the ball so it's completely outside the paddle
2. **Velocity direction**: Ensure the velocity is pointing away from the paddle (not just reversed)

## How to Fix

When a collision is detected:
1. Check which direction the ball was moving (using `velocity.x`)
2. Based on the direction, reposition the ball to the appropriate side of the paddle:
   - If ball was moving left (`velocity.x < 0`), push ball to the right side of paddle (`ball.rect.left = paddle.rect.right`)
   - If ball was moving right (`velocity.x > 0`), push ball to the left side of paddle (`ball.rect.right = paddle.rect.left`)
3. Set velocity to point away from the paddle (use `abs()` to ensure correct sign)

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```

The key test assertions are:
- `ball.rect.left >= paddle.rect.right` after hitting left paddle
- `ball.rect.right <= paddle.rect.left` after hitting right paddle
- `not ball.rect.colliderect(paddle.rect)` after any collision
