# Add Bonus Food with Timer

## Feature Description

Add special bonus food that appears periodically, stays for a limited time, and gives extra points when eaten.

## Task

Create a `BonusFood` class and integrate it into the game loop.

## Files

The main game file is `game/main.py`. Add a new class and modify the `Game` class.

## Requirements

1. Create a `BonusFood` class with:
   - `position`: Where the bonus food is located
   - `active`: Whether the bonus food is currently visible
   - `timer`: Countdown until the bonus disappears
   - `spawn_chance`: Probability of spawning each frame (e.g., 0.005)
   - `duration`: How many frames the bonus stays active (e.g., 100)
   - `points`: Points awarded (e.g., 50)

2. Add methods:
   - `update()`: Decrease timer, deactivate when timer reaches 0
   - `try_spawn(snake)`: Randomly spawn if not active
   - `check_eaten(snake)`: Check if snake head is at bonus position
   - `draw(surface)`: Draw the bonus food (different color, e.g., YELLOW)

3. Integrate into Game:
   - Create bonus_food in __init__
   - Call try_spawn and update in game update loop
   - Check if eaten and award points
   - Draw bonus food when active

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```
