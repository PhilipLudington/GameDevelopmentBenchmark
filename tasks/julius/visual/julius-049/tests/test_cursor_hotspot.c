/**
 * Test for cursor hotspot offset bug (julius-049)
 *
 * This test validates that cursor_get_click_position() subtracts
 * the hotspot offset instead of adding it.
 *
 * The bug: The function adds the hotspot offset instead of subtracting,
 * causing clicks to register at the wrong screen position.
 *
 * Test validation:
 * - FIXED code: click = cursor - hotspot
 * - BUGGY code: click = cursor + hotspot
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/* External functions from cursor.c */
extern void cursor_get_click_position(int cursor_x, int cursor_y,
                                       int hotspot_x, int hotspot_y,
                                       int *click_x, int *click_y);

extern void cursor_get_click_position_buggy(int cursor_x, int cursor_y,
                                             int hotspot_x, int hotspot_y,
                                             int *click_x, int *click_y);

/**
 * Test 1: Arrow cursor at origin (hotspot at tip)
 *
 * An arrow cursor typically has hotspot at (0, 0) - the tip.
 * For this case, click position should equal cursor position.
 */
static int test_arrow_cursor_origin(void)
{
    printf("Test: Arrow cursor with hotspot at origin...\n");

    int click_x, click_y;
    int cursor_x = 100, cursor_y = 200;
    int hotspot_x = 0, hotspot_y = 0;

    cursor_get_click_position(cursor_x, cursor_y, hotspot_x, hotspot_y,
                              &click_x, &click_y);

    printf("  Cursor: (%d, %d), Hotspot: (%d, %d)\n",
           cursor_x, cursor_y, hotspot_x, hotspot_y);
    printf("  Click position: (%d, %d)\n", click_x, click_y);
    printf("  Expected: (%d, %d)\n", cursor_x, cursor_y);

    if (click_x != cursor_x || click_y != cursor_y) {
        printf("  FAIL: Zero hotspot should give cursor position\n");
        return 0;
    }

    printf("  PASS\n");
    return 1;
}

/**
 * Test 2: Crosshair cursor (hotspot in center)
 *
 * A 32x32 crosshair cursor has hotspot at (16, 16) - the center.
 * Click should be at cursor position minus the hotspot offset.
 */
static int test_crosshair_cursor(void)
{
    printf("Test: Crosshair cursor with centered hotspot...\n");

    int click_x, click_y;
    int cursor_x = 100, cursor_y = 200;
    int hotspot_x = 16, hotspot_y = 16;  /* Center of 32x32 cursor */

    cursor_get_click_position(cursor_x, cursor_y, hotspot_x, hotspot_y,
                              &click_x, &click_y);

    /* Expected: cursor - hotspot = (100-16, 200-16) = (84, 184) */
    int expected_x = cursor_x - hotspot_x;  /* 84 */
    int expected_y = cursor_y - hotspot_y;  /* 184 */

    /* Buggy would give: cursor + hotspot = (116, 216) */
    int buggy_x = cursor_x + hotspot_x;
    int buggy_y = cursor_y + hotspot_y;

    printf("  Cursor: (%d, %d), Hotspot: (%d, %d)\n",
           cursor_x, cursor_y, hotspot_x, hotspot_y);
    printf("  Click position: (%d, %d)\n", click_x, click_y);
    printf("  Expected (fixed): (%d, %d)\n", expected_x, expected_y);
    printf("  Buggy would be: (%d, %d)\n", buggy_x, buggy_y);

    if (click_x == buggy_x && click_y == buggy_y) {
        printf("  FAIL: Result matches buggy calculation (added instead of subtracted)\n");
        return 0;
    }

    if (click_x != expected_x || click_y != expected_y) {
        printf("  FAIL: Click position doesn't match expected\n");
        return 0;
    }

    printf("  PASS\n");
    return 1;
}

/**
 * Test 3: Hand/pointer cursor (hotspot at fingertip)
 *
 * A hand cursor might have hotspot at (5, 0) - the pointing finger tip.
 */
static int test_hand_cursor(void)
{
    printf("Test: Hand cursor with offset hotspot...\n");

    int click_x, click_y;
    int cursor_x = 150, cursor_y = 300;
    int hotspot_x = 5, hotspot_y = 0;

    cursor_get_click_position(cursor_x, cursor_y, hotspot_x, hotspot_y,
                              &click_x, &click_y);

    int expected_x = cursor_x - hotspot_x;  /* 145 */
    int expected_y = cursor_y - hotspot_y;  /* 300 */

    printf("  Cursor: (%d, %d), Hotspot: (%d, %d)\n",
           cursor_x, cursor_y, hotspot_x, hotspot_y);
    printf("  Click position: (%d, %d)\n", click_x, click_y);
    printf("  Expected: (%d, %d)\n", expected_x, expected_y);

    if (click_x != expected_x || click_y != expected_y) {
        printf("  FAIL: Hand cursor click position wrong\n");
        return 0;
    }

    printf("  PASS\n");
    return 1;
}

/**
 * Test 4: Large hotspot offset
 *
 * Test with a large hotspot to make the bug obvious.
 */
