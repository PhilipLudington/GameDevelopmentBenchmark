# Snake Can Reverse Direction with Quick Inputs

## Problem Description

Players have discovered they can kill the snake instantly by pressing two direction keys in quick succession. For example, if the snake is moving right and the player quickly presses UP then LEFT, the snake reverses direction and collides with itself on the next frame.

The problem is that the `change_direction` method checks against the *current* direction, but multiple direction changes can be queued before the snake moves. The first change (e.g., UP) updates `next_direction`, then the second change (e.g., LEFT) is now valid because it checks against the old direction (RIGHT) rather than the pending direction (UP).

## Task

Fix the `Snake.change_direction()` method so that direction changes are validated against the pending direction (`next_direction`), not the current direction.

## Files

The main game file is `game/main.py`. Focus on the `Snake` class and its `change_direction` method.

## Expected Behavior

After your fix:
- Direction changes should be validated against `next_direction` instead of `direction`
- The snake should never be able to reverse direction, even with rapid key presses
- Valid 90-degree turns should still work normally

## How to Fix

In the `change_direction` method, change the comparison from:
```python
if new_direction != self.direction.opposite:
```
to check against `next_direction` instead:
```python
if new_direction != self.next_direction.opposite:
```

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```

The key test assertions verify that:
- Rapid opposite direction changes are blocked
- Valid 90-degree turns still work
- The snake cannot instantly reverse direction
