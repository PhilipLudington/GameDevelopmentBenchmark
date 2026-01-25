/**
 * Test for plague spread rate integer overflow (julius-036)
 *
 * This test verifies that the spread rate calculation handles
 * potential integer overflow correctly.
 *
 * Test validation:
 * - On FIXED code: Large values correctly capped to MAX_SPREAD_RATE
 * - On BUGGY code: Overflow causes incorrect (possibly negative) values
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

/* External functions from stubs */
extern int calculate_plague_spread_rate(int population_density, int base_spread_factor);
extern int get_max_spread_rate(void);
extern int get_min_spread_rate(void);

/**
 * Test 1: Normal values that don't overflow
 */
static int test_normal_values(void)
{
    printf("Test: Normal values (no overflow)...\n");

    int density = 100;
    int factor = 50;
    int result = calculate_plague_spread_rate(density, factor);

    printf("  density=%d, factor=%d -> spread_rate=%d\n", density, factor, result);

    int expected = 5000;  /* 100 * 50 */
    if (result != expected) {
        printf("  FAIL: Expected %d, got %d\n", expected, result);
        return 0;
    }

    printf("  PASS: Correct spread rate\n");
    return 1;
}

/**
 * Test 2: Large values that would overflow
 *
 * 50000 * 100000 = 5,000,000,000 which overflows 32-bit int
 * Should be capped to MAX_SPREAD_RATE (10,000,000)
 */
static int test_overflow_values(void)
{
    printf("Test: Large values that would overflow...\n");

    int density = 50000;
    int factor = 100000;
    int result = calculate_plague_spread_rate(density, factor);

    printf("  density=%d, factor=%d -> spread_rate=%d\n", density, factor, result);
    printf("  (Would be %lld without overflow)\n", (long long)density * factor);

    int max_rate = get_max_spread_rate();

    /* Result should be capped to MAX_SPREAD_RATE, not some random overflow value */
    if (result != max_rate) {
        printf("  FAIL: Expected MAX_SPREAD_RATE=%d, got %d\n", max_rate, result);
        if (result < 0) {
            printf("  Result is NEGATIVE - integer overflow occurred!\n");
        } else if (result < max_rate) {
            printf("  Result wrapped around due to overflow!\n");
        }
        return 0;
    }

    printf("  PASS: Correctly capped to MAX_SPREAD_RATE\n");
    return 1;
}

/**
 * Test 3: Values just below overflow threshold
 */
static int test_near_overflow(void)
{
    printf("Test: Values near overflow threshold...\n");

    /* These should multiply without overflow */
    int density = 10000;
    int factor = 500;
    int result = calculate_plague_spread_rate(density, factor);

    printf("  density=%d, factor=%d -> spread_rate=%d\n", density, factor, result);

    /* 10000 * 500 = 5,000,000 which is valid */
    int expected = 5000000;
    if (result != expected) {
        printf("  FAIL: Expected %d, got %d\n", expected, result);
        return 0;
    }

    printf("  PASS: Correct spread rate near threshold\n");
    return 1;
}

/**
 * Test 4: Result would exceed MAX but no overflow
 */
static int test_exceeds_max_no_overflow(void)
{
    printf("Test: Result exceeds MAX but doesn't overflow int...\n");

    /* 20000 * 1000 = 20,000,000 > MAX_SPREAD_RATE but fits in int */
    int density = 20000;
    int factor = 1000;
    int result = calculate_plague_spread_rate(density, factor);

    printf("  density=%d, factor=%d -> spread_rate=%d\n", density, factor, result);

    int max_rate = get_max_spread_rate();
    if (result != max_rate) {
        printf("  FAIL: Expected MAX_SPREAD_RATE=%d, got %d\n", max_rate, result);
        return 0;
    }

    printf("  PASS: Correctly capped to MAX_SPREAD_RATE\n");
    return 1;
}

/**
 * Test 5: Zero values
 */
static int test_zero_values(void)
{
    printf("Test: Zero values...\n");

    int result1 = calculate_plague_spread_rate(0, 100);
    int result2 = calculate_plague_spread_rate(100, 0);

    printf("  density=0, factor=100 -> %d\n", result1);
    printf("  density=100, factor=0 -> %d\n", result2);

    int min_rate = get_min_spread_rate();

    /* Zero multiplication should give minimum spread rate */
    if (result1 < min_rate || result2 < min_rate) {
        printf("  FAIL: Result below MIN_SPREAD_RATE\n");
        return 0;
    }

    printf("  PASS: Zero values handled correctly\n");
    return 1;
}

/**
 * Test 6: Maximum possible INT values
 *
 * This is an extreme test - INT_MAX * 2 would definitely overflow
 */
static int test_extreme_values(void)
{
    printf("Test: Extreme values (INT_MAX scale)...\n");

    int density = 100000;
    int factor = 50000;  /* 100000 * 50000 = 5,000,000,000 - overflows */

    int result = calculate_plague_spread_rate(density, factor);

    printf("  density=%d, factor=%d -> spread_rate=%d\n", density, factor, result);

    int max_rate = get_max_spread_rate();

    if (result != max_rate) {
        printf("  FAIL: Expected MAX_SPREAD_RATE=%d, got %d\n", max_rate, result);
        if (result < 0) {
            printf("  CRITICAL: Result is negative due to overflow!\n");
        }
        return 0;
    }

    printf("  PASS: Extreme values handled correctly\n");
    return 1;
}

int main(void)
{
    int passed = 0;
    int total = 6;

    printf("=== Plague Spread Rate Overflow Test Suite ===\n");
    printf("Testing integer overflow prevention in spread calculation\n");
    printf("Bug: Multiplication overflow before bounds check\n\n");

    if (test_normal_values()) passed++;
    printf("\n");

    if (test_overflow_values()) passed++;
    printf("\n");

    if (test_near_overflow()) passed++;
    printf("\n");

    if (test_exceeds_max_no_overflow()) passed++;
    printf("\n");

    if (test_zero_values()) passed++;
    printf("\n");

    if (test_extreme_values()) passed++;

    printf("\n=== Test Results ===\n");
    printf("Results: %d/%d tests passed\n", passed, total);

    return (passed == total) ? 0 : 1;
}
