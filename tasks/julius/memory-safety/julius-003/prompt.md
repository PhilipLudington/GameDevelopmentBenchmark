# Bug Report: Sheep Out-of-Bounds Destination

## Summary

Herd animals (sheep and wolves) can receive destination coordinates that fall outside the valid map grid boundaries. This causes out-of-bounds memory access when the game uses these coordinates to access map arrays.

## Symptoms

- Game crash when sheep or wolves move near map edges
- AddressSanitizer reports `heap-buffer-overflow` or similar access violation
- Animals occasionally teleporting or behaving erratically at map boundaries

## Technical Details

The bug is in `src/figuretype/animal.c` in the `figure_sheep_action()` and `figure_wolf_action()` functions. When calculating the destination position for herd animals, the code adds formation layout offsets to the formation's destination coordinates without checking if the result stays within map bounds.

```c
f->destination_x = m->destination_x + formation_layout_position_x(FORMATION_HERD, f->index_in_formation);
f->destination_y = m->destination_y + formation_layout_position_y(FORMATION_HERD, f->index_in_formation);
```

If `m->destination_x` is near the edge of the map and `offset_x` is positive, the result can exceed the map bounds.

## Relevant Code Location

File: `src/figuretype/animal.c`
Functions: `figure_sheep_action()`, `figure_wolf_action()`

Look for where `destination_x` and `destination_y` are calculated using formation positions.

## Expected Fix

The fix should:
1. Add bounds checking to ensure destination coordinates stay within the valid map grid
2. Preferably extract the destination calculation into a reusable helper function
3. Use the existing `map_grid_bound()` function to clamp coordinates

## Testing

To verify the fix:
1. Build with AddressSanitizer enabled
2. Run tests that simulate herd animals at map boundaries
3. Confirm no ASan errors occur and coordinates are properly bounded

## Constraints

- The fix should apply to both sheep and wolves (and any other herd animals)
- Do not change the overall movement behavior beyond bounds clamping
- Maintain compatibility with existing save files
