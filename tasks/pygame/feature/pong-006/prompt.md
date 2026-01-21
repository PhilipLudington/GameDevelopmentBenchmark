# Add Pause Functionality

## Problem Description

The game currently has no way to pause. Players need to be able to pause the game during play.

## Task

Add pause functionality that can be toggled with the P key.

## Files

The main game file is `game/main.py`.

## Expected Behavior

After your implementation:
- Pressing P during gameplay should pause the game
- A "PAUSED" message should be displayed
- Pressing P or SPACE while paused should resume the game
- The game state should freeze while paused (ball and paddles don't move)

## Hints

- You may need to add a PAUSED state to the GameState enum
- Handle the pause input in the appropriate input handler

## Testing

```bash
pytest tests/ -v
```
