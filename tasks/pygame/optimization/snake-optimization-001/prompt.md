# Optimize Collision Detection with Set

## Problem Description

The current self-collision detection uses a linear search through the snake body list, which is O(n) for each check. As the snake grows longer, this becomes increasingly slow.

## Task

Optimize the collision detection by maintaining a set of body positions for O(1) lookup.

## Files

The main game file is `game/main.py`. Modify the `Snake` class.

## Requirements

1. Add a `body_set` attribute to Snake that mirrors the body list as a set of tuples
2. Update `body_set` whenever the body changes (move, grow, reset)
3. Modify `check_self_collision()` to use the set for O(1) lookup
4. Modify `check_collision()` to use the set

## Implementation

Add to Snake.__init__:
```python
self.body_set: set = set()
```

Update body_set in move():
```python
def move(self) -> None:
    # ... existing movement code ...

    # Update body set
    self.body_set = set(p.to_tuple() for p in self.body)
```

Optimize check_collision:
```python
def check_collision(self, position: Position) -> bool:
    return position.to_tuple() in self.body_set
```

Optimize check_self_collision:
```python
def check_self_collision(self) -> bool:
    # Head collides with body if head position is in tail set
    tail_set = set(p.to_tuple() for p in self.tail)
    return self.head.to_tuple() in tail_set
```

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```

Tests verify:
- Collision detection still works correctly
- Performance is improved (optional benchmark test)
