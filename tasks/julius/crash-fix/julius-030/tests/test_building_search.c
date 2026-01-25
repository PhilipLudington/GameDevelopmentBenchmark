/*
 * Test for julius-030: Stack overflow from deep recursion in building search
 *
 * This test verifies that building search terminates properly even with
 * deeply connected building chains.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BUILDINGS 2000
#define MAX_CONNECTIONS 4
#define MAX_SEARCH_DEPTH 500
#define BUILDING_STATE_ACTIVE 1
#define BUILDING_STATE_DELETED 0

typedef struct {
    int id;
    int state;
    int num_connections;
    int connections[MAX_CONNECTIONS];
} building;

/* Simulated building data */
static building buildings[MAX_BUILDINGS];
static int visited[MAX_BUILDINGS];
static int recursion_depth = 0;
static int max_recursion_depth = 0;

/* Get building by ID */
building *building_get(int id)
{
    if (id < 0 || id >= MAX_BUILDINGS) {
        return NULL;
    }
    return &buildings[id];
}

/* Check if already visited */
int is_already_visited(int building_id)
{
    return visited[building_id];
}

/* Mark as visited */
void mark_visited(int building_id)
{
    visited[building_id] = 1;
}

/* Reset visited array */
void reset_visited(void)
{
    memset(visited, 0, sizeof(visited));
}

#ifdef BUGGY_VERSION
/*
 * Buggy version - no depth limit
 */
void search_connected_buildings(int building_id, int *count, int *building_ids)
{
    recursion_depth++;
    if (recursion_depth > max_recursion_depth) {
        max_recursion_depth = recursion_depth;
    }

    /* Safety limit for testing - detect the bug without actual crash */
    if (recursion_depth > MAX_SEARCH_DEPTH + 100) {
        recursion_depth--;
        return;  /* Would have crashed in real code */
    }

    building *b = building_get(building_id);
    if (!b || b->state != BUILDING_STATE_ACTIVE) {
        recursion_depth--;
        return;
    }

    if (is_already_visited(building_id)) {
        recursion_depth--;
        return;
    }

    mark_visited(building_id);
    building_ids[*count] = building_id;
    (*count)++;

    for (int i = 0; i < b->num_connections; i++) {
        search_connected_buildings(b->connections[i], count, building_ids);
    }

    recursion_depth--;
}

#else
/*
 * Fixed version - with depth limit
 */
static void search_connected_buildings_impl(int building_id, int *count,
                                             int *building_ids, int depth)
{
    recursion_depth++;
    if (recursion_depth > max_recursion_depth) {
        max_recursion_depth = recursion_depth;
    }

    if (depth >= MAX_SEARCH_DEPTH) {
        recursion_depth--;
        return;
    }

    building *b = building_get(building_id);
    if (!b || b->state != BUILDING_STATE_ACTIVE) {
        recursion_depth--;
        return;
    }

    if (is_already_visited(building_id)) {
        recursion_depth--;
        return;
    }

    mark_visited(building_id);
    building_ids[*count] = building_id;
    (*count)++;

    for (int i = 0; i < b->num_connections; i++) {
        search_connected_buildings_impl(b->connections[i], count,
                                        building_ids, depth + 1);
    }

    recursion_depth--;
}

void search_connected_buildings(int building_id, int *count, int *building_ids)
{
    search_connected_buildings_impl(building_id, count, building_ids, 0);
}
#endif

/* Reset test state */
void reset_test_state(void)
{
    memset(buildings, 0, sizeof(buildings));
    reset_visited();
    recursion_depth = 0;
    max_recursion_depth = 0;
}

/* Create a chain of connected buildings */
void create_building_chain(int length)
{
    for (int i = 0; i < length && i < MAX_BUILDINGS - 1; i++) {
        buildings[i].id = i;
        buildings[i].state = BUILDING_STATE_ACTIVE;
        buildings[i].num_connections = 1;
        buildings[i].connections[0] = i + 1;  /* Connect to next */
    }
    /* Last building has no connections */
    buildings[length - 1].num_connections = 0;
}

/* Create a tree of connected buildings */
void create_building_tree(int depth, int branching)
{
    int id = 0;
    int queue[MAX_BUILDINGS];
    int depths[MAX_BUILDINGS];
    int front = 0, back = 0;

    buildings[0].id = 0;
    buildings[0].state = BUILDING_STATE_ACTIVE;
    queue[back] = 0;
    depths[back++] = 0;
    id = 1;

    while (front < back && id < MAX_BUILDINGS) {
        int current = queue[front];
        int current_depth = depths[front++];

        if (current_depth >= depth) continue;

        buildings[current].num_connections = 0;
        for (int i = 0; i < branching && id < MAX_BUILDINGS; i++) {
            buildings[id].id = id;
            buildings[id].state = BUILDING_STATE_ACTIVE;
            buildings[current].connections[buildings[current].num_connections++] = id;
            queue[back] = id;
            depths[back++] = current_depth + 1;
            id++;
        }
    }
}

