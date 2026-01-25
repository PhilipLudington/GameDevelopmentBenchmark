/**
 * Test for scrollbar position rounding bug (julius-048)
 *
 * This test validates that scrollbar_get_thumb_position() uses
 * proper rounding instead of truncation.
 *
 * The bug: Simple integer division truncates, causing the scrollbar
 * thumb to jitter during smooth scrolling.
 *
 * Test validation:
 * - FIXED code: Uses (scroll_pos * track + content/2) / content -> smooth
 * - BUGGY code: Uses (scroll_pos * track) / content -> jittery
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

/* External functions from scrollbar.c */
extern int scrollbar_get_thumb_position(int scroll_pos, int content_height, int track_height);
extern int get_expected_rounded_position(int scroll_pos, int content_height, int track_height);
extern int scrollbar_get_thumb_position_truncated(int scroll_pos, int content_height, int track_height);

/**
 * Test 1: Basic rounding vs truncation
 *
 * With specific values, rounding should give different result than truncation.
 */
static int test_basic_rounding(void)
{
    printf("Test: Basic rounding vs truncation...\n");

    /* Values chosen where truncation and rounding give different results:
     * scroll=50, content=100, track=150
     * Exact: 50 * 150 / 100 = 75.0 (no difference)
     *
     * scroll=33, content=100, track=150
     * Exact: 33 * 150 / 100 = 49.5
     * Truncated: 49
     * Rounded: 50
     */
    int scroll_pos = 33;
    int content_height = 100;
    int track_height = 150;

    int result = scrollbar_get_thumb_position(scroll_pos, content_height, track_height);
    int expected = get_expected_rounded_position(scroll_pos, content_height, track_height);
    int truncated = scrollbar_get_thumb_position_truncated(scroll_pos, content_height, track_height);

    printf("  scroll=%d, content=%d, track=%d\n", scroll_pos, content_height, track_height);
    printf("  Result: %d\n", result);
    printf("  Expected (rounded): %d\n", expected);
    printf("  Truncated would be: %d\n", truncated);

    if (result != expected) {
        printf("  FAIL: Result doesn't match rounded expectation\n");
        return 0;
    }

    if (result == truncated && truncated != expected) {
        printf("  FAIL: Result matches truncated value (bug!)\n");
        return 0;
    }

    printf("  PASS\n");
    return 1;
}

/**
 * Test 2: Position at 50% should be exact
 *
 * When scroll is at 50%, the position should be exactly half track height.
 */
static int test_midpoint_exact(void)
{
    printf("Test: Midpoint position exactness...\n");

    int content_height = 200;
    int track_height = 100;
    int scroll_pos = 100;  /* 50% of content */

    int result = scrollbar_get_thumb_position(scroll_pos, content_height, track_height);
    int expected = 50;  /* 50% of track */

    printf("  scroll=%d (50%% of %d), track=%d\n", scroll_pos, content_height, track_height);
    printf("  Result: %d, Expected: %d\n", result, expected);

    if (result != expected) {
        printf("  FAIL: Midpoint not exact\n");
        return 0;
    }

    printf("  PASS\n");
    return 1;
}

/**
 * Test 3: Boundary cases (0 and max)
 */
static int test_boundary_cases(void)
{
    printf("Test: Boundary cases...\n");

    int content_height = 500;
    int track_height = 200;

    /* Position 0 should always be 0 */
    int pos_zero = scrollbar_get_thumb_position(0, content_height, track_height);
    printf("  Position 0: %d (expected 0)\n", pos_zero);

    if (pos_zero != 0) {
        printf("  FAIL: Position 0 should map to 0\n");
        return 0;
    }

    /* Maximum position should be close to track height */
    int pos_max = scrollbar_get_thumb_position(content_height, content_height, track_height);
    printf("  Position max: %d (expected %d)\n", pos_max, track_height);

    if (pos_max != track_height) {
        printf("  FAIL: Max position should map to track height\n");
        return 0;
    }

    printf("  PASS\n");
    return 1;
}

