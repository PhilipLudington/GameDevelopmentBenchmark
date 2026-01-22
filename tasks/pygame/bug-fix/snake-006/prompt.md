# Snake Grows in Wrong Direction

## Problem Description

When the snake eats food, the new segment sometimes appears at the head instead of at the tail. This causes a visual glitch where the snake seems to grow from the front.

## Task

Fix the `Snake.grow()` and `Snake.move()` methods to ensure new segments are added at the tail, not the head.

## Files

The main game file is `game/main.py`. Focus on the `Snake` class and its `grow` and `move` methods.

## Expected Behavior

After your fix:
- When the snake grows, the new segment should appear where the tail was
- The snake should extend from its tail end, not the head
- Growth should happen during move() by not removing the tail segment

## The Bug

The current implementation has the growth logic inverted - it's adding segments at the front:

```python
def move(self) -> None:
    # BUG: Insert at wrong end when growing
    if self.grow_pending > 0:
        self.body.insert(0, new_head)  # Wrong: inserts extra at head
        self.grow_pending -= 1
    else:
        self.body.insert(0, new_head)
        self.body.pop()  # Correct: removes tail
```

## How to Fix

The correct logic is:
1. Always insert new head at position 0
2. Only pop the tail if NOT growing
3. Decrement grow_pending when not popping

```python
def move(self) -> None:
    # ... calculate new_head ...
    self.body.insert(0, new_head)

    if self.grow_pending > 0:
        self.grow_pending -= 1  # Don't pop tail = snake grows
    else:
        self.body.pop()  # Pop tail = snake stays same length
```

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```
