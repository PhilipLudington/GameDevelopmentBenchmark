# Add Paddle Width Power-Up

## Problem Description

Add a power-up that temporarily increases the paddle width, making it easier to catch the ball.

## Task

Add a paddle width power-up that:
1. Randomly drops from destroyed bricks (15% chance)
2. When collected, increases paddle width by 50%
3. Effect lasts for 10 seconds
4. Visual indication of active power-up

## Files

The main game file is `game/main.py`. You'll need to modify the `Paddle` and `Game` classes.

## Requirements

1. Modify `Paddle` class:
   - Add `base_width` to store original width
   - Add `set_width(width)` method
   - Add `reset_width()` method

2. Create `PowerUp` class (if not exists):
   - Different color for paddle width (e.g., green)
   - Falls from destroyed brick position

3. Modify `Game` class:
   - Track active power-up timers
   - When collected, expand paddle and start timer
   - After 10 seconds, reset paddle width
   - Show timer or visual indicator

## Expected Behavior

- Green power-up occasionally falls from bricks
- Collecting it makes paddle 50% wider
- Paddle returns to normal after 10 seconds
- Power-up can be refreshed by collecting another

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```
