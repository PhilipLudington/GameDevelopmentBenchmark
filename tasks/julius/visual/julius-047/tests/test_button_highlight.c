/**
 * Test for button highlight color bug (julius-047)
 *
 * This test validates that button_get_highlight_color() returns
 * the correct color index (COLOR_BUTTON_HIGHLIGHT, not COLOR_BUTTON_BORDER).
 *
 * The bug: The function uses COLOR_BUTTON_BORDER (index 3) instead
 * of COLOR_BUTTON_HIGHLIGHT (index 4), causing wrong hover colors.
 *
 * Test validation:
 * - FIXED code: Returns theme_colors[id][4] (highlight color)
 * - BUGGY code: Returns theme_colors[id][3] (border color)
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

typedef uint32_t color_t;

/* External functions from button.c */
extern color_t button_get_highlight_color(int theme_id);
extern color_t button_get_border_color(int theme_id);
extern color_t get_expected_highlight_color(int theme_id);
extern color_t get_border_color_for_comparison(int theme_id);

/**
 * Test 1: Default theme highlight color
 *
 * Verify that the highlight color is different from the border color.
 */
static int test_default_theme_highlight(void)
{
    printf("Test: Default theme highlight color...\n");

    color_t highlight = button_get_highlight_color(0);
    color_t expected = get_expected_highlight_color(0);
    color_t border = get_border_color_for_comparison(0);

    printf("  Highlight returned: 0x%08X\n", highlight);
    printf("  Expected highlight: 0x%08X\n", expected);
    printf("  Border color:       0x%08X\n", border);

    if (highlight == border) {
        printf("  FAIL: Highlight color equals border color!\n");
        printf("  The function is using COLOR_BUTTON_BORDER instead of COLOR_BUTTON_HIGHLIGHT\n");
        return 0;
    }

    if (highlight != expected) {
        printf("  FAIL: Highlight color doesn't match expected\n");
        return 0;
    }

    printf("  PASS: Highlight color is correct (not border)\n");
    return 1;
}

/**
 * Test 2: Dark theme highlight color
 */
static int test_dark_theme_highlight(void)
{
    printf("Test: Dark theme highlight color...\n");

    color_t highlight = button_get_highlight_color(1);
    color_t expected = get_expected_highlight_color(1);
    color_t border = get_border_color_for_comparison(1);

    printf("  Highlight returned: 0x%08X\n", highlight);
    printf("  Expected highlight: 0x%08X\n", expected);
    printf("  Border color:       0x%08X\n", border);

    if (highlight == border) {
        printf("  FAIL: Highlight equals border (bug detected)\n");
        return 0;
    }

    if (highlight != expected) {
        printf("  FAIL: Highlight doesn't match expected\n");
        return 0;
    }

    printf("  PASS\n");
    return 1;
}

/**
 * Test 3: Classic theme highlight color
 */
static int test_classic_theme_highlight(void)
{
    printf("Test: Classic theme highlight color...\n");

    color_t highlight = button_get_highlight_color(2);
    color_t expected = get_expected_highlight_color(2);
    color_t border = get_border_color_for_comparison(2);

    printf("  Highlight returned: 0x%08X\n", highlight);
    printf("  Expected highlight: 0x%08X\n", expected);
    printf("  Border color:       0x%08X\n", border);

    if (highlight == border) {
        printf("  FAIL: Highlight equals border (bug detected)\n");
        return 0;
    }

    if (highlight != expected) {
        printf("  FAIL: Highlight doesn't match expected\n");
        return 0;
    }

    printf("  PASS\n");
    return 1;
}

/**
 * Test 4: Invalid theme ID handling
 *
 * Ensure out-of-range theme IDs default to theme 0.
 */
static int test_invalid_theme_id(void)
{
    printf("Test: Invalid theme ID handling...\n");

    color_t expected_default = get_expected_highlight_color(0);

    /* Negative theme ID should default to 0 */
    color_t negative = button_get_highlight_color(-1);
    printf("  Theme -1: 0x%08X (should be 0x%08X)\n", negative, expected_default);

    if (negative != expected_default) {
        printf("  FAIL: Negative theme ID didn't default to theme 0\n");
        return 0;
    }

    /* Large theme ID should default to 0 */
    color_t large = button_get_highlight_color(999);
    printf("  Theme 999: 0x%08X (should be 0x%08X)\n", large, expected_default);

    if (large != expected_default) {
        printf("  FAIL: Large theme ID didn't default to theme 0\n");
        return 0;
    }

    printf("  PASS\n");
    return 1;
}

/**
 * Test 5: All themes return different highlight vs border
 *
 * Comprehensive check that no theme has highlight == border.
 */
static int test_all_themes_distinct(void)
{
    printf("Test: All themes have distinct highlight vs border...\n");

    for (int theme = 0; theme < 3; theme++) {
        color_t highlight = button_get_highlight_color(theme);
        color_t border = button_get_border_color(theme);

        printf("  Theme %d: highlight=0x%08X, border=0x%08X\n",
               theme, highlight, border);

        if (highlight == border) {
            printf("  FAIL: Theme %d has highlight == border (bug!)\n", theme);
            return 0;
        }
    }

    printf("  PASS: All themes have distinct colors\n");
    return 1;
}

/**
 * Test 6: Verify border color function works correctly
 *
 * Make sure we're testing against the actual border color.
 */
static int test_border_color_function(void)
{
    printf("Test: Border color function correctness...\n");

    /* Border and highlight should be different for each theme */
    for (int theme = 0; theme < 3; theme++) {
        color_t border1 = button_get_border_color(theme);
        color_t border2 = get_border_color_for_comparison(theme);

        if (border1 != border2) {
            printf("  FAIL: Border color functions inconsistent for theme %d\n", theme);
            return 0;
        }
    }

    printf("  PASS: Border color function is consistent\n");
    return 1;
}

int main(void)
{
    int passed = 0;
    int total = 6;

    printf("=== Button Highlight Color Test Suite ===\n");
    printf("Testing button_get_highlight_color() from graphics/button.c\n");
    printf("Bug: Uses COLOR_BUTTON_BORDER instead of COLOR_BUTTON_HIGHLIGHT\n\n");

    if (test_default_theme_highlight()) passed++;
    printf("\n");

    if (test_dark_theme_highlight()) passed++;
    printf("\n");

    if (test_classic_theme_highlight()) passed++;
    printf("\n");

    if (test_invalid_theme_id()) passed++;
    printf("\n");

    if (test_all_themes_distinct()) passed++;
    printf("\n");

    if (test_border_color_function()) passed++;

    printf("\n=== Test Results ===\n");
    printf("Results: %d/%d tests passed\n", passed, total);

    return (passed == total) ? 0 : 1;
}
