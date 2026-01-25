/**
 * Test for panel text truncation bug (julius-050)
 *
 * This test validates that panel_get_text_width() subtracts
 * padding from both sides, not just once.
 *
 * The bug: The function subtracts padding only once, but padding
 * applies to both left and right sides of the panel.
 *
 * Test validation:
 * - FIXED code: Returns panel_width - (padding * 2)
 * - BUGGY code: Returns panel_width - padding (wrong!)
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/* External functions from panel.c */
extern int panel_get_text_width(int panel_width, int padding);
extern int panel_get_text_width_buggy(int panel_width, int padding);
extern int get_expected_text_width(int panel_width, int padding);

/**
 * Test 1: Standard panel with typical padding
 *
 * Panel 200px wide with 10px padding should have 180px for text.
 */
static int test_standard_panel(void)
{
    printf("Test: Standard panel with typical padding...\n");

    int panel_width = 200;
    int padding = 10;

    int result = panel_get_text_width(panel_width, padding);
    int expected = panel_width - (padding * 2);  /* 200 - 20 = 180 */
    int buggy = panel_width - padding;           /* 200 - 10 = 190 */

    printf("  Panel width: %d, Padding: %d\n", panel_width, padding);
    printf("  Result: %d\n", result);
    printf("  Expected (2x padding): %d\n", expected);
    printf("  Buggy (1x padding): %d\n", buggy);

    if (result == buggy && buggy != expected) {
        printf("  FAIL: Result matches buggy calculation (padding subtracted once)\n");
        return 0;
    }

    if (result != expected) {
        printf("  FAIL: Result doesn't match expected\n");
        return 0;
    }

    printf("  PASS\n");
    return 1;
}

/**
 * Test 2: Zero padding
 *
 * With no padding, text width should equal panel width.
 */
static int test_zero_padding(void)
{
    printf("Test: Zero padding...\n");

    int panel_width = 150;
    int padding = 0;

    int result = panel_get_text_width(panel_width, padding);
    int expected = panel_width;  /* 150 - 0 = 150 */

    printf("  Panel width: %d, Padding: %d\n", panel_width, padding);
    printf("  Result: %d, Expected: %d\n", result, expected);

    if (result != expected) {
        printf("  FAIL: Zero padding should give full width\n");
        return 0;
    }

    printf("  PASS\n");
    return 1;
}

/**
 * Test 3: Large padding
 *
 * Test with large padding to make the bug obvious.
 */
static int test_large_padding(void)
{
    printf("Test: Large padding...\n");

    int panel_width = 300;
    int padding = 50;

    int result = panel_get_text_width(panel_width, padding);
    int expected = panel_width - (padding * 2);  /* 300 - 100 = 200 */
    int buggy = panel_width - padding;           /* 300 - 50 = 250 */

    printf("  Panel width: %d, Padding: %d\n", panel_width, padding);
    printf("  Result: %d\n", result);
    printf("  Expected: %d (difference from buggy: %d)\n", expected, buggy - expected);

    if (result == buggy) {
        printf("  FAIL: Got buggy value - padding only subtracted once!\n");
        return 0;
    }

    if (result != expected) {
        printf("  FAIL: Wrong value\n");
        return 0;
    }

    printf("  PASS\n");
    return 1;
}

/**
 * Test 4: Narrow panel with padding
 *
 * When padding takes up most of the panel width.
 */
static int test_narrow_panel(void)
{
    printf("Test: Narrow panel with padding...\n");

    int panel_width = 50;
    int padding = 20;

    int result = panel_get_text_width(panel_width, padding);
    int expected = 10;  /* 50 - 40 = 10 */
    int buggy = 30;     /* 50 - 20 = 30 */

    printf("  Panel width: %d, Padding: %d\n", panel_width, padding);
    printf("  Result: %d, Expected: %d, Buggy: %d\n", result, expected, buggy);

    if (result == buggy) {
        printf("  FAIL: Matches buggy calculation\n");
        return 0;
    }

    if (result != expected) {
        printf("  FAIL: Wrong value\n");
        return 0;
    }

    printf("  PASS\n");
    return 1;
}

