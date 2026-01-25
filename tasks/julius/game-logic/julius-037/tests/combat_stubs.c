/**
 * Stub implementation for julius-037 tower archer range test
 *
 * This provides mock range calculation for testing elevation handling.
 */

#include <stdio.h>
#include <math.h>

/* Integer square root approximation */
int int_sqrt(int n)
{
    if (n < 0) return 0;
    if (n < 2) return n;

    int result = (int)sqrt((double)n);
    return result;
}

/**
 * Calculate archer range between tower and target
 *
 * BUGGY version: Only uses dx and dy (2D distance)
 * FIXED version: Uses dx, dy, and dz (3D distance)
 */
int calculate_archer_range(int tower_x, int tower_y, int tower_z,
                           int target_x, int target_y, int target_z)
{
    int dx = target_x - tower_x;
    int dy = target_y - tower_y;
    int dz = target_z - tower_z;

    /* FIXED: Include elevation in distance calculation */
    int distance_squared = dx * dx + dy * dy + dz * dz;

    return int_sqrt(distance_squared);
}

/* Check if target is in range */
int tower_can_fire_at(int tower_x, int tower_y, int tower_z,
                      int target_x, int target_y, int target_z,
                      int max_range)
{
    int distance = calculate_archer_range(tower_x, tower_y, tower_z,
                                          target_x, target_y, target_z);
    return distance <= max_range;
}
