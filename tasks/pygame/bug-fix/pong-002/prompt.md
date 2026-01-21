# Score Displays Wrong Player

## Problem Description

Players have reported that the scores shown on screen are reversed - Player 1's score appears on the right side and Player 2's score appears on the left side.

## Task

Fix the score display in the `draw_scores()` method so that Player 1's score appears on the left and Player 2's score appears on the right.

## Files

The main game file is `game/main.py`. Focus on the `draw_scores` method in the `Game` class.

## Expected Behavior

After your fix:
- Player 1's score should be displayed on the left side of the screen
- Player 2's score should be displayed on the right side of the screen
- The scores should update correctly as points are scored

## Testing

```bash
pytest tests/ -v
```
