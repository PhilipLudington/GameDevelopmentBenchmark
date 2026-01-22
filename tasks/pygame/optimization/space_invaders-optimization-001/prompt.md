# Optimize Collision Detection with Spatial Partitioning

## Problem Description

The current collision detection is O(bullets * aliens) - every bullet checks every alien every frame. With 55 aliens and potentially multiple bullets, this means hundreds of rectangle collision checks per frame. While manageable for this game, it doesn't scale well.

Implement spatial partitioning to reduce collision checks to only nearby objects.

## Task

Implement a spatial hash grid (or similar structure) that divides the screen into cells and only checks collisions between objects in the same or adjacent cells.

## Files

The main game file is `game/main.py`.

## Requirements

### Spatial Hash Grid Class
```python
class SpatialHashGrid:
    def __init__(self, cell_size: int = 50):
        self.cell_size = cell_size
        self.cells: dict[tuple, list] = {}

    def clear(self):
        """Clear all cells for new frame."""
        self.cells.clear()

    def insert(self, obj, rect: pygame.Rect):
        """Insert object into all cells it overlaps."""
        # Calculate which cells this rect overlaps
        # Insert obj reference into each cell
        pass

    def get_nearby(self, rect: pygame.Rect) -> set:
        """Get all objects in cells that rect overlaps."""
        pass

    def _get_cell_coords(self, x: float, y: float) -> tuple:
        """Convert position to cell coordinates."""
        return (int(x // self.cell_size), int(y // self.cell_size))
```

### Integration
1. Create a `SpatialHashGrid` instance in `Game`
2. At start of each `update()`:
   - Clear the grid
   - Insert all alive aliens into the grid
   - Insert shields (or shield blocks) into the grid
3. When checking bullet collisions:
   - Query nearby objects instead of checking all
   - Only check collision with returned objects

### Cell Size Considerations
- Too small: Many cells, objects span multiple cells
- Too large: Too many objects per cell, little benefit
- Suggested: 50-100 pixels (about 1.5-2x alien size)

### Performance Tracking
Add performance metrics to verify improvement:
```python
self.collision_checks_this_frame = 0
# Increment each time colliderect is called
```

## Expected Behavior

After optimization:
- Same gameplay behavior (no functional changes)
- Fewer collision checks per frame (measure and compare)
- Especially noticeable benefit with many bullets
- Code is clean and well-organized

## Performance Targets

Before optimization (rough estimates):
- Full fleet: ~55 collision checks per bullet per frame
- 3 bullets active: ~165 checks per frame

After optimization:
- Each bullet only checks aliens in nearby cells
- Estimated 5-15 checks per bullet per frame
- 3 bullets active: ~15-45 checks per frame

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```

Tests verify:
- Collision detection still works correctly
- Spatial grid correctly tracks objects
- Performance improvement is measurable
- Edge cases (objects spanning cells) handled