static int test_large_hotspot(void)
{
    printf("Test: Large hotspot offset...\n");

    int click_x, click_y;
    int cursor_x = 200, cursor_y = 200;
    int hotspot_x = 50, hotspot_y = 40;

    cursor_get_click_position(cursor_x, cursor_y, hotspot_x, hotspot_y,
                              &click_x, &click_y);

    /* Expected: (200-50, 200-40) = (150, 160) */
    /* Buggy: (200+50, 200+40) = (250, 240) */
    int expected_x = 150, expected_y = 160;
    int buggy_x = 250, buggy_y = 240;

    printf("  Cursor: (%d, %d), Hotspot: (%d, %d)\n",
           cursor_x, cursor_y, hotspot_x, hotspot_y);
    printf("  Click position: (%d, %d)\n", click_x, click_y);
    printf("  Expected: (%d, %d), Buggy would be: (%d, %d)\n",
           expected_x, expected_y, buggy_x, buggy_y);

    if (click_x == buggy_x || click_y == buggy_y) {
        printf("  FAIL: Matches buggy calculation\n");
        return 0;
    }

    if (click_x != expected_x || click_y != expected_y) {
        printf("  FAIL: Doesn't match expected\n");
        return 0;
    }

    printf("  PASS\n");
    return 1;
}

/**
 * Test 5: NULL pointer handling
 */
static int test_null_pointers(void)
{
    printf("Test: NULL pointer handling...\n");

    int click_x = -1, click_y = -1;

    /* These should not crash */
    cursor_get_click_position(100, 100, 10, 10, NULL, &click_y);
    cursor_get_click_position(100, 100, 10, 10, &click_x, NULL);
    cursor_get_click_position(100, 100, 10, 10, NULL, NULL);

    printf("  No crash with NULL pointers\n");
    printf("  PASS\n");
    return 1;
}

/**
 * Test 6: Cursor at screen edge
 *
 * Test with cursor near origin to verify negative click positions work.
 */
static int test_edge_position(void)
{
    printf("Test: Cursor at screen edge...\n");

    int click_x, click_y;
    int cursor_x = 10, cursor_y = 5;
    int hotspot_x = 16, hotspot_y = 16;

    cursor_get_click_position(cursor_x, cursor_y, hotspot_x, hotspot_y,
                              &click_x, &click_y);

    /* Expected: (10-16, 5-16) = (-6, -11) - negative is OK */
    int expected_x = -6, expected_y = -11;

    printf("  Cursor: (%d, %d), Hotspot: (%d, %d)\n",
           cursor_x, cursor_y, hotspot_x, hotspot_y);
    printf("  Click position: (%d, %d)\n", click_x, click_y);
    printf("  Expected: (%d, %d)\n", expected_x, expected_y);

    if (click_x != expected_x || click_y != expected_y) {
        printf("  FAIL: Edge position wrong\n");
        return 0;
    }

    printf("  PASS\n");
    return 1;
}

/**
 * Test 7: Multiple cursor types simulation
 */
static int test_multiple_cursors(void)
{
    printf("Test: Multiple cursor types...\n");

    struct {
        const char *name;
        int cursor_x, cursor_y;
        int hotspot_x, hotspot_y;
        int expected_x, expected_y;
    } cursors[] = {
        {"Arrow",     100, 100,  0,  0, 100, 100},
        {"Crosshair", 100, 100, 16, 16,  84,  84},
        {"Hand",      100, 100,  5,  0,  95, 100},
        {"Resize",    100, 100,  8,  8,  92,  92},
        {"Move",      100, 100, 12, 12,  88,  88}
    };

    for (size_t i = 0; i < sizeof(cursors) / sizeof(cursors[0]); i++) {
        int click_x, click_y;
        cursor_get_click_position(cursors[i].cursor_x, cursors[i].cursor_y,
                                  cursors[i].hotspot_x, cursors[i].hotspot_y,
                                  &click_x, &click_y);

        printf("  %s: click (%d, %d), expected (%d, %d)\n",
               cursors[i].name, click_x, click_y,
               cursors[i].expected_x, cursors[i].expected_y);

        if (click_x != cursors[i].expected_x ||
            click_y != cursors[i].expected_y) {
            printf("  FAIL: %s cursor position wrong\n", cursors[i].name);
            return 0;
        }
    }

    printf("  PASS: All cursor types correct\n");
    return 1;
}

int main(void)
{
    int passed = 0;
    int total = 7;

    printf("=== Cursor Hotspot Offset Test Suite ===\n");
    printf("Testing cursor_get_click_position() from input/cursor.c\n");
    printf("Bug: Hotspot offset added instead of subtracted\n\n");

    if (test_arrow_cursor_origin()) passed++;
    printf("\n");

    if (test_crosshair_cursor()) passed++;
    printf("\n");

    if (test_hand_cursor()) passed++;
    printf("\n");

    if (test_large_hotspot()) passed++;
    printf("\n");

    if (test_null_pointers()) passed++;
    printf("\n");

    if (test_edge_position()) passed++;
    printf("\n");

    if (test_multiple_cursors()) passed++;

    printf("\n=== Test Results ===\n");
    printf("Results: %d/%d tests passed\n", passed, total);

    return (passed == total) ? 0 : 1;
}
