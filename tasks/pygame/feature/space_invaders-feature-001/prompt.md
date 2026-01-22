# Mystery Ship Bonus

## Problem Description

The classic Space Invaders experience is incomplete without the iconic Mystery Ship (also known as the UFO or flying saucer) that periodically flies across the top of the screen. This bonus target adds excitement and risk/reward gameplay.

## Task

Implement a Mystery Ship that:
- Appears periodically at random intervals
- Flies across the top of the screen (left-to-right or right-to-left)
- Can be shot for bonus points (50-300 points based on timing)
- Has a distinct appearance and/or color

## Files

The main game file is `game/main.py`.

## Requirements

### Mystery Ship Class
Create a new `MysteryShip` class with:
- Position tracking (x, y)
- Direction (1 for right, -1 for left)
- Speed (faster than regular aliens)
- Active state (visible/flying or inactive/waiting)
- Points value (randomized or based on shot timing)
- Spawn timer/counter

### Behavior
- Spawn randomly every 20-40 seconds of gameplay
- Appear from either left or right edge (randomized)
- Fly horizontally across the screen at a constant speed
- Disappear when reaching the opposite edge
- Can be shot by player bullets
- Award bonus points when destroyed

### Visual
- Draw as a distinct shape (saucer, diamond, or different colored rectangle)
- Should stand out from regular aliens
- Consider adding a simple animation or color cycling

### Integration
- Add to the `Game` class
- Update in `Game.update()`
- Draw in `Game._draw_game()`
- Check collision with player bullets
- Award points through the existing score system

## Expected Behavior

After implementation:
- Mystery ship appears every 20-40 seconds
- Flies across the top of the screen (above the alien fleet)
- Can be shot for 50-300 bonus points
- Properly resets when game restarts
- Does not interfere with normal gameplay

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```

Tests verify:
- Mystery ship spawns at correct intervals
- Mystery ship moves across screen
- Mystery ship can be shot and awards points
- Mystery ship despawns when reaching screen edge
- Mystery ship resets properly on game reset
