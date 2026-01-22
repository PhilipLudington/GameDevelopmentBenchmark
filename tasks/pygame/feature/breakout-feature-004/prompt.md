# Add Multi-Hit Bricks

## Problem Description

All bricks currently take only one hit to destroy. Add bricks that require multiple hits.

## Task

Implement multi-hit bricks:
1. Bricks have a `health` property (1-3 hits)
2. Top rows have more health
3. Visual feedback showing remaining hits
4. Color changes as brick takes damage

## Files

The main game file is `game/main.py`. Modify the `Brick` class.

## Requirements

1. Modify `Brick` class:
   - Add `health` property (1, 2, or 3)
   - Add `take_damage()` method
   - Health based on row (top = 3, middle = 2, bottom = 1)
   - Change color as health decreases

2. Modify collision handling:
   - Call `brick.take_damage()` instead of `brick.alive = False`
   - Only destroy brick when health reaches 0
   - Award points for each hit (partial points)

3. Visual feedback:
   - Full color at full health
   - Slightly darker at 2 health
   - Much darker at 1 health
   - Or show crack patterns

## Expected Behavior

- Top row bricks need 3 hits
- Middle row bricks need 2 hits
- Bottom row bricks need 1 hit
- Bricks visually show damage

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```
