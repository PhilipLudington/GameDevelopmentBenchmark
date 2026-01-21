# Add Ball Speed Increase

## Problem Description

The game feels static because the ball moves at the same speed throughout. The game should become more exciting as the rally continues.

## Task

Make the ball speed increase slightly after each paddle hit.

## Files

The main game file is `game/main.py`. Focus on the `Ball` class.

## Expected Behavior

After your implementation:
- Ball should start at normal speed
- Each paddle hit should increase ball speed by a small amount (e.g., 5-10%)
- Speed should reset to normal after a point is scored
- There should be a maximum speed cap to keep the game playable

## Testing

```bash
pytest tests/ -v
```
