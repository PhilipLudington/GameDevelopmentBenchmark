# Bug Report: Cursor Hotspot Offset

## Summary

Mouse clicks are registered at a position different from where the cursor visually appears. The cursor's hotspot (the precise click point) is calculated with the wrong sign on the offset, causing clicks to miss their intended targets.

## Symptoms

- Clicking on buttons requires clicking slightly to the side of them
- The offset is consistent across all cursor types
- Users report needing to "aim" differently from where the cursor appears
- The problem is worse with cursors that have large hotspot offsets

## Technical Details

The bug is in `src/input/cursor.c` in the `cursor_get_click_position()` function. The hotspot offset is added to the cursor position when it should be subtracted (or vice versa).

The current (buggy) logic:
```c
void cursor_get_click_position(int cursor_x, int cursor_y,
                               int hotspot_x, int hotspot_y,
                               int *click_x, int *click_y)
{
    // BUG: Adding hotspot offset instead of subtracting
    *click_x = cursor_x + hotspot_x;
    *click_y = cursor_y + hotspot_y;
}
```

The correct logic should subtract the hotspot offset:
```c
void cursor_get_click_position(int cursor_x, int cursor_y,
                               int hotspot_x, int hotspot_y,
                               int *click_x, int *click_y)
{
    // The hotspot defines where in the cursor image the click point is
    // So we subtract it from the cursor position to get the actual click point
    *click_x = cursor_x - hotspot_x;
    *click_y = cursor_y - hotspot_y;
}
```

## Relevant Code Location

File: `src/input/cursor.c`
Function: `cursor_get_click_position()`

Look for:
- The arithmetic operation applying the hotspot offset
- Whether it uses + or - for the offset calculation

## Expected Fix

Change the addition to subtraction for the hotspot offset:

```c
// BUGGY:
*click_x = cursor_x + hotspot_x;
*click_y = cursor_y + hotspot_y;

// FIXED:
*click_x = cursor_x - hotspot_x;
*click_y = cursor_y - hotspot_y;
```

## Testing

To verify the fix:
1. Test with various hotspot offsets (including 0, positive, negative)
2. Verify click position matches expected coordinates
3. Check that standard arrow cursor (hotspot at 0,0) is unaffected

## Constraints

- Only fix the hotspot offset calculation
- Do not modify the cursor drawing code
- Ensure the fix works for all cursor types
