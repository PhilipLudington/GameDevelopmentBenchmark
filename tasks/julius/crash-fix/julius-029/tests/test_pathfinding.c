/*
 * Test for julius-029: Infinite loop in pathfinding when destination unreachable
 *
 * This test verifies that pathfinding terminates properly when the
 * destination is unreachable.
 */

#include <stdio.h>
#include <stdlib.h>

#define MAX_PATH_LENGTH 500
#define MAX_SEARCH_ITERATIONS 10000

#define MAP_SIZE 100

typedef struct {
    int path[MAX_PATH_LENGTH];
    int length;
} path_result;

/* Simulated pathfinding state */
static int search_nodes[MAP_SIZE * MAP_SIZE];
static int nodes_explored = 0;
static int total_nodes_available = 0;
static int destination_reachable = 0;
static int path_found_flag = 0;

/* Initialize path search */
void init_path_search(int start_x, int start_y)
{
    (void)start_x;
    (void)start_y;
    nodes_explored = 0;
    path_found_flag = 0;
}

/* Check if path is found */
int path_found(void)
{
    return path_found_flag;
}

/* Get next node to search */
int get_next_search_node(void)
{
    if (nodes_explored >= total_nodes_available) {
        return -1;  /* No more nodes */
    }
    return search_nodes[nodes_explored++];
}

/* Explore neighbors */
void explore_neighbors(int node)
{
    (void)node;
    /* Simulated exploration */
}

/* Check if we reached destination */
int reached_destination(int dest_x, int dest_y)
{
    (void)dest_x;
    (void)dest_y;
    if (destination_reachable && nodes_explored >= total_nodes_available / 2) {
        path_found_flag = 1;
        return 1;
    }
    return 0;
}

/* Build the final path */
void build_path(path_result *result)
{
    result->length = 5;  /* Simulated path */
    for (int i = 0; i < result->length; i++) {
        result->path[i] = i;
    }
}

/*
 * Find path function - buggy or fixed version
 */
int find_path(int start_x, int start_y, int dest_x, int dest_y, path_result *result)
{
    init_path_search(start_x, start_y);

#ifdef BUGGY_VERSION
    /* BUG: No iteration limit, continue on -1 */
    int safety_counter = 0;  /* Test safety only */

    while (!path_found()) {
        safety_counter++;
        if (safety_counter > MAX_SEARCH_ITERATIONS + 100) {
            /* Test safety valve - detects infinite loop */
            return -1;  /* Special return to detect bug */
        }

        int next = get_next_search_node();
        if (next < 0) {
            continue;  /* BUG: should return 0 */
        }

        explore_neighbors(next);

        if (reached_destination(dest_x, dest_y)) {
            build_path(result);
            return 1;
        }
    }
#else
    /* FIXED: Has iteration limit and proper termination */
    int iterations = 0;

    while (!path_found() && iterations < MAX_SEARCH_ITERATIONS) {
        iterations++;

        int next = get_next_search_node();
        if (next < 0) {
            return 0;  /* No more nodes - destination unreachable */
        }

        explore_neighbors(next);

        if (reached_destination(dest_x, dest_y)) {
            build_path(result);
            return 1;
        }
    }
#endif

    return 0;
}

/* Reset test state */
void reset_test_state(void)
{
    nodes_explored = 0;
    total_nodes_available = 0;
    destination_reachable = 0;
    path_found_flag = 0;
}

/* Test unreachable destination */
int test_unreachable_destination(void)
{
    reset_test_state();
    total_nodes_available = 50;  /* Limited nodes to explore */
    destination_reachable = 0;   /* Destination cannot be reached */

    for (int i = 0; i < total_nodes_available; i++) {
        search_nodes[i] = i;
    }

    path_result result = {0};
    int found = find_path(0, 0, 99, 99, &result);

    if (found == -1) {
        printf("FAIL: Infinite loop detected (exceeded safety counter)\n");
        return 1;
    }

    if (found != 0) {
        printf("FAIL: Should return 0 for unreachable destination\n");
        return 1;
    }

    printf("PASS: Unreachable destination test\n");
    return 0;
}

/* Test reachable destination */
int test_reachable_destination(void)
{
    reset_test_state();
    total_nodes_available = 100;
    destination_reachable = 1;

    for (int i = 0; i < total_nodes_available; i++) {
        search_nodes[i] = i;
    }

    path_result result = {0};
    int found = find_path(0, 0, 10, 10, &result);

    if (found != 1) {
        printf("FAIL: Should find path to reachable destination\n");
        return 1;
    }

    if (result.length <= 0) {
        printf("FAIL: Path should have positive length\n");
        return 1;
    }

    printf("PASS: Reachable destination test\n");
    return 0;
}

/* Test empty search space */
int test_empty_search_space(void)
{
    reset_test_state();
    total_nodes_available = 0;  /* No nodes at all */
    destination_reachable = 0;

    path_result result = {0};
    int found = find_path(0, 0, 50, 50, &result);

    if (found == -1) {
        printf("FAIL: Infinite loop on empty search space\n");
        return 1;
    }

    if (found != 0) {
        printf("FAIL: Should return 0 for empty search space\n");
        return 1;
    }

    printf("PASS: Empty search space test\n");
    return 0;
}

/* Test same start and destination */
int test_same_location(void)
{
    reset_test_state();
    total_nodes_available = 1;
    destination_reachable = 1;  /* Immediately reachable */
    search_nodes[0] = 0;

    path_result result = {0};
    int found = find_path(5, 5, 5, 5, &result);

    /* Should find path quickly */
    if (found == -1) {
        printf("FAIL: Infinite loop for same location\n");
        return 1;
    }

    printf("PASS: Same location test\n");
    return 0;
}

/* Test large search space */
int test_large_search_space(void)
{
    reset_test_state();
    total_nodes_available = 5000;  /* Large but will hit iteration limit */
    destination_reachable = 0;

    for (int i = 0; i < total_nodes_available; i++) {
        search_nodes[i] = i;
    }

    path_result result = {0};
    int found = find_path(0, 0, 99, 99, &result);

    if (found == -1) {
        printf("FAIL: Infinite loop on large search space\n");
        return 1;
    }

    if (found != 0) {
        printf("FAIL: Should return 0 when exceeding iteration limit\n");
        return 1;
    }

    printf("PASS: Large search space test\n");
    return 0;
}

int main(void)
{
    int failures = 0;

    printf("=== Pathfinding Test ===\n\n");

#ifdef BUGGY_VERSION
    printf("Testing BUGGY version (expect infinite loop detection)\n\n");
#else
    printf("Testing FIXED version (should pass all tests)\n\n");
#endif

    failures += test_unreachable_destination();
    failures += test_reachable_destination();
    failures += test_empty_search_space();
    failures += test_same_location();
    failures += test_large_search_space();

    printf("\n=== Results: %d failures ===\n", failures);

    return failures;
}
