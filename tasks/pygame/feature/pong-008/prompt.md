# Add Power-ups

## Problem Description

The game needs more variety and excitement. Add power-up items that spawn during gameplay.

## Task

Implement a power-up system with at least 3 different power-up types.

## Files

The main game file is `game/main.py`.

## Expected Behavior

After your implementation:
- Power-ups should spawn randomly during gameplay
- Collecting a power-up (ball hitting it) should activate its effect
- Power-up effects should last for a limited time
- At least 3 power-up types:
  - Speed boost: Makes your paddle faster
  - Paddle size: Makes your paddle larger
  - Slow ball: Slows down the ball temporarily

## Hints

- Create a PowerUp class to represent power-ups
- Track active power-ups and their remaining duration
- Render power-ups with different colors for each type

## Testing

```bash
pytest tests/ -v
```
