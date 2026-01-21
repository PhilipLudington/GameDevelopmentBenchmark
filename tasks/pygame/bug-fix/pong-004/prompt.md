# AI Paddle Jitters at Boundaries

## Problem Description

The AI paddle exhibits rapid oscillation (jittering) when the ball is near the top or bottom edges of the screen, or when the ball is moving slowly.

## Task

Fix the AI movement logic to be smoother and prevent jittering.

## Files

The main game file is `game/main.py`. Focus on the `Paddle.update_ai()` method.

## Expected Behavior

After your fix:
- AI paddle should move smoothly without oscillating
- AI should have a dead zone where it doesn't move if close enough to target
- Movement should be stable regardless of ball position

## Hints

- Consider adding a larger tolerance/dead zone for the AI decision
- The AI should not constantly switch between moving up and down

## Testing

```bash
pytest tests/ -v
```
