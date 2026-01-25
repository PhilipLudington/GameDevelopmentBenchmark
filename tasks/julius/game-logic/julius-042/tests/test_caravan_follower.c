/**
 * Test for caravan follower index mismatch (julius-042)
 *
 * This test verifies that update_all_followers() processes ALL
 * followers including the one at index 0.
 *
 * Test validation:
 * - On FIXED code: All followers updated (was_updated flag set)
 * - On BUGGY code: First follower (index 0) not updated
 */

#include <stdio.h>
#include <stdlib.h>

#define MAX_CARAVAN_FOLLOWERS 4

/* Caravan follower structure */
typedef struct {
    int figure_id;
    int x, y;
    int carrying_resource;
    int was_updated;
} caravan_follower;

/* External functions from stubs */
extern void add_follower(int figure_id);
extern void update_all_followers(void);
extern int get_num_followers(void);
extern int was_follower_updated(int index);
extern caravan_follower *get_follower(int index);
extern void clear_followers(void);
extern int count_updated_followers(void);
extern void reset_update_flags(void);

/**
 * Test 1: First follower (index 0) should be updated
 *
 * This is the KEY test for the bug.
 */
static int test_first_follower_updated(void)
{
    printf("Test: First follower (index 0) is updated...\n");

    clear_followers();
    add_follower(100);  /* Add follower at index 0 */

    printf("  Added 1 follower (index 0)\n");

    update_all_followers();

    int updated = was_follower_updated(0);
    printf("  Follower 0 was_updated: %d\n", updated);

    if (updated != 1) {
        printf("  FAIL: First follower was NOT updated!\n");
        printf("  This is the bug - loop starts at index 1 instead of 0\n");
        return 0;
    }

    printf("  PASS: First follower correctly updated\n");
    return 1;
}

/**
 * Test 2: All followers should be updated
 */
static int test_all_followers_updated(void)
{
    printf("Test: All followers are updated...\n");

    clear_followers();
    add_follower(100);
    add_follower(101);
    add_follower(102);
    add_follower(103);

    int total = get_num_followers();
    printf("  Added %d followers\n", total);

    update_all_followers();

    int updated = count_updated_followers();
    printf("  Followers updated: %d / %d\n", updated, total);

    if (updated != total) {
        printf("  FAIL: Not all followers were updated\n");
        printf("  Missing: follower at index 0 (off-by-one bug)\n");
        return 0;
    }

    printf("  PASS: All followers updated\n");
    return 1;
}

/**
 * Test 3: Specific check - index 0 vs index 1
 */
static int test_index_zero_vs_one(void)
{
    printf("Test: Compare index 0 and index 1 update status...\n");

    clear_followers();
    add_follower(100);  /* index 0 */
    add_follower(101);  /* index 1 */

    update_all_followers();

    int idx0_updated = was_follower_updated(0);
    int idx1_updated = was_follower_updated(1);

    printf("  Index 0 updated: %d\n", idx0_updated);
    printf("  Index 1 updated: %d\n", idx1_updated);

    if (idx0_updated != 1) {
        printf("  FAIL: Index 0 not updated (off-by-one error)\n");
        return 0;
    }

    if (idx1_updated != 1) {
        printf("  FAIL: Index 1 not updated\n");
        return 0;
    }

    printf("  PASS: Both indices correctly updated\n");
    return 1;
}

/**
 * Test 4: Single follower at index 0
 */
static int test_single_follower(void)
{
    printf("Test: Single follower at index 0...\n");

    clear_followers();
    add_follower(999);

    int count_before = count_updated_followers();
    update_all_followers();
    int count_after = count_updated_followers();

    printf("  Before update: %d updated\n", count_before);
    printf("  After update: %d updated\n", count_after);

    if (count_after != 1) {
        printf("  FAIL: Single follower should be updated\n");
        if (count_after == 0) {
            printf("  The loop skips index 0 when i starts at 1!\n");
        }
        return 0;
    }

    printf("  PASS: Single follower updated\n");
    return 1;
}

/**
 * Test 5: Multiple update cycles
 */
static int test_multiple_updates(void)
{
    printf("Test: Multiple update cycles...\n");

    clear_followers();
    add_follower(1);
    add_follower(2);

    /* First update */
    update_all_followers();
    int first_update = count_updated_followers();

    /* Reset flags */
    reset_update_flags();

    /* Second update */
    update_all_followers();
    int second_update = count_updated_followers();

    printf("  First cycle: %d updated\n", first_update);
    printf("  Second cycle: %d updated\n", second_update);

    if (first_update != 2 || second_update != 2) {
        printf("  FAIL: Expected 2 updates each cycle\n");
        return 0;
    }

    printf("  PASS: Multiple cycles work correctly\n");
    return 1;
}

/**
 * Test 6: Verify follower positions actually changed
 */
static int test_position_changed(void)
{
    printf("Test: Follower positions actually change...\n");

    clear_followers();
    add_follower(1);
    add_follower(2);

    caravan_follower *f0 = get_follower(0);
    caravan_follower *f1 = get_follower(1);

    int f0_x_before = f0->x;
    int f1_x_before = f1->x;

    update_all_followers();

    printf("  Follower 0: x changed from %d to %d\n", f0_x_before, f0->x);
    printf("  Follower 1: x changed from %d to %d\n", f1_x_before, f1->x);

    if (f0->x == f0_x_before) {
        printf("  FAIL: Follower 0 position didn't change (not updated!)\n");
        return 0;
    }

    if (f1->x == f1_x_before) {
        printf("  FAIL: Follower 1 position didn't change\n");
        return 0;
    }

    printf("  PASS: Both followers' positions changed\n");
    return 1;
}

int main(void)
{
    int passed = 0;
    int total = 6;

    printf("=== Caravan Follower Index Test Suite ===\n");
    printf("Testing that all followers are updated including index 0\n");
    printf("Bug: Loop starts at index 1 instead of 0\n\n");

    if (test_first_follower_updated()) passed++;
    printf("\n");

    if (test_all_followers_updated()) passed++;
    printf("\n");

    if (test_index_zero_vs_one()) passed++;
    printf("\n");

    if (test_single_follower()) passed++;
    printf("\n");

    if (test_multiple_updates()) passed++;
    printf("\n");

    if (test_position_changed()) passed++;

    printf("\n=== Test Results ===\n");
    printf("Results: %d/%d tests passed\n", passed, total);

    return (passed == total) ? 0 : 1;
}
