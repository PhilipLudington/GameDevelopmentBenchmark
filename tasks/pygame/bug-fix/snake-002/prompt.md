# Food Spawns on Snake Body

## Problem Description

Players have reported that sometimes food appears inside the snake's body, making it impossible to collect. This happens because the `Food.spawn()` method is missing the collision check against the snake.

## Task

Fix the `Food.spawn()` method to ensure food never spawns on any part of the snake's body.

## Files

The main game file is `game/main.py`. Focus on the `Food` class and its `spawn` method.

## Expected Behavior

After your fix:
- Food should never spawn on any segment of the snake
- Food should always spawn within the game grid boundaries
- If initially spawned on the snake, it should try again until it finds a valid position

## How to Fix

The `spawn` method needs a loop that keeps generating random positions until it finds one that doesn't collide with the snake:

```python
def spawn(self, snake: Snake) -> None:
    while True:
        x = random.randint(0, GRID_WIDTH - 1)
        y = random.randint(0, GRID_HEIGHT - 1)
        self.position = Position(x, y)

        # Make sure food doesn't spawn on snake
        if not snake.check_collision(self.position):
            break
```

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```

The key test assertions verify that:
- Food never spawns on snake head
- Food never spawns on snake body segments
- Food always spawns within grid boundaries
