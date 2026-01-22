# Create Two-Player Snake

## Feature Description

Create a two-player competitive mode where both players control their own snake on the same grid. Players compete to eat food and survive longer.

## Task

Add a second snake and player controls, with proper collision handling between snakes.

## Files

The main game file is `game/main.py`. Modify the `Game` class significantly.

## Requirements

1. Add a second snake (`snake2`) controlled by arrow keys (WASD for player 1)
2. Each snake has its own score
3. Collision rules:
   - Snake dies if it hits a wall
   - Snake dies if it hits itself
   - Snake dies if it hits the other snake's body
   - If both snakes hit each other's head simultaneously, both die
4. Food can be eaten by either snake (first to reach it)
5. Game ends when one or both snakes die
6. Display winner at game over

## Implementation

Add to Game.__init__:
```python
start_pos1 = Position(GRID_WIDTH // 4, GRID_HEIGHT // 2)
start_pos2 = Position(3 * GRID_WIDTH // 4, GRID_HEIGHT // 2)
self.snake1 = Snake(start_pos1)
self.snake2 = Snake(start_pos2)
self.snake2.direction = Direction.LEFT  # Start facing each other
self.score1 = 0
self.score2 = 0
```

Add separate input handling:
```python
# Player 1: WASD
if event.key == pygame.K_w:
    self.snake1.change_direction(Direction.UP)
# ... etc

# Player 2: Arrow keys
if event.key == pygame.K_UP:
    self.snake2.change_direction(Direction.UP)
# ... etc
```

Add collision between snakes:
```python
def check_snake_collision(self, snake: Snake, other: Snake) -> bool:
    """Check if snake head hits other snake's body."""
    return other.check_collision(snake.head)
```

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```
