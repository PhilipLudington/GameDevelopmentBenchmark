# Ball Passes Through Brick Corners

## Problem Description

Players have reported that the ball sometimes passes through bricks without destroying them, especially when hitting near the corners. This happens because the collision detection doesn't account for the ball's previous position - if the ball moves fast enough, it can "tunnel" through a brick corner in a single frame.

## Task

Fix the collision detection in the `update()` method of the `Game` class to ensure the ball never passes through bricks, even when moving at high speeds.

## Files

The main game file is `game/main.py`. Focus on the brick collision section in the `Game.update()` method.

## The Core Issue

The current collision check only tests if the ball overlaps with a brick at its current position. When the ball velocity is high (e.g., after bouncing at sharp angles), the ball can skip over brick corners entirely between frames.

The bug is that after detecting a collision, the code breaks out of the loop immediately. But the ball might be overlapping with multiple bricks, and we only destroy one per frame. This is actually correct, but the problem is that we don't reposition the ball after collision, so it might still be inside another brick.

**The actual bug**: When the ball collides with a brick, we calculate the bounce direction but don't ensure the ball is moved outside the brick. On the next frame, the same brick (now marked dead) is skipped, but the ball might still be partially inside another brick.

## How to Fix

After detecting a brick collision and bouncing:
1. Move the ball position slightly to ensure it's outside the brick it just hit
2. Based on the bounce direction, nudge the ball by a small amount (e.g., 1 pixel)

Add position correction after the bounce:
```python
# After determining bounce direction
if abs(dx) / BRICK_WIDTH > abs(dy) / BRICK_HEIGHT:
    self.ball.bounce_horizontal()
    # Push ball outside horizontally
    if self.ball.velocity.x > 0:
        self.ball.position.x = brick.x + BRICK_WIDTH + self.ball.radius + 1
    else:
        self.ball.position.x = brick.x - self.ball.radius - 1
else:
    self.ball.bounce_vertical()
    # Push ball outside vertically
    if self.ball.velocity.y > 0:
        self.ball.position.y = brick.y + BRICK_HEIGHT + self.ball.radius + 1
    else:
        self.ball.position.y = brick.y - self.ball.radius - 1
```

## Expected Behavior

After your fix:
- The ball should never pass through bricks
- After collision, the ball should be completely outside the brick
- Collision should work correctly at all ball speeds
- The ball should bounce at appropriate angles

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```

The tests verify that after collision:
- The ball is no longer overlapping with the destroyed brick
- The ball bounces in the correct direction
