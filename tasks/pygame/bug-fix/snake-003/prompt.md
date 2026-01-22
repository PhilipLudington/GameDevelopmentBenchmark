# Score Not Reset on New Game

## Problem Description

Players have noticed that when they start a new game after dying, their score from the previous game is still displayed. This happens because the `reset_game()` method doesn't reset the score to zero.

## Task

Fix the `Game.reset_game()` method to also reset the score to 0.

## Files

The main game file is `game/main.py`. Focus on the `Game` class and its `reset_game` method.

## Expected Behavior

After your fix:
- Score should be reset to 0 when starting a new game
- High score should be preserved (not reset)
- Snake should be reset to initial length
- Food should be respawned

## How to Fix

In the `reset_game` method, add this line:
```python
self.score = 0
```

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```

Tests verify that:
- Score is reset to 0 on new game
- High score is preserved
- Snake is reset to initial length
