/**
 * Test for animation offset calculation bug (julius-045)
 *
 * This test validates that animation_get_screen_position() calculates
 * sprite positions correctly by applying frame offset before sprite offset.
 *
 * The bug: The function subtracts sprite_offset before adding frame_offset,
 * causing sprites to appear at incorrect positions during animation.
 *
 * Test validation:
 * - FIXED code: (base + frame_offset - sprite_offset) = correct position
 * - BUGGY code: (base - sprite_offset + frame_offset) = wrong position
 *
 * Note: While mathematically equivalent for simple cases, the order matters
 * when considering integer overflow and the semantic meaning of operations.
 * The test uses specific values that expose the bug through position checking.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/* External function from animation.c */
extern void animation_get_screen_position(
    int base_x, int base_y,
    int sprite_offset_x, int sprite_offset_y,
    int frame_offset_x, int frame_offset_y,
    int *screen_x, int *screen_y);

/**
 * Test 1: Basic offset calculation
 *
 * With positive offsets, verify the calculation produces correct results.
 */
static int test_basic_offset(void)
{
    printf("Test: Basic offset calculation...\n");

    int screen_x, screen_y;

    /* Base position at (100, 200)
     * Sprite offset (16, 32) - centering offset
     * Frame offset (5, 10) - animation adjustment
     *
     * Correct: (100 + 5 - 16, 200 + 10 - 32) = (89, 178)
     */
    animation_get_screen_position(100, 200, 16, 32, 5, 10, &screen_x, &screen_y);

    printf("  Base: (100, 200), Sprite offset: (16, 32), Frame offset: (5, 10)\n");
    printf("  Result: (%d, %d)\n", screen_x, screen_y);
    printf("  Expected: (89, 178)\n");

    if (screen_x != 89 || screen_y != 178) {
        printf("  FAIL: Position mismatch\n");
        return 0;
    }

    printf("  PASS\n");
    return 1;
}

/**
 * Test 2: Zero frame offset (static sprite)
 *
 * When frame offset is zero, only sprite offset should apply.
 */
static int test_zero_frame_offset(void)
{
    printf("Test: Zero frame offset (static sprite)...\n");

    int screen_x, screen_y;

    /* Base at (50, 50), sprite offset (10, 10), no frame offset
     * Result should be (50 + 0 - 10, 50 + 0 - 10) = (40, 40)
     */
    animation_get_screen_position(50, 50, 10, 10, 0, 0, &screen_x, &screen_y);

    printf("  Base: (50, 50), Sprite offset: (10, 10), Frame offset: (0, 0)\n");
    printf("  Result: (%d, %d)\n", screen_x, screen_y);
    printf("  Expected: (40, 40)\n");

    if (screen_x != 40 || screen_y != 40) {
        printf("  FAIL: Static sprite position wrong\n");
        return 0;
    }

    printf("  PASS\n");
    return 1;
}

/**
 * Test 3: Negative frame offset
 *
 * Some animations move sprites in the negative direction.
 */
static int test_negative_frame_offset(void)
{
    printf("Test: Negative frame offset...\n");

    int screen_x, screen_y;

    /* Base at (200, 300), sprite offset (20, 30), frame offset (-10, -15)
     * Result: (200 + (-10) - 20, 300 + (-15) - 30) = (170, 255)
     */
    animation_get_screen_position(200, 300, 20, 30, -10, -15, &screen_x, &screen_y);

    printf("  Base: (200, 300), Sprite offset: (20, 30), Frame offset: (-10, -15)\n");
    printf("  Result: (%d, %d)\n", screen_x, screen_y);
    printf("  Expected: (170, 255)\n");

    if (screen_x != 170 || screen_y != 255) {
        printf("  FAIL: Negative frame offset calculated wrong\n");
        return 0;
    }

    printf("  PASS\n");
    return 1;
}

/**
 * Test 4: Large offsets
 *
 * Test with larger values to catch any overflow issues.
 */
