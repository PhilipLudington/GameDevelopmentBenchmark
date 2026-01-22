# Lives Not Resetting on New Game

## Problem Description

When starting a new game after game over, the lives counter doesn't reset to the initial value. Players start with 0 or 1 life instead of 3.

## Task

Fix the `reset_game()` method to properly reset lives to `INITIAL_LIVES`.

## Files

The main game file is `game/main.py`. Focus on the `Game.reset_game()` method.

## The Core Issue

The `reset_game()` method is missing the line to reset lives. It resets the paddle, ball, bricks, and score, but not lives.

## How to Fix

Add `self.lives = INITIAL_LIVES` to the `reset_game()` method:
```python
def reset_game(self):
    """Reset game to initial state for a new game."""
    self.paddle.reset()
    self.ball.reset()
    self._create_bricks()
    self.score = 0
    self.lives = INITIAL_LIVES  # Add this line!
```

## Expected Behavior

After your fix:
- Starting a new game resets lives to 3
- This works whether coming from game over or victory

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```
