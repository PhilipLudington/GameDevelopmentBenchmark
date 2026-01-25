/**
 * Test for popularity decay rounding error (julius-035)
 *
 * This test verifies that the decay calculation multiplies before dividing
 * to preserve precision.
 *
 * Test validation:
 * - On FIXED code: Correct decay values calculated
 * - On BUGGY code: Truncation to zero for small values
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/* External functions from stubs */
extern void test_set_difficulty_modifier(int modifier);
extern void test_set_time_factor(int factor);
extern int calculate_popularity_decay(int current_popularity, int base_decay_rate);
extern int calculate_raw_decay(int base_decay_rate);
extern void test_reset_state(void);

/**
 * Test 1: Small decay rate that truncates to zero with buggy code
 *
 * base_decay = 50, modifier = 3, time_factor = 2, SCALE = 100
 * Buggy:  50 / 100 * 3 * 2 = 0 * 3 * 2 = 0
 * Fixed:  50 * 3 * 2 / 100 = 300 / 100 = 3
 */
static int test_small_decay_rate(void)
{
    printf("Test: Small decay rate precision...\n");

    test_reset_state();
    test_set_difficulty_modifier(3);
    test_set_time_factor(2);

    int base_decay = 50;
    int decay = calculate_raw_decay(base_decay);

    printf("  base_decay=%d, modifier=3, time_factor=2\n", base_decay);
    printf("  Buggy formula: %d / 100 * 3 * 2 = %d\n", base_decay, (base_decay / 100) * 3 * 2);
    printf("  Fixed formula: %d * 3 * 2 / 100 = %d\n", base_decay, decay);

    /* With correct formula: 50 * 3 * 2 / 100 = 3 */
    int expected = 3;

    if (decay != expected) {
        printf("  FAIL: Expected decay=%d, got %d\n", expected, decay);
        if (decay == 0) {
            printf("  This is the bug - division before multiplication truncates to zero!\n");
        }
        return 0;
    }

    printf("  PASS: Decay correctly calculated as %d\n", decay);
    return 1;
}

/**
 * Test 2: Very small decay rate
 *
 * base_decay = 10, modifier = 2, time_factor = 3, SCALE = 100
 * Buggy:  10 / 100 * 2 * 3 = 0 * 2 * 3 = 0
 * Fixed:  10 * 2 * 3 / 100 = 60 / 100 = 0 (correct truncation)
 *
 * Actually for this case both are 0, so let's use larger multipliers
 * base_decay = 10, modifier = 5, time_factor = 5
 * Buggy:  10 / 100 * 5 * 5 = 0
 * Fixed:  10 * 5 * 5 / 100 = 250 / 100 = 2
 */
static int test_very_small_decay(void)
{
    printf("Test: Very small decay rate with large multipliers...\n");

    test_reset_state();
    test_set_difficulty_modifier(5);
    test_set_time_factor(5);

    int base_decay = 10;
    int decay = calculate_raw_decay(base_decay);

    printf("  base_decay=%d, modifier=5, time_factor=5\n", base_decay);
    printf("  Buggy:  %d / 100 * 5 * 5 = %d\n", base_decay, (base_decay / 100) * 5 * 5);
    printf("  Fixed:  %d * 5 * 5 / 100 = %d\n", base_decay, decay);

    /* With correct formula: 10 * 5 * 5 / 100 = 2 */
    int expected = 2;

    if (decay != expected) {
        printf("  FAIL: Expected decay=%d, got %d\n", expected, decay);
        return 0;
    }

    printf("  PASS: Decay correctly calculated as %d\n", decay);
    return 1;
}

/**
 * Test 3: Larger decay rate (should work with either formula)
 *
 * base_decay = 200, modifier = 2, time_factor = 1
 * Buggy:  200 / 100 * 2 * 1 = 2 * 2 * 1 = 4
 * Fixed:  200 * 2 * 1 / 100 = 400 / 100 = 4
 */
static int test_larger_decay_rate(void)
{
    printf("Test: Larger decay rate (should work either way)...\n");

    test_reset_state();
    test_set_difficulty_modifier(2);
    test_set_time_factor(1);

    int base_decay = 200;
    int decay = calculate_raw_decay(base_decay);

    printf("  base_decay=%d, modifier=2, time_factor=1\n", base_decay);
    printf("  Both formulas: decay=%d\n", decay);

    int expected = 4;

    if (decay != expected) {
        printf("  FAIL: Expected decay=%d, got %d\n", expected, decay);
        return 0;
    }

    printf("  PASS: Decay correctly calculated as %d\n", decay);
    return 1;
}

/**
 * Test 4: Accumulated error over time
 *
 * Simulate multiple decay ticks and compare accumulated error.
 */
static int test_accumulated_error(void)
{
    printf("Test: Accumulated decay over 100 ticks...\n");

    test_reset_state();
    test_set_difficulty_modifier(3);
    test_set_time_factor(2);

    int initial_popularity = 100;
    int popularity = initial_popularity;
    int base_decay = 50;

    /* Expected decay per tick: 50 * 3 * 2 / 100 = 3 */
    int expected_decay_per_tick = 3;

    /* Simulate 10 ticks */
    for (int i = 0; i < 10; i++) {
        popularity = calculate_popularity_decay(popularity, base_decay);
    }

    int expected_final = initial_popularity - (expected_decay_per_tick * 10);
    printf("  After 10 ticks: popularity=%d (expected %d)\n", popularity, expected_final);

    if (popularity != expected_final) {
        printf("  FAIL: Accumulated error detected\n");
        printf("  Expected: %d, Got: %d, Error: %d\n",
               expected_final, popularity, expected_final - popularity);
        if (popularity == initial_popularity) {
            printf("  Popularity unchanged - decay is being truncated to zero!\n");
        }
        return 0;
    }

    printf("  PASS: No accumulated error\n");
    return 1;
}

/**
 * Test 5: Edge case - decay exactly equals SCALE_FACTOR
 *
 * base_decay = 100, modifier = 1, time_factor = 1
 * Both: 100 * 1 * 1 / 100 = 1 or 100 / 100 * 1 * 1 = 1
 */
static int test_exact_scale_factor(void)
{
    printf("Test: Decay equals SCALE_FACTOR...\n");

    test_reset_state();
    test_set_difficulty_modifier(1);
    test_set_time_factor(1);

    int base_decay = 100;
    int decay = calculate_raw_decay(base_decay);

    printf("  base_decay=%d, modifier=1, time_factor=1\n", base_decay);
    printf("  Decay: %d\n", decay);

    if (decay != 1) {
        printf("  FAIL: Expected decay=1, got %d\n", decay);
        return 0;
    }

    printf("  PASS: Decay correctly calculated as 1\n");
    return 1;
}

int main(void)
{
    int passed = 0;
    int total = 5;

    printf("=== Popularity Decay Rounding Test Suite ===\n");
    printf("Testing integer math precision in decay calculation\n");
    printf("Bug: Division before multiplication causes truncation\n\n");

    if (test_small_decay_rate()) passed++;
    printf("\n");

    if (test_very_small_decay()) passed++;
    printf("\n");

    if (test_larger_decay_rate()) passed++;
    printf("\n");

    if (test_accumulated_error()) passed++;
    printf("\n");

    if (test_exact_scale_factor()) passed++;

    printf("\n=== Test Results ===\n");
    printf("Results: %d/%d tests passed\n", passed, total);

    return (passed == total) ? 0 : 1;
}
