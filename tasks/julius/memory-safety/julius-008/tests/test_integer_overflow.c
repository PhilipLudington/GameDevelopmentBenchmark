/**
 * Test for integer overflow in resource value calculation
 *
 * Test validation:
 * - FIXED code: Overflow is detected/clamped, tests pass
 * - BUGGY code: Overflow causes negative result, tests fail
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <limits.h>

/* External function from stub or Julius src/city/resource.c */
extern int resource_calculate_value(int quantity, int price_per_unit);

void test_normal_calculation(void)
{
    printf("Test: normal resource value calculation...\n");
    int value = resource_calculate_value(100, 50);
    printf("  100 units * 50 price = %d\n", value);
    assert(value == 5000);
    printf("  PASS\n");
}

void test_overflow_detection(void)
{
    printf("Test: overflow with large values...\n");
    /* 50000 * 50000 = 2,500,000,000 - overflows int32 (max ~2.1 billion) */
    int value = resource_calculate_value(50000, 50000);
    printf("  50000 * 50000 = %d\n", value);
    printf("  Expected: 2500000000 (but > INT_MAX, so should be clamped)\n");

    /* FIXED code: should return INT_MAX (clamped) or handle overflow gracefully */
    /* BUGGY code: returns negative number due to overflow */
    if (value < 0) {
        printf("  FAIL: overflow caused negative result (buggy code)\n");
        exit(1);
    }

    /* Should be clamped to INT_MAX */
    assert(value == INT_MAX);
    printf("  PASS: overflow handled correctly (clamped to INT_MAX)\n");
}

void test_zero_values(void)
{
    printf("Test: zero values...\n");
    assert(resource_calculate_value(0, 100) == 0);
    assert(resource_calculate_value(100, 0) == 0);
    printf("  PASS\n");
}

void test_negative_overflow(void)
{
    printf("Test: potential negative overflow...\n");
    /* Large negative * positive could underflow */
    int value = resource_calculate_value(-50000, 50000);
    printf("  -50000 * 50000 = %d\n", value);

    /* Should be clamped to INT_MIN or handled gracefully */
    if (value > 0) {
        printf("  FAIL: underflow caused positive wraparound (buggy code)\n");
        exit(1);
    }

    assert(value == INT_MIN);
    printf("  PASS: underflow handled correctly (clamped to INT_MIN)\n");
}

int main(void)
{
    printf("=== Integer Overflow Test Suite ===\n\n");
    test_normal_calculation();
    test_zero_values();
    test_overflow_detection();
    test_negative_overflow();
    printf("\n=== All tests passed ===\n");
    return 0;
}
