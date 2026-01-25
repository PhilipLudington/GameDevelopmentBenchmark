/**
 * Stub implementation for julius-048 scrollbar position test
 *
 * This provides the scrollbar_get_thumb_position function for testing.
 */

#include <stdio.h>
#include <stdlib.h>

/**
 * Calculate the pixel position of the scrollbar thumb.
 *
 * BUGGY version (commented):
 *   return (scroll_pos * track_height) / content_height;
 *
 * FIXED version:
 *   return (scroll_pos * track_height + content_height / 2) / content_height;
 *
 * The difference is proper rounding vs truncation.
 */
int scrollbar_get_thumb_position(int scroll_pos, int content_height, int track_height)
{
    if (content_height <= 0) return 0;
    if (scroll_pos < 0) scroll_pos = 0;

    /* FIXED: Add half the divisor before dividing for proper rounding */
    return (scroll_pos * track_height + content_height / 2) / content_height;
}

/**
 * Helper: Calculate position using truncation (the buggy way)
 */
int scrollbar_get_thumb_position_truncated(int scroll_pos, int content_height, int track_height)
{
    if (content_height <= 0) return 0;
    if (scroll_pos < 0) scroll_pos = 0;

    /* Truncation version (buggy) */
    return (scroll_pos * track_height) / content_height;
}

/**
 * Helper: Calculate expected position with rounding
 */
int get_expected_rounded_position(int scroll_pos, int content_height, int track_height)
{
    if (content_height <= 0) return 0;
    if (scroll_pos < 0) scroll_pos = 0;

    /* Proper rounding */
    return (scroll_pos * track_height + content_height / 2) / content_height;
}