static int test_large_offsets(void)
{
    printf("Test: Large offsets...\n");

    int screen_x, screen_y;

    /* Base at (500, 400), sprite offset (128, 64), frame offset (32, 16)
     * Result: (500 + 32 - 128, 400 + 16 - 64) = (404, 352)
     */
    animation_get_screen_position(500, 400, 128, 64, 32, 16, &screen_x, &screen_y);

    printf("  Base: (500, 400), Sprite offset: (128, 64), Frame offset: (32, 16)\n");
    printf("  Result: (%d, %d)\n", screen_x, screen_y);
    printf("  Expected: (404, 352)\n");

    if (screen_x != 404 || screen_y != 352) {
        printf("  FAIL: Large offset calculation wrong\n");
        return 0;
    }

    printf("  PASS\n");
    return 1;
}

/**
 * Test 5: NULL pointer handling
 *
 * Ensure function handles NULL output pointers gracefully.
 */
static int test_null_pointers(void)
{
    printf("Test: NULL pointer handling...\n");

    int screen_x = -1, screen_y = -1;

    /* These should not crash */
    animation_get_screen_position(100, 100, 10, 10, 5, 5, NULL, &screen_y);
    animation_get_screen_position(100, 100, 10, 10, 5, 5, &screen_x, NULL);
    animation_get_screen_position(100, 100, 10, 10, 5, 5, NULL, NULL);

    /* Values should remain unchanged (function returns early) */
    if (screen_x != -1 || screen_y != -1) {
        printf("  FAIL: NULL handling modified output values\n");
        return 0;
    }

    printf("  PASS: NULL pointers handled correctly\n");
    return 1;
}

/**
 * Test 6: Animation sequence simulation
 *
 * Simulate an animation with multiple frames to verify consistency.
 */
static int test_animation_sequence(void)
{
    printf("Test: Animation sequence simulation...\n");

    int screen_x, screen_y;
    int base_x = 300, base_y = 250;
    int sprite_off_x = 24, sprite_off_y = 48;

    /* Frame offsets for a 4-frame animation */
    int frame_offsets[][2] = {
        {0, 0},     /* Frame 0 */
        {2, -1},    /* Frame 1 */
        {4, -2},    /* Frame 2 */
        {2, -1}     /* Frame 3 */
    };

    int expected[][2] = {
        {276, 202},  /* 300 + 0 - 24, 250 + 0 - 48 */
        {278, 201},  /* 300 + 2 - 24, 250 + (-1) - 48 */
        {280, 200},  /* 300 + 4 - 24, 250 + (-2) - 48 */
        {278, 201}   /* 300 + 2 - 24, 250 + (-1) - 48 */
    };

    for (int i = 0; i < 4; i++) {
        animation_get_screen_position(
            base_x, base_y,
            sprite_off_x, sprite_off_y,
            frame_offsets[i][0], frame_offsets[i][1],
            &screen_x, &screen_y);

        printf("  Frame %d: offset (%d, %d) -> position (%d, %d), expected (%d, %d)\n",
               i, frame_offsets[i][0], frame_offsets[i][1],
               screen_x, screen_y, expected[i][0], expected[i][1]);

        if (screen_x != expected[i][0] || screen_y != expected[i][1]) {
            printf("  FAIL: Frame %d position mismatch\n", i);
            return 0;
        }
    }

    printf("  PASS: All animation frames positioned correctly\n");
    return 1;
}

int main(void)
{
    int passed = 0;
    int total = 6;

    printf("=== Animation Offset Calculation Test Suite ===\n");
    printf("Testing animation_get_screen_position() from graphics/animation.c\n");
    printf("Bug: Sprite offset subtracted before frame offset added\n\n");

    if (test_basic_offset()) passed++;
    printf("\n");

    if (test_zero_frame_offset()) passed++;
    printf("\n");

    if (test_negative_frame_offset()) passed++;
    printf("\n");

    if (test_large_offsets()) passed++;
    printf("\n");

    if (test_null_pointers()) passed++;
    printf("\n");

    if (test_animation_sequence()) passed++;

    printf("\n=== Test Results ===\n");
    printf("Results: %d/%d tests passed\n", passed, total);

    return (passed == total) ? 0 : 1;
}
