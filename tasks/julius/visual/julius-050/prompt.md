# Bug Report: Panel Text Truncation

## Summary

Text displayed in UI panels is being cut off prematurely. The available width calculation for text doesn't account for the panel's internal padding, causing text to be truncated before it reaches the actual edge of the usable area.

## Symptoms

- Text in panels appears cut off before reaching the right edge
- Long strings are truncated more than necessary
- The issue is more visible with wider padding values
- Panel content doesn't utilize full available width

## Technical Details

The bug is in `src/graphics/panel.c` in the `panel_get_text_width()` function. When calculating available width for text, the code subtracts padding only once instead of accounting for both left and right padding.

The current (buggy) logic:
```c
int panel_get_text_width(int panel_width, int padding)
{
    // BUG: Only subtracts padding once, should subtract for both sides
    return panel_width - padding;
}
```

The correct logic should subtract padding from both sides:
```c
int panel_get_text_width(int panel_width, int padding)
{
    // Subtract padding from both left and right sides
    return panel_width - (padding * 2);
}
```

## Relevant Code Location

File: `src/graphics/panel.c`
Function: `panel_get_text_width()`

Look for:
- The calculation that determines available text width
- Whether padding is subtracted once or twice

## Expected Fix

Multiply the padding by 2 to account for both sides:

```c
// BUGGY:
return panel_width - padding;

// FIXED:
return panel_width - (padding * 2);
```

Or equivalently:
```c
return panel_width - padding - padding;
```

## Testing

To verify the fix:
1. Test with various panel widths and padding values
2. Verify available width equals panel_width - 2*padding
3. Check that text fits correctly within the calculated area

## Constraints

- Only fix the width calculation
- Do not modify the text rendering itself
- Ensure the fix works for all padding values including 0
