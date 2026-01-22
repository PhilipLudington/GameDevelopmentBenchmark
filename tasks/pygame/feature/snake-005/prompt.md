# Add Obstacles on the Grid

## Feature Description

Add static obstacles that the snake must avoid. Hitting an obstacle should end the game just like hitting a wall.

## Task

Create an obstacle system and integrate it into collision detection.

## Files

The main game file is `game/main.py`. Add a new class and modify the `Game` class.

## Requirements

1. Create an `Obstacle` class or use a list of Position objects for obstacles

2. Add to Game class:
   - `obstacles`: List of obstacle positions
   - `num_obstacles`: Number of obstacles to place (e.g., 5)
   - `generate_obstacles()`: Place obstacles randomly, avoiding snake start position

3. Modify collision detection:
   - Add `check_obstacle_collision()` method to check if snake head hits an obstacle
   - Call this in the update loop alongside wall and self collision

4. Modify food spawning:
   - Food should not spawn on obstacles

5. Drawing:
   - Draw obstacles as gray squares on the grid

## Implementation

```python
class Game:
    def __init__(self):
        # ... existing code ...
        self.obstacles: List[Position] = []
        self.num_obstacles = 5

    def generate_obstacles(self) -> None:
        """Generate random obstacle positions."""
        self.obstacles = []
        snake_positions = set(p.to_tuple() for p in self.snake.body)

        while len(self.obstacles) < self.num_obstacles:
            x = random.randint(0, GRID_WIDTH - 1)
            y = random.randint(0, GRID_HEIGHT - 1)
            pos = Position(x, y)
            if pos.to_tuple() not in snake_positions and pos not in self.obstacles:
                self.obstacles.append(pos)
```

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```
