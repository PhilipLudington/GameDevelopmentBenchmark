/**
 * Test for waypoint deletion corruption (julius-039)
 *
 * This test verifies that deleting a waypoint properly compacts
 * the array by shifting remaining elements.
 *
 * Test validation:
 * - On FIXED code: Array compacted, waypoints shifted correctly
 * - On BUGGY code: Array not compacted, stale data remains
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_WAYPOINTS 20

/* Waypoint structure */
typedef struct {
    int x;
    int y;
    int id;
} waypoint;

/* Route data structure */
typedef struct {
    waypoint waypoints[MAX_WAYPOINTS];
    int num_waypoints;
} route_data;

/* External functions from stubs */
extern void route_delete_waypoint(route_data *route, int index);
extern void route_init_test(route_data *route, int count);
extern waypoint *route_get_waypoint(route_data *route, int index);
extern int route_get_count(route_data *route);

/**
 * Test 1: Delete from middle and verify shift
 *
 * Start: [1, 2, 3, 4, 5] (IDs)
 * Delete index 2 (ID=3)
 * Expected: [1, 2, 4, 5]
 */
static int test_delete_middle(void)
{
    printf("Test: Delete from middle and verify shift...\n");

    route_data route;
    route_init_test(&route, 5);

    printf("  Before: [");
    for (int i = 0; i < route_get_count(&route); i++) {
        printf("%d", route_get_waypoint(&route, i)->id);
        if (i < route_get_count(&route) - 1) printf(", ");
    }
    printf("]\n");

    /* Delete index 2 (ID=3) */
    route_delete_waypoint(&route, 2);

    printf("  After delete index 2: [");
    for (int i = 0; i < route_get_count(&route); i++) {
        printf("%d", route_get_waypoint(&route, i)->id);
        if (i < route_get_count(&route) - 1) printf(", ");
    }
    printf("]\n");

    /* Verify count */
    if (route_get_count(&route) != 4) {
        printf("  FAIL: Expected count=4, got %d\n", route_get_count(&route));
        return 0;
    }

    /* Verify waypoint at index 2 is now ID=4 (was ID=3) */
    waypoint *wp = route_get_waypoint(&route, 2);
    if (wp->id != 4) {
        printf("  FAIL: Waypoint at index 2 has ID=%d, expected 4\n", wp->id);
        printf("  This is the bug - array was not compacted!\n");
        return 0;
    }

    /* Verify full sequence: 1, 2, 4, 5 */
    int expected[] = {1, 2, 4, 5};
    for (int i = 0; i < 4; i++) {
        if (route_get_waypoint(&route, i)->id != expected[i]) {
            printf("  FAIL: Waypoint sequence incorrect at index %d\n", i);
            return 0;
        }
    }

    printf("  PASS: Array properly compacted\n");
    return 1;
}

/**
 * Test 2: Delete first element
 *
 * Start: [1, 2, 3]
 * Delete index 0
 * Expected: [2, 3]
 */
static int test_delete_first(void)
{
    printf("Test: Delete first element...\n");

    route_data route;
    route_init_test(&route, 3);

    route_delete_waypoint(&route, 0);

    printf("  After delete: [");
    for (int i = 0; i < route_get_count(&route); i++) {
        printf("%d", route_get_waypoint(&route, i)->id);
        if (i < route_get_count(&route) - 1) printf(", ");
    }
    printf("]\n");

    if (route_get_count(&route) != 2) {
        printf("  FAIL: Expected count=2, got %d\n", route_get_count(&route));
        return 0;
    }

    if (route_get_waypoint(&route, 0)->id != 2) {
        printf("  FAIL: First element should be ID=2\n");
        return 0;
    }

    printf("  PASS: First element deleted correctly\n");
    return 1;
}

/**
 * Test 3: Delete last element
 *
 * Start: [1, 2, 3]
 * Delete index 2 (last)
 * Expected: [1, 2]
 */
