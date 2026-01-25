/*
 * Test for julius-024: Build menu hitbox calculation error
 *
 * This test verifies that clicking on build menu buttons returns
 * the correct button index.
 */

#include <stdio.h>
#include <stdlib.h>

#define BUTTON_WIDTH 64
#define BUTTON_HEIGHT 52
#define BUTTONS_PER_ROW 4

/*
 * Get clicked button index from mouse coordinates
 */
int get_clicked_button(int mouse_x, int mouse_y, int menu_x, int menu_y)
{
    int relative_x = mouse_x - menu_x;
    int relative_y = mouse_y - menu_y;

    int col = relative_x / BUTTON_WIDTH;

#ifdef BUGGY_VERSION
    /* BUG: +1 causes off-by-one row */
    int row = relative_y / BUTTON_HEIGHT + 1;
#else
    /* FIXED: No offset */
    int row = relative_y / BUTTON_HEIGHT;
#endif

    return row * BUTTONS_PER_ROW + col;
}

/* Test clicking first button (top-left) */
int test_first_button(void)
{
    int menu_x = 100;
    int menu_y = 100;

    /* Click in center of first button (row 0, col 0) */
    int mouse_x = menu_x + 32;  /* Center of button */
    int mouse_y = menu_y + 26;

    int button = get_clicked_button(mouse_x, mouse_y, menu_x, menu_y);

    if (button != 0) {
        printf("FAIL: First button click returned %d, expected 0\n", button);
        return 1;
    }

    printf("PASS: First button test\n");
    return 0;
}

/* Test clicking button at row 0, col 1 */
int test_second_button(void)
{
    int menu_x = 100;
    int menu_y = 100;

    /* Click second button (row 0, col 1) */
    int mouse_x = menu_x + BUTTON_WIDTH + 32;
    int mouse_y = menu_y + 26;

    int button = get_clicked_button(mouse_x, mouse_y, menu_x, menu_y);

    if (button != 1) {
        printf("FAIL: Second button click returned %d, expected 1\n", button);
        return 1;
    }

    printf("PASS: Second button test\n");
    return 0;
}

/* Test clicking button at row 1, col 0 (button 4) */
int test_row_one_button(void)
{
    int menu_x = 100;
    int menu_y = 100;

    /* Click first button on row 1 */
    int mouse_x = menu_x + 32;
    int mouse_y = menu_y + BUTTON_HEIGHT + 26;

    int button = get_clicked_button(mouse_x, mouse_y, menu_x, menu_y);

    if (button != 4) {
        printf("FAIL: Row 1 button click returned %d, expected 4\n", button);
        return 1;
    }

    printf("PASS: Row 1 button test\n");
    return 0;
}

/* Test clicking button at row 2, col 2 (button 10) */
int test_middle_button(void)
{
    int menu_x = 100;
    int menu_y = 100;

    /* Row 2, Col 2 -> index = 2 * 4 + 2 = 10 */
    int mouse_x = menu_x + (2 * BUTTON_WIDTH) + 32;
    int mouse_y = menu_y + (2 * BUTTON_HEIGHT) + 26;

    int button = get_clicked_button(mouse_x, mouse_y, menu_x, menu_y);

    if (button != 10) {
        printf("FAIL: Middle button click returned %d, expected 10\n", button);
        return 1;
    }

    printf("PASS: Middle button test\n");
    return 0;
}

/* Test clicking at exact button boundaries */
int test_boundary_clicks(void)
{
    int menu_x = 100;
    int menu_y = 100;
    int failures = 0;

    /* Click at very start of button 0 */
    int button = get_clicked_button(menu_x + 1, menu_y + 1, menu_x, menu_y);
    if (button != 0) {
        printf("FAIL: Boundary click (top-left corner) returned %d, expected 0\n", button);
        failures++;
    }

    /* Click at very end of first row */
    button = get_clicked_button(menu_x + BUTTON_WIDTH * 3 + BUTTON_WIDTH - 1,
                                 menu_y + BUTTON_HEIGHT - 1,
                                 menu_x, menu_y);
    if (button != 3) {
        printf("FAIL: Boundary click (row 0, col 3) returned %d, expected 3\n", button);
        failures++;
    }

    if (failures == 0) {
        printf("PASS: Boundary clicks test\n");
    }
    return failures > 0 ? 1 : 0;
}

/* Test all buttons in first two rows */
int test_all_buttons(void)
{
    int menu_x = 100;
    int menu_y = 100;
    int failures = 0;

    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < BUTTONS_PER_ROW; col++) {
            int expected = row * BUTTONS_PER_ROW + col;
            int mouse_x = menu_x + col * BUTTON_WIDTH + BUTTON_WIDTH / 2;
            int mouse_y = menu_y + row * BUTTON_HEIGHT + BUTTON_HEIGHT / 2;

            int button = get_clicked_button(mouse_x, mouse_y, menu_x, menu_y);

            if (button != expected) {
                printf("FAIL: Button[%d][%d] click returned %d, expected %d\n",
                       row, col, button, expected);
                failures++;
            }
        }
    }

    if (failures == 0) {
        printf("PASS: All buttons test (12 buttons checked)\n");
    }
    return failures > 0 ? 1 : 0;
}

int main(void)
{
    int failures = 0;

    printf("=== Build Menu Hitbox Test ===\n\n");

#ifdef BUGGY_VERSION
    printf("Testing BUGGY version (expect row offset errors)\n\n");
#else
    printf("Testing FIXED version (should pass all tests)\n\n");
#endif

    failures += test_first_button();
    failures += test_second_button();
    failures += test_row_one_button();
    failures += test_middle_button();
    failures += test_boundary_clicks();
    failures += test_all_buttons();

    printf("\n=== Results: %d failures ===\n", failures);

    return failures;
}
