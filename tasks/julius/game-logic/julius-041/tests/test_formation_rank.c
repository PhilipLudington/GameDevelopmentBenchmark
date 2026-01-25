/**
 * Test for formation rank array overflow (julius-041)
 *
 * This test verifies that assign_formation_rank() properly checks
 * array bounds and doesn't write past MAX_FORMATION_RANKS.
 *
 * Test validation:
 * - On FIXED code: Assignment fails gracefully when array is full
 * - On BUGGY code: ASAN detects buffer overflow
 */

#include <stdio.h>
#include <stdlib.h>

/* Formation rank entry */
typedef struct {
    int formation_id;
    int rank;
    int position;
} formation_rank_entry;

/* External functions from stubs */
extern int assign_formation_rank(int formation_id, int rank);
extern int get_num_ranks(void);
extern int get_max_ranks(void);
extern formation_rank_entry *get_rank_entry(int index);
extern void clear_formation_ranks(void);

/**
 * Test 1: Normal assignment within bounds
 */
static int test_normal_assignment(void)
{
    printf("Test: Normal assignment within bounds...\n");

    clear_formation_ranks();

    int result = assign_formation_rank(100, 1);

    if (result != 1) {
        printf("  FAIL: Assignment should succeed\n");
        return 0;
    }

    if (get_num_ranks() != 1) {
        printf("  FAIL: Count should be 1\n");
        return 0;
    }

    formation_rank_entry *entry = get_rank_entry(0);
    if (!entry || entry->formation_id != 100 || entry->rank != 1) {
        printf("  FAIL: Entry data incorrect\n");
        return 0;
    }

    printf("  PASS: Normal assignment works\n");
    return 1;
}

/**
 * Test 2: Fill array to capacity
 */
static int test_fill_to_capacity(void)
{
    printf("Test: Fill array to capacity...\n");

    clear_formation_ranks();
    int max = get_max_ranks();

    printf("  Assigning %d formations...\n", max);

    for (int i = 0; i < max; i++) {
        int result = assign_formation_rank(i + 1, i);
        if (result != 1) {
            printf("  FAIL: Assignment %d should succeed\n", i);
            return 0;
        }
    }

    if (get_num_ranks() != max) {
        printf("  FAIL: Count should be %d, got %d\n", max, get_num_ranks());
        return 0;
    }

    printf("  PASS: Array filled to capacity\n");
    return 1;
}

/**
 * Test 3: Assignment when array is full - THE KEY TEST
 *
 * With buggy code, this would write past the array bounds.
 * With ASAN enabled, this would crash on the buggy code.
 */
static int test_overflow_prevention(void)
{
    printf("Test: Assignment when array is full (overflow prevention)...\n");

    clear_formation_ranks();
    int max = get_max_ranks();

    /* Fill to capacity */
    for (int i = 0; i < max; i++) {
        assign_formation_rank(i + 1, i);
    }

    printf("  Array full (%d entries), attempting one more...\n", max);

    /* Try to add one more - this would overflow without bounds check */
    int result = assign_formation_rank(999, 999);

    if (result != 0) {
        printf("  FAIL: Assignment should have failed (returned %d)\n", result);
        printf("  This means bounds check is missing!\n");
        return 0;
    }

    /* Count should still be max */
    if (get_num_ranks() != max) {
        printf("  FAIL: Count changed despite failed assignment\n");
        return 0;
    }

    printf("  PASS: Overflow prevented, assignment correctly rejected\n");
    return 1;
}

/**
 * Test 4: Multiple overflow attempts
 */
static int test_multiple_overflow_attempts(void)
{
    printf("Test: Multiple overflow attempts...\n");

    clear_formation_ranks();
    int max = get_max_ranks();

    /* Fill to capacity */
    for (int i = 0; i < max; i++) {
        assign_formation_rank(i + 1, i);
    }

    /* Try many more assignments - all should fail safely */
    int failures = 0;
    for (int i = 0; i < 100; i++) {
        int result = assign_formation_rank(1000 + i, i);
        if (result == 0) {
            failures++;
        }
    }

    printf("  100 overflow attempts, %d correctly rejected\n", failures);

    if (failures != 100) {
        printf("  FAIL: Some overflow assignments succeeded!\n");
        return 0;
    }

    if (get_num_ranks() != max) {
        printf("  FAIL: Count was modified despite rejections\n");
        return 0;
    }

    printf("  PASS: All overflow attempts safely rejected\n");
    return 1;
}

/**
 * Test 5: Clear and reuse
 */
static int test_clear_and_reuse(void)
{
    printf("Test: Clear and reuse array...\n");

    /* Fill the array */
    clear_formation_ranks();
    int max = get_max_ranks();

    for (int i = 0; i < max; i++) {
        assign_formation_rank(i + 1, i);
    }

    /* Clear it */
    clear_formation_ranks();

    if (get_num_ranks() != 0) {
        printf("  FAIL: Clear didn't reset count\n");
        return 0;
    }

    /* Should be able to add again */
    int result = assign_formation_rank(1, 1);
    if (result != 1) {
        printf("  FAIL: Assignment should work after clear\n");
        return 0;
    }

    printf("  PASS: Array correctly cleared and reusable\n");
    return 1;
}

int main(void)
{
    int passed = 0;
    int total = 5;

    printf("=== Formation Rank Overflow Test Suite ===\n");
    printf("Testing array bounds checking in rank assignment\n");
    printf("Bug: No bounds check allows array overflow\n\n");

    if (test_normal_assignment()) passed++;
    printf("\n");

    if (test_fill_to_capacity()) passed++;
    printf("\n");

    if (test_overflow_prevention()) passed++;
    printf("\n");

    if (test_multiple_overflow_attempts()) passed++;
    printf("\n");

    if (test_clear_and_reuse()) passed++;

    printf("\n=== Test Results ===\n");
    printf("Results: %d/%d tests passed\n", passed, total);

    return (passed == total) ? 0 : 1;
}
