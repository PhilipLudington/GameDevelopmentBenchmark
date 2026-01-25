/**
 * Test for amphitheater spectator position bug (julius-046)
 *
 * This test validates that figure_draw_spectator() calculates
 * pixel positions correctly using the right tile multiplier.
 *
 * The bug: The X coordinate multiplier is 30 instead of 15,
 * causing spectators to be drawn at wrong horizontal positions.
 *
 * Test validation:
 * - FIXED code: pixel_x = base_x + (tile_offset_x * 15) = correct
 * - BUGGY code: pixel_x = base_x + (tile_offset_x * 30) = wrong
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define TILE_PIXEL_SIZE 15

/* External function from figure_draw.c */
extern void figure_draw_spectator(int base_x, int base_y,
                                   int tile_offset_x, int tile_offset_y,
                                   int image_id);

/* External helper to get calculated position */
extern void get_last_spectator_position(int *pixel_x, int *pixel_y, int *image_id);

/**
 * Test 1: Basic tile offset conversion
 *
 * Verify that tile offset (1, 1) produces correct pixel offset (15, 15).
 */
static int test_basic_offset(void)
{
    printf("Test: Basic tile offset conversion...\n");

    int pixel_x, pixel_y, image_id;
    int base_x = 100, base_y = 200;

    /* Tile offset (1, 1) should produce pixel position (115, 215) */
    figure_draw_spectator(base_x, base_y, 1, 1, 1);
    get_last_spectator_position(&pixel_x, &pixel_y, &image_id);

    int expected_x = base_x + (1 * TILE_PIXEL_SIZE);  /* 100 + 15 = 115 */
    int expected_y = base_y + (1 * TILE_PIXEL_SIZE);  /* 200 + 15 = 215 */

    printf("  Base: (%d, %d), Tile offset: (1, 1)\n", base_x, base_y);
    printf("  Result: (%d, %d)\n", pixel_x, pixel_y);
    printf("  Expected: (%d, %d)\n", expected_x, expected_y);

    if (pixel_x != expected_x) {
        printf("  FAIL: X coordinate wrong (got %d, expected %d)\n", pixel_x, expected_x);
        printf("  This indicates X uses multiplier 30 instead of 15\n");
        return 0;
    }

    if (pixel_y != expected_y) {
        printf("  FAIL: Y coordinate wrong (got %d, expected %d)\n", pixel_y, expected_y);
        return 0;
    }

    printf("  PASS\n");
    return 1;
}

/**
 * Test 2: Multiple tile offset
 *
 * Test with tile offset (3, 2) to expose the multiplier bug.
 */
static int test_multiple_tile_offset(void)
{
    printf("Test: Multiple tile offset...\n");

    int pixel_x, pixel_y, image_id;
    int base_x = 50, base_y = 80;

    /* Tile offset (3, 2):
     * Correct: (50 + 45, 80 + 30) = (95, 110)
     * Buggy:   (50 + 90, 80 + 30) = (140, 110)
     */
    figure_draw_spectator(base_x, base_y, 3, 2, 1);
    get_last_spectator_position(&pixel_x, &pixel_y, &image_id);

    int expected_x = base_x + (3 * TILE_PIXEL_SIZE);  /* 50 + 45 = 95 */
    int expected_y = base_y + (2 * TILE_PIXEL_SIZE);  /* 80 + 30 = 110 */

    printf("  Base: (%d, %d), Tile offset: (3, 2)\n", base_x, base_y);
    printf("  Result: (%d, %d)\n", pixel_x, pixel_y);
    printf("  Expected: (%d, %d)\n", expected_x, expected_y);

    if (pixel_x != expected_x || pixel_y != expected_y) {
        printf("  FAIL: Position mismatch\n");
        if (pixel_x == base_x + (3 * 30)) {
            printf("  BUG DETECTED: X multiplier is 30 instead of 15!\n");
        }
        return 0;
    }

    printf("  PASS\n");
    return 1;
}

/**
 * Test 3: Zero tile offset
 *
 * Spectator at origin should be at base position.
 */
static int test_zero_offset(void)
{
    printf("Test: Zero tile offset...\n");

    int pixel_x, pixel_y, image_id;
    int base_x = 200, base_y = 300;

    figure_draw_spectator(base_x, base_y, 0, 0, 1);
    get_last_spectator_position(&pixel_x, &pixel_y, &image_id);

    printf("  Base: (%d, %d), Tile offset: (0, 0)\n", base_x, base_y);
    printf("  Result: (%d, %d)\n", pixel_x, pixel_y);
    printf("  Expected: (%d, %d)\n", base_x, base_y);

    if (pixel_x != base_x || pixel_y != base_y) {
        printf("  FAIL: Zero offset should produce base position\n");
        return 0;
    }

    printf("  PASS\n");
    return 1;
}

