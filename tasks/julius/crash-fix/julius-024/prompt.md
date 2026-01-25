# Bug Report: Build menu hitbox calculation error

## Summary

The build menu's button hitbox calculation is off by one row, causing clicks on menu items to select the wrong building type. This is especially problematic when clicking near button boundaries.

## Environment

- Project: Julius (Caesar III reimplementation)
- File: `src/window/build_menu.c`
- Severity: Medium (incorrect UI behavior, frustrating gameplay)

## Bug Description

The build menu displays building options in a grid. When calculating which button was clicked, the code uses the wrong formula to convert mouse coordinates to button indices.

```c
#define BUTTON_WIDTH 64
#define BUTTON_HEIGHT 52
#define BUTTONS_PER_ROW 4

int get_clicked_button(int mouse_x, int mouse_y, int menu_x, int menu_y)
{
    int relative_x = mouse_x - menu_x;
    int relative_y = mouse_y - menu_y;

    // BUG: Integer division should use button dimensions correctly
    int col = relative_x / BUTTON_WIDTH;
    int row = relative_y / BUTTON_HEIGHT + 1;  // +1 is wrong!

    return row * BUTTONS_PER_ROW + col;
}
```

The `+ 1` in the row calculation causes the returned button index to be off by one row.

## Steps to Reproduce

1. Open the build menu
2. Click on the first row of building options
3. Second row building is selected instead
4. Click on last row - may crash if index exceeds array bounds

## Expected Behavior

Clicking on a menu button should select that exact button. The conversion from coordinates to button index should be:
```
row = relative_y / BUTTON_HEIGHT  // No offset
```

## Current Behavior

All button clicks are offset by one row:
- Clicking row 0 selects row 1
- Clicking row 1 selects row 2
- Clicking last row may cause array index out of bounds

## Relevant Code

Look at `src/window/build_menu.c`:
- `get_clicked_button()` function
- Mouse coordinate to button index conversion
- Related hitbox calculation functions

## Suggested Fix Approach

Remove the erroneous `+ 1`:

```c
int get_clicked_button(int mouse_x, int mouse_y, int menu_x, int menu_y)
{
    int relative_x = mouse_x - menu_x;
    int relative_y = mouse_y - menu_y;

    int col = relative_x / BUTTON_WIDTH;
    int row = relative_y / BUTTON_HEIGHT;  // FIXED: No +1

    return row * BUTTONS_PER_ROW + col;
}
```

## Your Task

Fix the hitbox calculation by correcting the row calculation. Your fix should:

1. Remove the incorrect `+ 1` offset from row calculation
2. Ensure clicking any button returns the correct index
3. Not change the column calculation (which is correct)
4. Handle edge cases (clicks outside menu bounds)

Provide your fix as a unified diff (patch).
