# Shield Collision Not Symmetric

## Problem Description

Players have noticed that shields behave differently depending on whether they're hit by player bullets (from below) or alien bullets (from above). When player bullets hit shields, they carve through from the bottom creating expected holes. However, alien bullets hitting from the top don't create a realistic top-down erosion pattern.

The issue is that shields should erode from the direction the bullet came from, but the current implementation doesn't account for bullet direction.

## Task

Fix the `Shield.check_collision()` method to properly handle bullets from both directions, creating realistic erosion patterns that match the bullet's direction of travel.

## Files

The main game file is `game/main.py`. Focus on the `Shield` class.

## The Core Issue

The current `check_collision()` method simply destroys the first block it finds that overlaps with the bullet. This means:
- Alien bullets from above might destroy blocks on the bottom of the shield (unrealistic)
- The erosion pattern doesn't reflect where the bullet actually came from

## Expected Behavior

After your fix:
- Player bullets (moving up) should destroy the lowest overlapping block first
- Alien bullets (moving down) should destroy the highest overlapping block first
- Shields should erode from the direction bullets hit them
- Visually, shields should show holes on the side they were hit from

## Implementation Hints

You need to:
1. Pass bullet direction information to `check_collision()` (or pass the bullet itself)
2. When multiple blocks overlap with the bullet, sort them by y-position
3. For upward bullets, destroy the block with the highest y (lowest on screen)
4. For downward bullets, destroy the block with the lowest y (highest on screen)

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```

The tests verify:
- Player bullets erode shields from the bottom
- Alien bullets erode shields from the top
- Shield blocks are destroyed in the correct order
