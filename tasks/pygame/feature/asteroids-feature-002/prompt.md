# Implement Momentum-Based Thrust Physics

## Feature Description
Enhance the ship physics to use realistic momentum-based thrust. The ship should maintain velocity when not thrusting and thrust should add to the current velocity vector rather than replace it.

## Requirements
1. Remove instant velocity changes - ship maintains momentum
2. Thrust adds acceleration in facing direction
3. Add configurable mass and thrust power
4. Implement proper deceleration (friction/drag)
5. Add momentum indicator to HUD

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```
