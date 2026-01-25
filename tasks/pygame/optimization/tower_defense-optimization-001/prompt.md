# Optimize Pathfinding with Flow Fields

## Feature Description
Replace per-enemy A* pathfinding with a single flow field that all enemies follow.

## Requirements
1. Generate flow field from goal to all cells
2. Enemies follow flow field gradient
3. Recalculate only when towers placed/removed
4. Support 1000+ enemies without slowdown
5. Handle multiple paths to goal

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```