/**
 * Test 4: Smooth progression
 *
 * As scroll position increases by 1, thumb position should progress smoothly
 * without backward jumps (which would indicate jitter).
 */
static int test_smooth_progression(void)
{
    printf("Test: Smooth progression without jitter...\n");

    int content_height = 1000;
    int track_height = 300;

    int prev_pos = -1;
    int jitter_count = 0;

    for (int scroll = 0; scroll <= content_height; scroll++) {
        int pos = scrollbar_get_thumb_position(scroll, content_height, track_height);

        if (prev_pos >= 0) {
            /* Position should never decrease as scroll increases */
            if (pos < prev_pos) {
                jitter_count++;
                if (jitter_count <= 3) {
                    printf("  Jitter at scroll=%d: pos went from %d to %d\n",
                           scroll, prev_pos, pos);
                }
            }
        }
        prev_pos = pos;
    }

    if (jitter_count > 0) {
        printf("  FAIL: Detected %d position decreases (jitter)\n", jitter_count);
        return 0;
    }

    printf("  PASS: No backward jumps detected\n");
    return 1;
}

/**
 * Test 5: Rounding consistency
 *
 * Compare actual results with expected rounded values across many positions.
 */
static int test_rounding_consistency(void)
{
    printf("Test: Rounding consistency across positions...\n");

    int content_height = 317;  /* Prime number for challenging division */
    int track_height = 200;

    int mismatches = 0;

    for (int scroll = 0; scroll <= content_height; scroll += 7) {
        int result = scrollbar_get_thumb_position(scroll, content_height, track_height);
        int expected = get_expected_rounded_position(scroll, content_height, track_height);

        if (result != expected) {
            mismatches++;
            if (mismatches <= 3) {
                printf("  Mismatch at scroll=%d: got %d, expected %d\n",
                       scroll, result, expected);
            }
        }
    }

    if (mismatches > 0) {
        printf("  FAIL: %d positions don't match rounded expectation\n", mismatches);
        return 0;
    }

    printf("  PASS: All positions match rounded values\n");
    return 1;
}

/**
 * Test 6: Edge case - negative scroll position
 */
static int test_negative_scroll(void)
{
    printf("Test: Negative scroll position handling...\n");

    int result = scrollbar_get_thumb_position(-10, 100, 200);

    printf("  scroll=-10: result=%d (expected 0)\n", result);

    if (result != 0) {
        printf("  FAIL: Negative scroll should clamp to 0\n");
        return 0;
    }

    printf("  PASS\n");
    return 1;
}

/**
 * Test 7: Edge case - zero/invalid content height
 */
static int test_zero_content(void)
{
    printf("Test: Zero content height handling...\n");

    int result_zero = scrollbar_get_thumb_position(50, 0, 200);
    int result_neg = scrollbar_get_thumb_position(50, -100, 200);

    printf("  content=0: result=%d (expected 0)\n", result_zero);
    printf("  content=-100: result=%d (expected 0)\n", result_neg);

    if (result_zero != 0 || result_neg != 0) {
        printf("  FAIL: Invalid content height should return 0\n");
        return 0;
    }

    printf("  PASS\n");
    return 1;
}

int main(void)
{
    int passed = 0;
    int total = 7;

    printf("=== Scrollbar Position Rounding Test Suite ===\n");
    printf("Testing scrollbar_get_thumb_position() from graphics/scrollbar.c\n");
    printf("Bug: Integer division truncates instead of rounding\n\n");

    if (test_basic_rounding()) passed++;
    printf("\n");

    if (test_midpoint_exact()) passed++;
    printf("\n");

    if (test_boundary_cases()) passed++;
    printf("\n");

    if (test_smooth_progression()) passed++;
    printf("\n");

    if (test_rounding_consistency()) passed++;
    printf("\n");

    if (test_negative_scroll()) passed++;
    printf("\n");

    if (test_zero_content()) passed++;

    printf("\n=== Test Results ===\n");
    printf("Results: %d/%d tests passed\n", passed, total);

    return (passed == total) ? 0 : 1;
}
