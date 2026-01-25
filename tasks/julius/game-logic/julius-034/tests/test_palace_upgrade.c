/**
 * Test for palace upgrade validation bug (julius-034)
 *
 * This test verifies that can_upgrade_governors_house() correctly
 * uses AND logic instead of OR logic.
 *
 * Test validation:
 * - On FIXED code: Upgrade denied when villa OR palace exists
 * - On BUGGY code: Upgrade allowed when only one exists (wrong!)
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/* External functions from stubs */
extern void test_set_has_villa(int has_villa);
extern void test_set_has_palace(int has_palace);
extern int can_upgrade_governors_house(void);
extern void test_reset_state(void);

/**
 * Test 1: No existing upgrades - should allow upgrade
 */
static int test_no_upgrades_allow(void)
{
    printf("Test: No existing upgrades should allow upgrade...\n");

    test_reset_state();
    test_set_has_villa(0);
    test_set_has_palace(0);

    int result = can_upgrade_governors_house();
    printf("  has_villa=0, has_palace=0: can_upgrade=%d\n", result);

    if (result != 1) {
        printf("  FAIL: Should allow upgrade when no upgrades exist\n");
        return 0;
    }

    printf("  PASS: Correctly allows upgrade\n");
    return 1;
}

/**
 * Test 2: Villa exists - should DENY upgrade
 *
 * This is the KEY test for the bug. With OR logic, this incorrectly returns 1.
 */
static int test_villa_exists_deny(void)
{
    printf("Test: Existing villa should deny upgrade...\n");

    test_reset_state();
    test_set_has_villa(1);
    test_set_has_palace(0);

    int result = can_upgrade_governors_house();
    printf("  has_villa=1, has_palace=0: can_upgrade=%d\n", result);

    if (result != 0) {
        printf("  FAIL: Should DENY upgrade when villa exists!\n");
        printf("  This is the bug - OR logic allows this incorrectly\n");
        printf("  !has_villa || !has_palace = !1 || !0 = 0 || 1 = 1 (WRONG)\n");
        return 0;
    }

    printf("  PASS: Correctly denies upgrade\n");
    return 1;
}

/**
 * Test 3: Palace exists - should DENY upgrade
 *
 * Another key test for the bug.
 */
static int test_palace_exists_deny(void)
{
    printf("Test: Existing palace should deny upgrade...\n");

    test_reset_state();
    test_set_has_villa(0);
    test_set_has_palace(1);

    int result = can_upgrade_governors_house();
    printf("  has_villa=0, has_palace=1: can_upgrade=%d\n", result);

    if (result != 0) {
        printf("  FAIL: Should DENY upgrade when palace exists!\n");
        printf("  This is the bug - OR logic allows this incorrectly\n");
        printf("  !has_villa || !has_palace = !0 || !1 = 1 || 0 = 1 (WRONG)\n");
        return 0;
    }

    printf("  PASS: Correctly denies upgrade\n");
    return 1;
}

/**
 * Test 4: Both exist - should DENY upgrade
 *
 * This case works correctly even with buggy OR logic.
 */
static int test_both_exist_deny(void)
{
    printf("Test: Both villa and palace exist should deny upgrade...\n");

    test_reset_state();
    test_set_has_villa(1);
    test_set_has_palace(1);

    int result = can_upgrade_governors_house();
    printf("  has_villa=1, has_palace=1: can_upgrade=%d\n", result);

    if (result != 0) {
        printf("  FAIL: Should DENY upgrade when both exist\n");
        return 0;
    }

    printf("  PASS: Correctly denies upgrade\n");
    return 1;
}

/**
 * Test 5: Truth table verification
 *
 * Exhaustively test all combinations to verify AND logic.
 */
static int test_truth_table(void)
{
    printf("Test: Complete truth table for upgrade logic...\n");

    int test_cases[][4] = {
        /* has_villa, has_palace, expected_result, test_id */
        {0, 0, 1, 1},  /* Neither exists - allow */
        {0, 1, 0, 2},  /* Palace exists - deny */
        {1, 0, 0, 3},  /* Villa exists - deny */
        {1, 1, 0, 4},  /* Both exist - deny */
    };

    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    int all_passed = 1;

    for (int i = 0; i < num_cases; i++) {
        test_reset_state();
        test_set_has_villa(test_cases[i][0]);
        test_set_has_palace(test_cases[i][1]);

        int result = can_upgrade_governors_house();
        int expected = test_cases[i][2];

        printf("  Case %d: villa=%d, palace=%d -> %d (expected %d) %s\n",
               test_cases[i][3],
               test_cases[i][0],
               test_cases[i][1],
               result,
               expected,
               result == expected ? "OK" : "FAIL");

        if (result != expected) {
            all_passed = 0;
        }
    }

    if (!all_passed) {
        printf("  FAIL: Truth table verification failed\n");
        return 0;
    }

    printf("  PASS: All truth table cases correct\n");
    return 1;
}

int main(void)
{
    int passed = 0;
    int total = 5;

    printf("=== Palace Upgrade Validation Test Suite ===\n");
    printf("Testing can_upgrade_governors_house() boolean logic\n");
    printf("Bug: Uses OR instead of AND in upgrade validation\n\n");

    if (test_no_upgrades_allow()) passed++;
    printf("\n");

    if (test_villa_exists_deny()) passed++;
    printf("\n");

    if (test_palace_exists_deny()) passed++;
    printf("\n");

    if (test_both_exist_deny()) passed++;
    printf("\n");

    if (test_truth_table()) passed++;

    printf("\n=== Test Results ===\n");
    printf("Results: %d/%d tests passed\n", passed, total);

    return (passed == total) ? 0 : 1;
}
