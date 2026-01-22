# Add Multi-Ball Power-Up

## Problem Description

The game needs a multi-ball power-up that spawns additional balls when collected, making the game more exciting and strategic.

## Task

Add a power-up system with a multi-ball power-up that:
1. Randomly drops from destroyed bricks (10% chance)
2. Falls down the screen
3. When collected by the paddle, spawns 2 additional balls

## Files

The main game file is `game/main.py`. You'll need to add a `PowerUp` class and modify the `Game` class.

## Requirements

1. Create a `PowerUp` class with:
   - Position and velocity
   - Type (for now, just "multiball")
   - `update()` method to move it down
   - `rect` property for collision detection

2. Modify `Game` class:
   - Track list of `powerups`
   - Track list of `balls` (currently single ball)
   - When brick destroyed, 10% chance to spawn powerup
   - Update and draw powerups
   - On paddle collision with powerup, activate effect
   - Multi-ball spawns 2 new balls at current ball position

3. Game over only when ALL balls are lost, not just one

## Expected Behavior

- Power-ups appear as falling colored rectangles
- Collecting multi-ball power-up creates 2 additional balls
- Game continues as long as at least one ball is in play
- When all balls are lost, lose a life

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```
