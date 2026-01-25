/**
 * Test for tower archer range elevation bug (julius-037)
 *
 * This test verifies that the range calculation includes
 * elevation (Z coordinate) in the distance formula.
 *
 * Test validation:
 * - On FIXED code: Range accounts for elevation difference
 * - On BUGGY code: Range ignores elevation (2D only)
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* External functions from stubs */
extern int calculate_archer_range(int tower_x, int tower_y, int tower_z,
                                  int target_x, int target_y, int target_z);
extern int tower_can_fire_at(int tower_x, int tower_y, int tower_z,
                             int target_x, int target_y, int target_z,
                             int max_range);

/**
 * Test 1: Target directly below tower (same X,Y, different Z)
 *
 * This is the KEY test. With 2D distance, this would be 0.
 * With 3D distance, it should be the elevation difference.
 */
static int test_vertical_distance(void)
{
    printf("Test: Target directly below tower (vertical distance)...\n");

    /* Tower at (10, 10, 10), target at (10, 10, 0) */
    int range = calculate_archer_range(10, 10, 10,
                                       10, 10, 0);

    printf("  Tower at (10,10,10), Target at (10,10,0)\n");
    printf("  Calculated range: %d\n", range);
    printf("  2D distance would be: 0\n");
    printf("  3D distance should be: 10\n");

    /* With 3D calculation, distance should be 10 (elevation difference) */
    if (range == 0) {
        printf("  FAIL: Range is 0 - elevation is being ignored!\n");
        printf("  This is the bug - only 2D distance is calculated\n");
        return 0;
    }

    if (range != 10) {
        printf("  FAIL: Expected range=10, got %d\n", range);
        return 0;
    }

    printf("  PASS: Elevation correctly affects range\n");
    return 1;
}

/**
 * Test 2: Target above tower
 */
static int test_target_above(void)
{
    printf("Test: Target above tower...\n");

    /* Tower at (5, 5, 0), target at (5, 5, 12) */
    int range = calculate_archer_range(5, 5, 0,
                                       5, 5, 12);

    printf("  Tower at (5,5,0), Target at (5,5,12)\n");
    printf("  Calculated range: %d\n", range);

    if (range != 12) {
        printf("  FAIL: Expected range=12, got %d\n", range);
        return 0;
    }

    printf("  PASS: Target above handled correctly\n");
    return 1;
}

/**
 * Test 3: Diagonal distance with elevation
 *
 * Tower at (0, 0, 0), Target at (3, 4, 0)
 * 2D: sqrt(9 + 16) = sqrt(25) = 5
 * 3D: sqrt(9 + 16 + 0) = 5 (same in this case)
 */
static int test_flat_diagonal(void)
{
    printf("Test: Flat diagonal (3-4-5 triangle)...\n");

    int range = calculate_archer_range(0, 0, 0,
                                       3, 4, 0);

    printf("  Tower at (0,0,0), Target at (3,4,0)\n");
    printf("  Calculated range: %d\n", range);

    if (range != 5) {
        printf("  FAIL: Expected range=5, got %d\n", range);
        return 0;
    }

    printf("  PASS: Flat diagonal correct\n");
    return 1;
}

/**
 * Test 4: 3D diagonal (with elevation)
 *
 * Tower at (0, 0, 0), Target at (2, 2, 1)
 * 3D: sqrt(4 + 4 + 1) = sqrt(9) = 3
 * 2D: sqrt(4 + 4) = sqrt(8) = 2.83
 */
static int test_3d_diagonal(void)
{
    printf("Test: 3D diagonal with elevation...\n");

    int range = calculate_archer_range(0, 0, 0,
                                       2, 2, 1);

    printf("  Tower at (0,0,0), Target at (2,2,1)\n");
    printf("  2D distance: %d\n", (int)sqrt(8.0));
    printf("  3D distance: %d\n", (int)sqrt(9.0));
    printf("  Calculated range: %d\n", range);

    /* 3D distance is 3 */
    if (range != 3) {
        printf("  FAIL: Expected range=3, got %d\n", range);
        if (range == 2) {
            printf("  This suggests elevation is being ignored (2D distance)\n");
        }
        return 0;
    }

    printf("  PASS: 3D diagonal correct\n");
    return 1;
}

/**
 * Test 5: Range check with elevation
 *
 * Tower with range 5 should NOT hit target at horizontal distance 4
 * but vertical distance 5 (total 3D distance = sqrt(41) = 6.4)
 */
static int test_range_check_with_elevation(void)
{
    printf("Test: Range check considering elevation...\n");

    /* Tower at (0,0,5), Target at (4,0,0), max range = 5 */
    /* dx=4, dy=0, dz=5, distance = sqrt(16+0+25) = sqrt(41) = 6.4 (int: 6) */
    int in_range = tower_can_fire_at(0, 0, 5,
                                     4, 0, 0,
                                     5);

    printf("  Tower at (0,0,5), Target at (4,0,0), max_range=5\n");
    printf("  2D distance: 4 (would be in range)\n");
    printf("  3D distance: ~6.4 (should be OUT of range)\n");
    printf("  In range: %s\n", in_range ? "YES" : "NO");

    if (in_range) {
        printf("  FAIL: Target should be OUT of range (3D distance > 5)\n");
        printf("  This suggests elevation is being ignored\n");
        return 0;
    }

    printf("  PASS: Range check correctly considers elevation\n");
    return 1;
}

/**
 * Test 6: Large elevation difference
 */
static int test_large_elevation(void)
{
    printf("Test: Large elevation difference...\n");

    /* Tower at (0,0,100), Target at (0,0,0) */
    int range = calculate_archer_range(0, 0, 100,
                                       0, 0, 0);

    printf("  Tower at (0,0,100), Target at (0,0,0)\n");
    printf("  Calculated range: %d\n", range);

    if (range != 100) {
        printf("  FAIL: Expected range=100, got %d\n", range);
        return 0;
    }

    printf("  PASS: Large elevation handled correctly\n");
    return 1;
}

int main(void)
{
    int passed = 0;
    int total = 6;

    printf("=== Tower Archer Range Elevation Test Suite ===\n");
    printf("Testing 3D distance calculation for range\n");
    printf("Bug: Range calculation ignores elevation (Z coordinate)\n\n");

    if (test_vertical_distance()) passed++;
    printf("\n");

    if (test_target_above()) passed++;
    printf("\n");

    if (test_flat_diagonal()) passed++;
    printf("\n");

    if (test_3d_diagonal()) passed++;
    printf("\n");

    if (test_range_check_with_elevation()) passed++;
    printf("\n");

    if (test_large_elevation()) passed++;

    printf("\n=== Test Results ===\n");
    printf("Results: %d/%d tests passed\n", passed, total);

    return (passed == total) ? 0 : 1;
}
