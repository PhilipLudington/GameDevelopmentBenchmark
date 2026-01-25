# Add Shield Power-Up System

## Feature Description

Implement a shield power-up system that adds strategic depth to the gameplay. A shield icon should spawn periodically, and when collected by the player, it provides temporary invulnerability.

## Task

Create a `PowerUp` class and integrate shield power-up spawning, collection, and duration tracking into the game.

## Files

The main game file is `game/main.py`. Modify the `Game` class and add a new `PowerUp` class.

## Requirements

1. Create a `PowerUp` class with:
   - `position: Vector2` - spawn location
   - `power_type: str` - type of power-up (for now, just "shield")
   - `active: bool` - whether the power-up is available for collection
   - `radius: int` - collision radius (15 pixels)
   - `draw()` method that renders a cyan hexagon

2. Add power-up spawning logic to `Game`:
   - `spawn_power_up()` method that creates a power-up at a random safe location
   - Spawn a power-up every 600 frames (10 seconds at 60 FPS)
   - Only one power-up can exist at a time
   - Power-ups despawn after 300 frames if not collected

3. Add shield state to `Ship`:
   - `shield_active: bool` - whether shield is currently active
   - `shield_timer: int` - remaining shield duration
   - Shield duration: 300 frames (5 seconds)
   - When shield is active, draw a cyan circle around the ship

4. Add collision detection between ship and power-up:
   - When ship touches power-up, activate shield
   - Remove power-up when collected

5. Shield behavior:
   - While shield is active, ship is invulnerable (like existing invulnerable state)
   - Shield timer decrements each frame
   - Shield deactivates when timer reaches 0

## Implementation Hints

```python
class PowerUp:
    def __init__(self, x: float, y: float, power_type: str = "shield"):
        self.position = Vector2(x, y)
        self.power_type = power_type
        self.active = True
        self.radius = 15
        self.spawn_timer = 300  # Despawn timer
```

Add to Game class:
```python
self.power_up: Optional[PowerUp] = None
self.power_up_spawn_timer = 600
```

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```
