/**
 * Test for granary cart pathfinding loop (julius-038)
 *
 * This test verifies that the route following arrival check
 * compares against destination, not source.
 *
 * Test validation:
 * - On FIXED code: Cart arrives at destination
 * - On BUGGY code: Cart never arrives (infinite loop)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Route status */
#define ROUTE_CONTINUE 0
#define ROUTE_ARRIVED 1

/* Waypoint structure */
typedef struct {
    int x;
    int y;
} route_waypoint;

/* Figure structure */
typedef struct {
    int x;
    int y;
    int route_start_x;
    int route_start_y;
    int destination_x;
    int destination_y;
    int route_index;
    route_waypoint *route;
    int route_length;
} figure;

/* External functions from stubs */
extern void test_setup_route(figure *f, int start_x, int start_y, int dest_x, int dest_y);
extern int figure_route_follow(figure *f);
extern int simulate_route_follow(figure *f, int max_steps);
extern void figure_get_position(figure *f, int *x, int *y);

/**
 * Test 1: Cart should arrive at destination
 *
 * Route: (0,0) -> (5,5) -> (10,10)
 * Should arrive at (10,10) in 3 steps or less
 */
static int test_arrives_at_destination(void)
{
    printf("Test: Cart arrives at destination...\n");

    figure cart;
    memset(&cart, 0, sizeof(cart));
    test_setup_route(&cart, 0, 0, 10, 10);

    printf("  Start: (0,0), Destination: (10,10)\n");

    int steps = simulate_route_follow(&cart, 10);

    if (steps < 0) {
        printf("  FAIL: Cart did not arrive within 10 steps (stuck in loop!)\n");
        printf("  This is the bug - arrival check uses wrong coordinates\n");
        return 0;
    }

    printf("  Cart arrived in %d steps\n", steps);

    int x, y;
    figure_get_position(&cart, &x, &y);
    printf("  Final position: (%d,%d)\n", x, y);

    if (x != 10 || y != 10) {
        printf("  FAIL: Cart at wrong position (expected 10,10)\n");
        return 0;
    }

    printf("  PASS: Cart correctly arrived at destination\n");
    return 1;
}

/**
 * Test 2: Cart doesn't stop at source
 *
 * With buggy code, cart might stop if it returns to source
 */
static int test_doesnt_stop_at_source(void)
{
    printf("Test: Cart doesn't falsely stop at source...\n");

    figure cart;
    memset(&cart, 0, sizeof(cart));
    test_setup_route(&cart, 5, 5, 15, 15);

    printf("  Start: (5,5), Destination: (15,15)\n");

    /* Follow route once - should not arrive */
    int status = figure_route_follow(&cart);

    int x, y;
    figure_get_position(&cart, &x, &y);
    printf("  After first step: position=(%d,%d), status=%s\n",
           x, y, status == ROUTE_ARRIVED ? "ARRIVED" : "CONTINUE");

    /* Cart shouldn't have arrived yet (at midpoint) */
    if (status == ROUTE_ARRIVED && (x != 15 || y != 15)) {
        printf("  FAIL: Cart incorrectly reported arrival before destination\n");
        return 0;
    }

    printf("  PASS: Cart continues toward destination\n");
    return 1;
}

/**
 * Test 3: Different source and destination
 *
 * Key test: source (0,0) != destination (20,20)
 */
static int test_different_source_dest(void)
{
    printf("Test: Cart with distant source and destination...\n");

    figure cart;
    memset(&cart, 0, sizeof(cart));
    test_setup_route(&cart, 0, 0, 20, 20);

    printf("  Start: (0,0), Destination: (20,20)\n");

    int steps = simulate_route_follow(&cart, 10);

    if (steps < 0) {
        printf("  FAIL: Cart stuck - comparing with source instead of destination\n");
        return 0;
    }

    int x, y;
    figure_get_position(&cart, &x, &y);

    if (x != 20 || y != 20) {
        printf("  FAIL: Cart at (%d,%d), expected (20,20)\n", x, y);
        return 0;
    }

    printf("  PASS: Cart arrived at correct destination\n");
    return 1;
}

/**
 * Test 4: Source equals destination (edge case)
 *
 * In this case, both buggy and fixed code should work
 */
static int test_source_equals_dest(void)
{
    printf("Test: Source equals destination (edge case)...\n");

    figure cart;
    memset(&cart, 0, sizeof(cart));
    test_setup_route(&cart, 10, 10, 10, 10);

    printf("  Start: (10,10), Destination: (10,10)\n");

    int steps = simulate_route_follow(&cart, 5);

    if (steps < 0) {
        printf("  FAIL: Cart did not arrive\n");
        return 0;
    }

    printf("  Cart arrived in %d steps\n", steps);
    printf("  PASS: Edge case handled correctly\n");
    return 1;
}

/**
 * Test 5: Verify no infinite loop
 *
 * Run for many steps and verify arrival happens
 */
static int test_no_infinite_loop(void)
{
    printf("Test: No infinite loop (100 step limit)...\n");

    figure cart;
    memset(&cart, 0, sizeof(cart));
    test_setup_route(&cart, 1, 1, 50, 50);

    printf("  Start: (1,1), Destination: (50,50)\n");

    int steps = simulate_route_follow(&cart, 100);

    if (steps < 0) {
        printf("  FAIL: Cart did not arrive after 100 steps - INFINITE LOOP!\n");
        printf("  Cart is comparing position against source, not destination\n");
        return 0;
    }

    printf("  Cart arrived in %d steps\n", steps);
    printf("  PASS: No infinite loop detected\n");
    return 1;
}

int main(void)
{
    int passed = 0;
    int total = 5;

    printf("=== Granary Cart Pathfinding Test Suite ===\n");
    printf("Testing route following arrival detection\n");
    printf("Bug: Arrival check compares against source instead of destination\n\n");

    if (test_arrives_at_destination()) passed++;
    printf("\n");

    if (test_doesnt_stop_at_source()) passed++;
    printf("\n");

    if (test_different_source_dest()) passed++;
    printf("\n");

    if (test_source_equals_dest()) passed++;
    printf("\n");

    if (test_no_infinite_loop()) passed++;

    printf("\n=== Test Results ===\n");
    printf("Results: %d/%d tests passed\n", passed, total);

    return (passed == total) ? 0 : 1;
}
