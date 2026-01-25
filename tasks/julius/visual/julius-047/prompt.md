# Bug Report: Button Highlight Color Lookup Wrong

## Summary

When hovering over buttons in the game UI, they display the wrong highlight color. The button uses an incorrect theme color index, causing it to show the border color instead of the intended highlight color.

## Symptoms

- Button hover state shows unexpected color
- The highlight color appears to be the border color
- Different themes show consistent wrong color (always off by one)
- Normal and pressed button states render correctly

## Technical Details

The bug is in `src/graphics/button.c` in the `button_get_highlight_color()` function. When looking up the highlight color from the theme palette, the code uses `COLOR_BUTTON_BORDER` instead of `COLOR_BUTTON_HIGHLIGHT`.

The current (buggy) logic:
```c
color_t button_get_highlight_color(int theme_id)
{
    // BUG: Using COLOR_BUTTON_BORDER (index 3) instead of COLOR_BUTTON_HIGHLIGHT (index 4)
    return theme_colors[theme_id][COLOR_BUTTON_BORDER];
}
```

The correct logic should use the highlight color index:
```c
color_t button_get_highlight_color(int theme_id)
{
    return theme_colors[theme_id][COLOR_BUTTON_HIGHLIGHT];
}
```

## Relevant Code Location

File: `src/graphics/button.c`
Function: `button_get_highlight_color()`

Look for:
- The color index constant used in the array lookup
- `COLOR_BUTTON_BORDER` vs `COLOR_BUTTON_HIGHLIGHT` usage

## Expected Fix

Change the color index from `COLOR_BUTTON_BORDER` to `COLOR_BUTTON_HIGHLIGHT`:

```c
// BUGGY:
return theme_colors[theme_id][COLOR_BUTTON_BORDER];

// FIXED:
return theme_colors[theme_id][COLOR_BUTTON_HIGHLIGHT];
```

## Testing

To verify the fix:
1. Test button highlight color lookup for each button state
2. Verify the correct color index is used
3. Check that other color lookups are not affected

## Constraints

- Only fix the highlight color lookup
- Do not modify the theme color palette
- Ensure the fix applies to all themes consistently