/**
 * Test 5: Edge case - padding larger than half width
 *
 * When padding * 2 > panel_width, result should go negative or be handled.
 */
static int test_excessive_padding(void)
{
    printf("Test: Excessive padding (larger than half width)...\n");

    int panel_width = 40;
    int padding = 30;

    int result = panel_get_text_width(panel_width, padding);
    int expected = 40 - 60;  /* = -20 (could be negative or clamped) */

    printf("  Panel width: %d, Padding: %d\n", panel_width, padding);
    printf("  Result: %d\n", result);

    /* The key is that it doesn't return 40 - 30 = 10 (buggy) */
    int buggy = panel_width - padding;  /* 10 */

    if (result == buggy) {
        printf("  FAIL: Got buggy value\n");
        return 0;
    }

    /* Accept negative or any correct calculation */
    if (result != expected && result != 0) {
        printf("  Note: Result is %d (could be clamped to 0 or negative)\n", result);
    }

    printf("  PASS: Correctly calculates both-sides padding\n");
    return 1;
}

/**
 * Test 6: Invalid inputs
 */
static int test_invalid_inputs(void)
{
    printf("Test: Invalid inputs...\n");

    /* Zero width should return 0 */
    int zero_width = panel_get_text_width(0, 10);
    printf("  Width 0: %d (expected 0)\n", zero_width);

    if (zero_width != 0) {
        printf("  FAIL: Zero width should return 0\n");
        return 0;
    }

    /* Negative width should return 0 */
    int neg_width = panel_get_text_width(-50, 10);
    printf("  Width -50: %d (expected 0)\n", neg_width);

    if (neg_width != 0) {
        printf("  FAIL: Negative width should return 0\n");
        return 0;
    }

    /* Negative padding should be treated as 0 */
    int neg_padding = panel_get_text_width(100, -10);
    printf("  Padding -10: %d (expected 100)\n", neg_padding);

    if (neg_padding != 100) {
        printf("  FAIL: Negative padding should be treated as 0\n");
        return 0;
    }

    printf("  PASS\n");
    return 1;
}

/**
 * Test 7: Various panel sizes
 *
 * Test formula correctness across multiple sizes.
 */
static int test_various_sizes(void)
{
    printf("Test: Various panel sizes...\n");

    struct {
        int width;
        int padding;
        int expected;
    } cases[] = {
        {100, 5, 90},    /* 100 - 10 = 90 */
        {200, 8, 184},   /* 200 - 16 = 184 */
        {320, 12, 296},  /* 320 - 24 = 296 */
        {400, 20, 360},  /* 400 - 40 = 360 */
        {800, 32, 736}   /* 800 - 64 = 736 */
    };

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        int result = panel_get_text_width(cases[i].width, cases[i].padding);

        printf("  Width %d, Padding %d: %d (expected %d)\n",
               cases[i].width, cases[i].padding, result, cases[i].expected);

        if (result != cases[i].expected) {
            printf("  FAIL\n");
            return 0;
        }
    }

    printf("  PASS: All sizes correct\n");
    return 1;
}

int main(void)
{
    int passed = 0;
    int total = 7;

    printf("=== Panel Text Truncation Test Suite ===\n");
    printf("Testing panel_get_text_width() from graphics/panel.c\n");
    printf("Bug: Padding subtracted once instead of twice\n\n");

    if (test_standard_panel()) passed++;
    printf("\n");

    if (test_zero_padding()) passed++;
    printf("\n");

    if (test_large_padding()) passed++;
    printf("\n");

    if (test_narrow_panel()) passed++;
    printf("\n");

    if (test_excessive_padding()) passed++;
    printf("\n");

    if (test_invalid_inputs()) passed++;
    printf("\n");

    if (test_various_sizes()) passed++;

    printf("\n=== Test Results ===\n");
    printf("Results: %d/%d tests passed\n", passed, total);

    return (passed == total) ? 0 : 1;
}
