# Add Length Display

## Feature Description

Add a display showing the current snake length on the game screen. This helps players track their progress.

## Task

Add a method to display the snake's current length and call it during the draw phase.

## Files

The main game file is `game/main.py`. Modify the `Game` class.

## Requirements

1. Add a `draw_length()` method to the `Game` class
2. Display the snake length in the format "Length: X"
3. Position it below the score display
4. Call `draw_length()` in the `draw()` method when in PLAYING state

## Implementation

Add to Game class:
```python
def draw_length(self) -> None:
    """Draw the current snake length."""
    length_text = self.small_font.render(
        f"Length: {len(self.snake.body)}", True, WHITE
    )
    self.screen.blit(length_text, (10, 50))  # Below score at y=10
```

Add call in draw() method after draw_score():
```python
self.draw_length()
```

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```
