# Bug Report: Scrollbar Position Rounding Causes Jitter

## Summary

The scrollbar thumb position jitters visually during smooth scrolling. This is caused by integer division truncation when converting scroll position to pixel position, which discards the fractional part and causes inconsistent rounding.

## Symptoms

- Scrollbar thumb appears to "jump" or "jitter" during scrolling
- The jitter is more noticeable with longer content lists
- Scrollbar position doesn't smoothly correspond to content position
- Fast scrolling makes the jitter less noticeable, slow scrolling makes it obvious

## Technical Details

The bug is in `src/graphics/scrollbar.c` in the `scrollbar_get_thumb_position()` function. The calculation uses simple integer division which truncates, causing accumulated rounding errors.

The current (buggy) logic:
```c
int scrollbar_get_thumb_position(int scroll_pos, int content_height, int track_height)
{
    // Simple division truncates, causing jitter
    return (scroll_pos * track_height) / content_height;
}
```

The correct logic should use proper rounding:
```c
int scrollbar_get_thumb_position(int scroll_pos, int content_height, int track_height)
{
    // Add half divisor before division for proper rounding
    return (scroll_pos * track_height + content_height / 2) / content_height;
}
```

## Relevant Code Location

File: `src/graphics/scrollbar.c`
Function: `scrollbar_get_thumb_position()`

Look for:
- The division operation that converts scroll position to pixel position
- Missing rounding adjustment before division

## Expected Fix

Add proper rounding by adding half the divisor before dividing:

```c
// BUGGY (truncates):
return (scroll_pos * track_height) / content_height;

// FIXED (rounds properly):
return (scroll_pos * track_height + content_height / 2) / content_height;
```

## Testing

To verify the fix:
1. Test with various scroll positions and content heights
2. Verify smooth position progression without jumps
3. Check boundary cases (position 0, max position)

## Constraints

- Only fix the rounding issue, not the overall scrollbar logic
- Maintain integer arithmetic (no floating point)
- Ensure boundary positions (0 and max) remain correct
