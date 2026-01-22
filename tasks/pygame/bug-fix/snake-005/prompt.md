# High Score Resets When Returning to Menu

## Problem Description

Players have noticed that their high score is lost when they return to the main menu from the game over screen. The high score should persist throughout the game session.

## Task

Fix the `Game.handle_game_over_input()` method to preserve the high score when returning to the menu.

## Files

The main game file is `game/main.py`. Focus on the `Game` class and its input handling methods.

## Expected Behavior

After your fix:
- High score should persist when returning to menu
- High score should persist when starting a new game
- High score should only reset when the game application is restarted

## The Bug

The current `handle_game_over_input` method resets the high score when pressing ESC to return to menu:

```python
def handle_game_over_input(self, event):
    if event.key == pygame.K_ESCAPE:
        self.high_score = 0  # BUG: This resets the high score
        self.set_state(GameState.MENU)
```

## How to Fix

Remove the line that resets `self.high_score`:

```python
def handle_game_over_input(self, event):
    if event.key == pygame.K_ESCAPE:
        self.set_state(GameState.MENU)  # Just change state, don't reset high score
```

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```