static int test_delete_last(void)
{
    printf("Test: Delete last element...\n");

    route_data route;
    route_init_test(&route, 3);

    route_delete_waypoint(&route, 2);

    printf("  After delete: [");
    for (int i = 0; i < route_get_count(&route); i++) {
        printf("%d", route_get_waypoint(&route, i)->id);
        if (i < route_get_count(&route) - 1) printf(", ");
    }
    printf("]\n");

    if (route_get_count(&route) != 2) {
        printf("  FAIL: Expected count=2, got %d\n", route_get_count(&route));
        return 0;
    }

    /* Verify 1, 2 remain */
    if (route_get_waypoint(&route, 0)->id != 1 ||
        route_get_waypoint(&route, 1)->id != 2) {
        printf("  FAIL: Remaining elements incorrect\n");
        return 0;
    }

    printf("  PASS: Last element deleted correctly\n");
    return 1;
}

/**
 * Test 4: Delete only element
 *
 * Start: [1]
 * Delete index 0
 * Expected: []
 */
static int test_delete_only(void)
{
    printf("Test: Delete only element...\n");

    route_data route;
    route_init_test(&route, 1);

    route_delete_waypoint(&route, 0);

    if (route_get_count(&route) != 0) {
        printf("  FAIL: Expected count=0, got %d\n", route_get_count(&route));
        return 0;
    }

    printf("  PASS: Single element deleted correctly\n");
    return 1;
}

/**
 * Test 5: Multiple deletions
 *
 * Start: [1, 2, 3, 4, 5]
 * Delete index 1, then index 2
 * After first: [1, 3, 4, 5]
 * After second: [1, 3, 5]
 */
static int test_multiple_deletes(void)
{
    printf("Test: Multiple consecutive deletions...\n");

    route_data route;
    route_init_test(&route, 5);

    /* Delete index 1 (ID=2) */
    route_delete_waypoint(&route, 1);

    printf("  After first delete: [");
    for (int i = 0; i < route_get_count(&route); i++) {
        printf("%d", route_get_waypoint(&route, i)->id);
        if (i < route_get_count(&route) - 1) printf(", ");
    }
    printf("]\n");

    /* Delete index 2 (now ID=4) */
    route_delete_waypoint(&route, 2);

    printf("  After second delete: [");
    for (int i = 0; i < route_get_count(&route); i++) {
        printf("%d", route_get_waypoint(&route, i)->id);
        if (i < route_get_count(&route) - 1) printf(", ");
    }
    printf("]\n");

    if (route_get_count(&route) != 3) {
        printf("  FAIL: Expected count=3, got %d\n", route_get_count(&route));
        return 0;
    }

    /* Verify sequence: 1, 3, 5 */
    int expected[] = {1, 3, 5};
    for (int i = 0; i < 3; i++) {
        if (route_get_waypoint(&route, i)->id != expected[i]) {
            printf("  FAIL: Expected ID=%d at index %d, got %d\n",
                   expected[i], i, route_get_waypoint(&route, i)->id);
            return 0;
        }
    }

    printf("  PASS: Multiple deletions work correctly\n");
    return 1;
}

/**
 * Test 6: Invalid index (should not crash)
 */
static int test_invalid_index(void)
{
    printf("Test: Invalid deletion indices...\n");

    route_data route;
    route_init_test(&route, 3);

    /* Try invalid indices */
    route_delete_waypoint(&route, -1);
    route_delete_waypoint(&route, 10);
    route_delete_waypoint(&route, 3);

    /* Should still have 3 waypoints */
    if (route_get_count(&route) != 3) {
        printf("  FAIL: Invalid deletes should not change count\n");
        return 0;
    }

    printf("  PASS: Invalid indices handled safely\n");
    return 1;
}

int main(void)
{
    int passed = 0;
    int total = 6;

    printf("=== Waypoint Deletion Test Suite ===\n");
    printf("Testing array compaction on waypoint delete\n");
    printf("Bug: Array not compacted, stale data remains\n\n");

    if (test_delete_middle()) passed++;
    printf("\n");

    if (test_delete_first()) passed++;
    printf("\n");

    if (test_delete_last()) passed++;
    printf("\n");

    if (test_delete_only()) passed++;
    printf("\n");

    if (test_multiple_deletes()) passed++;
    printf("\n");

    if (test_invalid_index()) passed++;

    printf("\n=== Test Results ===\n");
    printf("Results: %d/%d tests passed\n", passed, total);

    return (passed == total) ? 0 : 1;
}
