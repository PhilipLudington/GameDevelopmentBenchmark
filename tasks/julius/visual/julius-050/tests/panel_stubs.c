/**
 * Stub implementation for julius-050 panel text truncation test
 *
 * This provides the panel_get_text_width function for testing.
 */

#include <stdio.h>
#include <stdlib.h>

/**
 * Calculate the available width for text within a panel.
 *
 * BUGGY version (commented):
 *   return panel_width - padding;  // Only subtracts once!
 *
 * FIXED version:
 *   return panel_width - (padding * 2);  // Subtracts for both sides
 */
int panel_get_text_width(int panel_width, int padding)
{
    if (panel_width <= 0) return 0;
    if (padding < 0) padding = 0;

    /* FIXED: Subtract padding for both left and right sides */
    return panel_width - (padding * 2);
}

/**
 * Helper: Calculate width using the buggy method (for comparison)
 */
int panel_get_text_width_buggy(int panel_width, int padding)
{
    if (panel_width <= 0) return 0;
    if (padding < 0) padding = 0;

    /* Buggy: Only subtracts padding once */
    return panel_width - padding;
}

/**
 * Helper: Get expected correct width
 */
int get_expected_text_width(int panel_width, int padding)
{
    if (panel_width <= 0) return 0;
    if (padding < 0) padding = 0;

    return panel_width - (padding * 2);
}
