# Add Grid Toggle

## Feature Description

Add the ability to toggle the grid lines on and off by pressing the G key. Some players prefer to play without the grid for a cleaner look.

## Task

Add a `show_grid` attribute and toggle functionality to the Game class.

## Files

The main game file is `game/main.py`. Modify the `Game` class.

## Requirements

1. Add `show_grid` attribute to Game.__init__ (default: True)
2. Add G key handling in `handle_playing_input` to toggle the grid
3. Modify `draw()` to only call `draw_grid()` when `show_grid` is True

## Implementation

Add to Game.__init__:
```python
self.show_grid = True
```

Add to handle_playing_input:
```python
elif event.key == pygame.K_g:
    self.show_grid = not self.show_grid
```

Modify draw() method:
```python
if self.show_grid:
    self.draw_grid()
```

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```
