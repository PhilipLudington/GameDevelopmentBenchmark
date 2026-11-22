# Refactor to Entity Component System

## Feature Description
Refactor the game architecture to use an Entity Component System pattern.

## Requirements
1. Create Entity, Component, System base classes
2. Components: Transform, Sprite, Collider, Health, AI
3. Systems: Movement, Rendering, Collision, AI
4. Remove inheritance-based entity types
5. Maintain all existing functionality

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```
