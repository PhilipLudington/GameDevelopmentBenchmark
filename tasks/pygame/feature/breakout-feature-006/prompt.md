# Add Ball Speed Increase Over Time

## Problem Description

The ball moves at constant speed throughout the game. Add gradual speed increase as bricks are destroyed.

## Task

Implement progressive ball speed:
1. Ball starts at base speed
2. Speed increases slightly with each brick destroyed
3. Maximum speed cap to keep game playable
4. Speed resets when losing a life

## Files

The main game file is `game/main.py`. Modify the `Ball` and `Game` classes.

## Requirements

1. Modify `Ball` class:
   - Add `base_speed` and current `speed`
   - Add `increase_speed(amount)` method
   - Add `reset_speed()` method
   - Cap speed at 1.5x base speed

2. Modify `Game` class:
   - After destroying brick, increase ball speed by 0.5%
   - On life lost, reset ball speed
   - On new game, reset ball speed

3. The speed increase should affect velocity magnitude:
   ```python
   def increase_speed(self, factor=1.005):
       self.speed = min(self.speed * factor, self.base_speed * 1.5)
       # Normalize and rescale velocity
       ...
   ```

## Expected Behavior

- Ball starts at normal speed
- Speed gradually increases as game progresses
- Speed never exceeds 1.5x base speed
- Speed resets on life lost or new game

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```
