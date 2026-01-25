/**
 * Stub implementation for julius-049 cursor hotspot test
 *
 * This provides the cursor_get_click_position function for testing.
 */

#include <stdio.h>
#include <stdlib.h>

/**
 * Calculate the actual click position based on cursor position and hotspot.
 *
 * BUGGY version (commented):
 *   *click_x = cursor_x + hotspot_x;
 *   *click_y = cursor_y + hotspot_y;
 *
 * FIXED version:
 *   *click_x = cursor_x - hotspot_x;
 *   *click_y = cursor_y - hotspot_y;
 *
 * The hotspot is the offset from the cursor image origin to the active click
 * point. To get the click position, we subtract it from the cursor position.
 */
void cursor_get_click_position(int cursor_x, int cursor_y,
                               int hotspot_x, int hotspot_y,
                               int *click_x, int *click_y)
{
    if (!click_x || !click_y) return;

    /* FIXED: Subtract hotspot offset to get actual click position */
    *click_x = cursor_x - hotspot_x;
    *click_y = cursor_y - hotspot_y;
}

/**
 * Helper: Calculate click position using the buggy method (for comparison)
 */
void cursor_get_click_position_buggy(int cursor_x, int cursor_y,
                                      int hotspot_x, int hotspot_y,
                                      int *click_x, int *click_y)
{
    if (!click_x || !click_y) return;

    /* Buggy: Adding instead of subtracting */
    *click_x = cursor_x + hotspot_x;
    *click_y = cursor_y + hotspot_y;
}
