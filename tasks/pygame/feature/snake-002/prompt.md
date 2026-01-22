# Add Pause Functionality

## Feature Description

The game currently doesn't have proper pause functionality. Players should be able to pause the game by pressing P or ESC, and resume by pressing the same key or SPACE.

## Task

The pause state already exists in the game (GameState.PAUSED), but the input handling for entering and exiting pause is missing. Add the functionality to pause and resume the game.

## Files

The main game file is `game/main.py`. Modify the `Game` class input handling methods.

## Requirements

1. When in PLAYING state:
   - Pressing P or ESC should pause the game
2. When in PAUSED state:
   - Pressing P, ESC, or SPACE should resume the game
3. The pause state should show the "PAUSED" overlay (already implemented)

## Implementation

Add to `handle_playing_input`:
```python
if event.key == pygame.K_p or event.key == pygame.K_ESCAPE:
    self.set_state(GameState.PAUSED)
```

Add to `handle_paused_input`:
```python
if event.key in (pygame.K_p, pygame.K_ESCAPE, pygame.K_SPACE):
    self.set_state(GameState.PLAYING)
```

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```
