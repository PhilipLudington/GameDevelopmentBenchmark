/**
 * Stub implementation for julius-046 spectator position test
 *
 * This provides the figure_draw_spectator function for testing.
 * We capture the calculated pixel position instead of actually drawing.
 */

#include <stdio.h>
#include <stdlib.h>

#define TILE_PIXEL_SIZE 15

/* Global to capture the calculated position */
static int g_last_pixel_x = 0;
static int g_last_pixel_y = 0;
static int g_last_image_id = 0;

/**
 * Get the last calculated pixel position (for testing)
 */
void get_last_spectator_position(int *pixel_x, int *pixel_y, int *image_id)
{
    if (pixel_x) *pixel_x = g_last_pixel_x;
    if (pixel_y) *pixel_y = g_last_pixel_y;
    if (image_id) *image_id = g_last_image_id;
}

/**
 * Stub for image_draw - just captures the position
 */
static void image_draw(int image_id, int x, int y)
{
    g_last_pixel_x = x;
    g_last_pixel_y = y;
    g_last_image_id = image_id;
}

/**
 * Draw a spectator figure at an amphitheater or arena.
 *
 * BUGGY version (commented):
 *   int pixel_x = base_x + (tile_offset_x * 30);
 *
 * FIXED version:
 *   int pixel_x = base_x + (tile_offset_x * TILE_PIXEL_SIZE);
 */
void figure_draw_spectator(int base_x, int base_y,
                           int tile_offset_x, int tile_offset_y,
                           int image_id)
{
    /* FIXED: Using TILE_PIXEL_SIZE (15) for both X and Y */
    int pixel_x = base_x + (tile_offset_x * TILE_PIXEL_SIZE);
    int pixel_y = base_y + (tile_offset_y * TILE_PIXEL_SIZE);

    if (image_id > 0) {
        image_draw(image_id, pixel_x, pixel_y);
    }
}
