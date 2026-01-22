# Ball Stuck in Paddle After Collision

## Problem Description

Sometimes the ball gets stuck inside the paddle and bounces multiple times internally, causing erratic behavior. This happens when the ball is moving fast enough to penetrate deeply into the paddle before the collision is detected.

## Task

Fix the paddle collision to ensure the ball is repositioned above the paddle after bouncing.

## Files

The main game file is `game/main.py`. Focus on the paddle collision section in the `Game.update()` method.

## The Core Issue

The current code only checks if the ball is moving downward, but doesn't reposition the ball after reversing velocity. If the ball penetrates deeply into the paddle, it will still be inside on the next frame and bounce again.

## How to Fix

After detecting paddle collision and before bouncing, move the ball position so it's just above the paddle:
```python
# Before bouncing, push ball above paddle
self.ball.position.y = self.paddle.y - self.ball.radius
```

Actually, looking at the code, this line exists but the check `if self.ball.velocity.y > 0` might fail if the ball already bounced. The fix is to ensure the check works correctly:
```python
if self.ball.rect.colliderect(self.paddle.rect):
    if self.ball.velocity.y > 0:  # Only bounce if moving down
        self.ball.position.y = self.paddle.y - self.ball.radius
        self.ball.bounce_off_paddle(self.paddle)
```

Wait, the actual bug is that we need to check the ball is actually above the paddle (not inside from the side):
```python
if self.ball.rect.colliderect(self.paddle.rect):
    # Only bounce if ball center is above paddle top
    if self.ball.position.y < self.paddle.y and self.ball.velocity.y > 0:
        self.ball.position.y = self.paddle.y - self.ball.radius
        self.ball.bounce_off_paddle(self.paddle)
```

## Expected Behavior

After your fix:
- Ball never gets stuck inside the paddle
- Ball bounces cleanly off the paddle top
- Side collisions don't cause bouncing

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```
