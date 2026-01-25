# Implement Spatial Hashing for Collision Detection

## Feature Description
The current collision detection checks every entity against every other entity (O(n²)). Implement spatial hashing to reduce this to O(n) average case.

## Requirements
1. Create SpatialHash class with configurable cell size
2. Insert all collidable entities into hash each frame
3. Only check collisions between entities in same/adjacent cells
4. Maintain correctness for screen wrapping
5. Performance should scale linearly with entity count

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```