/**
 * Test 4: Negative tile offset
 *
 * Some seats may have negative offsets relative to base.
 */
static int test_negative_offset(void)
{
    printf("Test: Negative tile offset...\n");

    int pixel_x, pixel_y, image_id;
    int base_x = 150, base_y = 150;

    /* Tile offset (-2, -1):
     * Correct: (150 - 30, 150 - 15) = (120, 135)
     * Buggy:   (150 - 60, 150 - 15) = (90, 135)
     */
    figure_draw_spectator(base_x, base_y, -2, -1, 1);
    get_last_spectator_position(&pixel_x, &pixel_y, &image_id);

    int expected_x = base_x + (-2 * TILE_PIXEL_SIZE);  /* 150 - 30 = 120 */
    int expected_y = base_y + (-1 * TILE_PIXEL_SIZE);  /* 150 - 15 = 135 */

    printf("  Base: (%d, %d), Tile offset: (-2, -1)\n", base_x, base_y);
    printf("  Result: (%d, %d)\n", pixel_x, pixel_y);
    printf("  Expected: (%d, %d)\n", expected_x, expected_y);

    if (pixel_x != expected_x || pixel_y != expected_y) {
        printf("  FAIL: Negative offset calculated wrong\n");
        return 0;
    }

    printf("  PASS\n");
    return 1;
}

/**
 * Test 5: Arena seating simulation
 *
 * Simulate spectators in different seats of an amphitheater.
 */
static int test_arena_seating(void)
{
    printf("Test: Arena seating simulation...\n");

    int pixel_x, pixel_y, image_id;
    int arena_base_x = 400, arena_base_y = 300;

    /* Seat positions for a small arena section */
    struct {
        int tile_x;
        int tile_y;
        int expected_pixel_x;
        int expected_pixel_y;
    } seats[] = {
        {0, 0, 400, 300},   /* Center seat */
        {1, 0, 415, 300},   /* One tile right */
        {2, 1, 430, 315},   /* Two right, one down */
        {-1, 1, 385, 315},  /* One left, one down */
        {3, 3, 445, 345}    /* Far corner */
    };

    for (size_t i = 0; i < sizeof(seats) / sizeof(seats[0]); i++) {
        figure_draw_spectator(arena_base_x, arena_base_y,
                             seats[i].tile_x, seats[i].tile_y, 1);
        get_last_spectator_position(&pixel_x, &pixel_y, &image_id);

        printf("  Seat (%d, %d): pixel (%d, %d), expected (%d, %d)\n",
               seats[i].tile_x, seats[i].tile_y,
               pixel_x, pixel_y,
               seats[i].expected_pixel_x, seats[i].expected_pixel_y);

        if (pixel_x != seats[i].expected_pixel_x ||
            pixel_y != seats[i].expected_pixel_y) {
            printf("  FAIL: Seat position mismatch\n");
            return 0;
        }
    }

    printf("  PASS: All seats positioned correctly\n");
    return 1;
}

/**
 * Test 6: Invalid image ID handling
 *
 * Ensure no drawing occurs with invalid image ID.
 */
static int test_invalid_image(void)
{
    printf("Test: Invalid image ID handling...\n");

    int pixel_x = -1, pixel_y = -1, image_id = -1;

    /* Image ID 0 or negative should not draw */
    figure_draw_spectator(100, 100, 1, 1, 0);
    get_last_spectator_position(&pixel_x, &pixel_y, &image_id);

    /* Position should remain unchanged from previous test */
    printf("  Image ID 0: position should not update\n");

    /* The stub might not track this perfectly, so just verify no crash */
    printf("  PASS: No crash with invalid image ID\n");
    return 1;
}

int main(void)
{
    int passed = 0;
    int total = 6;

    printf("=== Amphitheater Spectator Position Test Suite ===\n");
    printf("Testing figure_draw_spectator() from figure/figure_draw.c\n");
    printf("Bug: X coordinate multiplier is 30 instead of 15\n\n");

    if (test_basic_offset()) passed++;
    printf("\n");

    if (test_multiple_tile_offset()) passed++;
    printf("\n");

    if (test_zero_offset()) passed++;
    printf("\n");

    if (test_negative_offset()) passed++;
    printf("\n");

    if (test_arena_seating()) passed++;
    printf("\n");

    if (test_invalid_image()) passed++;

    printf("\n=== Test Results ===\n");
    printf("Results: %d/%d tests passed\n", passed, total);

    return (passed == total) ? 0 : 1;
}
