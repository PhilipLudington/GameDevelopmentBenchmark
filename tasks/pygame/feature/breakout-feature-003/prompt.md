# Add Multiple Levels with Different Brick Patterns

## Problem Description

The game only has one level. Add a level system with different brick patterns for each level.

## Task

Implement a level system with:
1. At least 3 different brick patterns
2. Level indicator in the HUD
3. Transition to next level when all bricks cleared
4. Increasing difficulty (more bricks, faster ball)

## Files

The main game file is `game/main.py`. You'll need to modify the `Game` class.

## Requirements

1. Add level tracking:
   - `self.level` starting at 1
   - Display level in HUD

2. Create brick patterns:
   - Level 1: Standard 5x10 grid
   - Level 2: Pyramid pattern
   - Level 3: Checkerboard pattern (every other brick)

3. Level transition:
   - When all bricks destroyed, show "Level Complete"
   - Advance to next level
   - Reset ball and paddle, keep score
   - Increase ball speed slightly each level

4. After level 3, loop back to level 1 with higher speed

## Expected Behavior

- Game shows current level
- Each level has unique brick pattern
- Ball gets faster each level
- Score accumulates across levels

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```
