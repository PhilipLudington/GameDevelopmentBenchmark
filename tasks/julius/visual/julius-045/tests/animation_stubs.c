/**
 * Stub implementation for julius-045 animation offset test
 *
 * This provides the animation_get_screen_position function for testing.
 * The function calculates screen coordinates for animated sprites.
 */

#include <stdio.h>
#include <stdlib.h>

/**
 * Calculate screen position for an animated sprite.
 *
 * The correct calculation order is:
 * 1. Start with base position
 * 2. Add frame offset (animation-specific adjustment)
 * 3. Subtract sprite offset (centers the sprite image)
 *
 * BUGGY version (commented):
 *   screen_x = base_x - sprite_offset_x + frame_offset_x;
 *   screen_y = base_y - sprite_offset_y + frame_offset_y;
 *
 * FIXED version:
 *   screen_x = base_x + frame_offset_x - sprite_offset_x;
 *   screen_y = base_y + frame_offset_y - sprite_offset_y;
 */
void animation_get_screen_position(
    int base_x, int base_y,
    int sprite_offset_x, int sprite_offset_y,
    int frame_offset_x, int frame_offset_y,
    int *screen_x, int *screen_y)
{
    if (!screen_x || !screen_y) return;

    /* FIXED: Add frame offset first, then subtract sprite offset */
    *screen_x = base_x + frame_offset_x - sprite_offset_x;
    *screen_y = base_y + frame_offset_y - sprite_offset_y;
}
