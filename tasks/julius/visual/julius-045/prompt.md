# Bug Report: Animation Offset Calculation Error

## Summary

Animated sprites are being drawn at incorrect positions on screen. The offset calculation in the animation rendering code subtracts the sprite offset before applying the animation frame offset, when it should add them in the correct order.

## Symptoms

- Animated sprites appear shifted from their expected position
- Characters and objects "jump" to different locations during animation playback
- Static sprites render correctly, but animated versions are displaced
- The visual offset gets worse on sprites with larger frame offsets

## Technical Details

The bug is in `src/graphics/animation.c` in the `animation_get_screen_position()` function. The issue is in how the sprite base offset and animation frame offset are combined.

The current (buggy) logic:
1. Subtracts the sprite base offset from screen coordinates
2. Then adds the animation frame offset
3. This causes incorrect positioning because the operation order is wrong

The correct logic should:
1. Start with the base screen position
2. Add the animation frame offset to account for frame-specific positioning
3. Then subtract the sprite offset to center the image correctly

## Relevant Code Location

File: `src/graphics/animation.c`
Function: `animation_get_screen_position()`

Look for the offset calculation involving:
- `sprite_offset_x` / `sprite_offset_y`
- `frame_offset_x` / `frame_offset_y`
- The order of addition and subtraction operations

## Expected Fix

The fix should correct the order of operations:

```c
// BUGGY:
screen_x = base_x - sprite_offset_x + frame_offset_x;

// FIXED:
screen_x = base_x + frame_offset_x - sprite_offset_x;
```

Or equivalently, ensure the frame offset is applied before the sprite offset in the calculation.

## Testing

To verify the fix:
1. Test with various animation frame offsets (positive and negative)
2. Verify sprite positions match expected coordinates
3. Check that animated sprites align properly with their hitboxes

## Constraints

- The fix should not change the rendering of static (non-animated) sprites
- Maintain backward compatibility with existing animation data
- Only modify the offset calculation, not the underlying animation system