/* Test deep chain search */
int test_deep_chain(void)
{
    reset_test_state();

    /* Create a chain of 1000 buildings */
    create_building_chain(1000);

    int count = 0;
    int building_ids[MAX_BUILDINGS];
    search_connected_buildings(0, &count, building_ids);

    printf("Deep chain: found %d buildings, max depth %d\n", count, max_recursion_depth);

    /* In fixed version, should be limited by MAX_SEARCH_DEPTH (allow +1 for the depth check timing) */
#ifdef BUGGY_VERSION
    if (max_recursion_depth > MAX_SEARCH_DEPTH + 100) {
        printf("FAIL: Recursion depth %d exceeded safety limit %d\n",
               max_recursion_depth, MAX_SEARCH_DEPTH + 100);
        return 1;
    }
#else
    /* Allow +1 because recursion_depth increments before the depth check */
    if (max_recursion_depth > MAX_SEARCH_DEPTH + 1) {
        printf("FAIL: Fixed version should limit depth to ~%d, got %d\n",
               MAX_SEARCH_DEPTH, max_recursion_depth);
        return 1;
    }
#endif

    printf("PASS: Deep chain test\n");
    return 0;
}

/* Test normal search */
int test_normal_search(void)
{
    reset_test_state();

    /* Create a small chain */
    create_building_chain(10);

    int count = 0;
    int building_ids[MAX_BUILDINGS];
    search_connected_buildings(0, &count, building_ids);

    if (count != 10) {
        printf("FAIL: Expected 10 buildings, found %d\n", count);
        return 1;
    }

    printf("PASS: Normal search test\n");
    return 0;
}

/* Test tree search */
int test_tree_search(void)
{
    reset_test_state();

    /* Create a balanced tree */
    create_building_tree(5, 3);

    int count = 0;
    int building_ids[MAX_BUILDINGS];
    search_connected_buildings(0, &count, building_ids);

    printf("Tree search: found %d buildings, max depth %d\n", count, max_recursion_depth);

    if (count == 0) {
        printf("FAIL: Should find at least one building\n");
        return 1;
    }

    if (max_recursion_depth > MAX_SEARCH_DEPTH) {
        printf("FAIL: Recursion too deep in tree search\n");
        return 1;
    }

    printf("PASS: Tree search test\n");
    return 0;
}

/* Test single building */
int test_single_building(void)
{
    reset_test_state();

    buildings[0].id = 0;
    buildings[0].state = BUILDING_STATE_ACTIVE;
    buildings[0].num_connections = 0;

    int count = 0;
    int building_ids[MAX_BUILDINGS];
    search_connected_buildings(0, &count, building_ids);

    if (count != 1) {
        printf("FAIL: Expected 1 building, found %d\n", count);
        return 1;
    }

    printf("PASS: Single building test\n");
    return 0;
}

/* Test circular reference (should not infinite loop) */
int test_circular_reference(void)
{
    reset_test_state();

    /* Create circular chain: 0 -> 1 -> 2 -> 0 */
    for (int i = 0; i < 3; i++) {
        buildings[i].id = i;
        buildings[i].state = BUILDING_STATE_ACTIVE;
        buildings[i].num_connections = 1;
        buildings[i].connections[0] = (i + 1) % 3;
    }

    int count = 0;
    int building_ids[MAX_BUILDINGS];
    search_connected_buildings(0, &count, building_ids);

    if (count != 3) {
        printf("FAIL: Circular reference - expected 3, found %d\n", count);
        return 1;
    }

    printf("PASS: Circular reference test\n");
    return 0;
}

int main(void)
{
    int failures = 0;

    printf("=== Building Search Test ===\n\n");

#ifdef BUGGY_VERSION
    printf("Testing BUGGY version (no depth limit)\n\n");
#else
    printf("Testing FIXED version (with depth limit)\n\n");
#endif

    failures += test_single_building();
    failures += test_normal_search();
    failures += test_circular_reference();
    failures += test_tree_search();
    failures += test_deep_chain();

    printf("\n=== Results: %d failures ===\n", failures);

    return failures;
}
