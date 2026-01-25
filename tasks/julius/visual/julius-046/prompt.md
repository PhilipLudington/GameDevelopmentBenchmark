# Bug Report: Amphitheater Spectator Position Offset

## Summary

Spectators watching events at the amphitheater are rendered at incorrect positions. They appear floating above or beside their intended seats due to an error in the tile-to-pixel coordinate conversion for spectator figures.

## Symptoms

- Spectators appear shifted from their seats in the amphitheater
- The offset is consistent (always shifted by the same amount)
- Other figures in the game render at correct positions
- The bug only affects spectator figures in arena buildings

## Technical Details

The bug is in `src/figure/figure_draw.c` in the `figure_draw_spectator()` function. When converting the spectator's tile position to screen pixel coordinates, the code uses an incorrect multiplier for the tile offset.

The current (buggy) logic:
```c
// Uses 30 pixels per tile instead of the correct 15
int pixel_x = base_x + (tile_offset_x * 30);
int pixel_y = base_y + (tile_offset_y * 15);
```

The correct logic should use 15 pixels for both X and Y in isometric view:
```c
int pixel_x = base_x + (tile_offset_x * 15);
int pixel_y = base_y + (tile_offset_y * 15);
```

## Relevant Code Location

File: `src/figure/figure_draw.c`
Function: `figure_draw_spectator()`

Look for:
- Tile offset to pixel conversion
- The multiplier constant used (30 vs 15)
- Separate X and Y calculations

## Expected Fix

Change the X-axis pixel multiplier from 30 to 15 to match the isometric tile size:

```c
// BUGGY:
int pixel_x = base_x + (tile_offset_x * 30);

// FIXED:
int pixel_x = base_x + (tile_offset_x * 15);
```

## Testing

To verify the fix:
1. Test with spectators at various tile offsets
2. Verify spectators align with their seat positions
3. Check that the fix doesn't affect other figure types

## Constraints

- Only fix the spectator positioning calculation
- Do not modify the general figure rendering system
- Maintain compatibility with the isometric coordinate system
