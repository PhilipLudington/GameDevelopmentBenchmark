/*
 * Test for julius-023: Water building data corruption on collapse
 *
 * This test verifies that collapsing a water building properly clears
 * its water network data.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUILDING_STATE_ACTIVE 1
#define BUILDING_STATE_DELETED 0

#define BUILDING_HOUSE 1
#define BUILDING_RESERVOIR 10
#define BUILDING_FOUNTAIN 11
#define BUILDING_WELL 12

#define MAX_BUILDINGS 100
#define MAP_SIZE 1000

typedef struct {
    int id;
    int type;
    int state;
    int grid_offset;
    int has_water;
} building;

/* Simulated water network grid */
static int water_network[MAP_SIZE];
static int water_clear_called = 0;
static int last_cleared_building_id = -1;

/* Track if building is water type */
int building_is_water_type(int type)
{
    return type == BUILDING_RESERVOIR ||
           type == BUILDING_FOUNTAIN ||
           type == BUILDING_WELL;
}

/* Simulated water network clear */
void water_network_clear_building(building *b)
{
    water_clear_called = 1;
    last_cleared_building_id = b->id;

    /* Clear network data for building's grid offset */
    if (b->grid_offset >= 0 && b->grid_offset < MAP_SIZE) {
        water_network[b->grid_offset] = 0;
    }
}

/* Simulated map clear */
void map_building_clear(int grid_offset)
{
    (void)grid_offset;
}

/* Simulated house collapse handling */
void handle_house_collapse(building *b)
{
    (void)b;
}

/*
 * Building collapse function - buggy or fixed version
 */
void building_collapse(building *b)
{
    map_building_clear(b->grid_offset);

    if (b->type == BUILDING_HOUSE) {
        handle_house_collapse(b);
    }

#ifdef BUGGY_VERSION
    /* BUG: Water buildings not handled */
#else
    /* FIXED: Clear water network data for water buildings */
    if (building_is_water_type(b->type)) {
        water_network_clear_building(b);
    }
#endif

    b->state = BUILDING_STATE_DELETED;
}

/* Reset test state */
void reset_test_state(void)
{
    memset(water_network, 0, sizeof(water_network));
    water_clear_called = 0;
    last_cleared_building_id = -1;
}

/* Test collapsing a reservoir */
int test_reservoir_collapse(void)
{
    reset_test_state();

    building reservoir = {
        .id = 1,
        .type = BUILDING_RESERVOIR,
        .state = BUILDING_STATE_ACTIVE,
        .grid_offset = 100,
        .has_water = 1
    };

    /* Set up water network data */
    water_network[100] = 1;

    printf("Collapsing reservoir...\n");
    building_collapse(&reservoir);

    /* Verify building state changed */
    if (reservoir.state != BUILDING_STATE_DELETED) {
        printf("FAIL: Building state not set to deleted\n");
        return 1;
    }

    /* Verify water network was cleared */
    if (!water_clear_called) {
        printf("FAIL: Water network clear not called for reservoir\n");
        return 1;
    }

    if (last_cleared_building_id != 1) {
        printf("FAIL: Wrong building cleared (expected 1, got %d)\n",
               last_cleared_building_id);
        return 1;
    }

    printf("PASS: Reservoir collapse test\n");
    return 0;
}

/* Test collapsing a fountain */
int test_fountain_collapse(void)
{
    reset_test_state();

    building fountain = {
        .id = 5,
        .type = BUILDING_FOUNTAIN,
        .state = BUILDING_STATE_ACTIVE,
        .grid_offset = 200,
        .has_water = 1
    };

    water_network[200] = 1;

    printf("Collapsing fountain...\n");
    building_collapse(&fountain);

    if (!water_clear_called) {
        printf("FAIL: Water network clear not called for fountain\n");
        return 1;
    }

    printf("PASS: Fountain collapse test\n");
    return 0;
}

/* Test collapsing a non-water building (should not clear water network) */
int test_house_collapse(void)
{
    reset_test_state();

    building house = {
        .id = 10,
        .type = BUILDING_HOUSE,
        .state = BUILDING_STATE_ACTIVE,
        .grid_offset = 300,
        .has_water = 0
    };

    building_collapse(&house);

    /* Water clear should NOT be called for houses */
    if (water_clear_called) {
        printf("FAIL: Water network clear should not be called for house\n");
        return 1;
    }

    printf("PASS: House collapse test (water not cleared)\n");
    return 0;
}

/* Test multiple water buildings */
int test_multiple_water_buildings(void)
{
    reset_test_state();

    building buildings[3] = {
        {.id = 1, .type = BUILDING_RESERVOIR, .state = BUILDING_STATE_ACTIVE, .grid_offset = 100},
        {.id = 2, .type = BUILDING_FOUNTAIN, .state = BUILDING_STATE_ACTIVE, .grid_offset = 200},
        {.id = 3, .type = BUILDING_WELL, .state = BUILDING_STATE_ACTIVE, .grid_offset = 300}
    };

    water_network[100] = 1;
    water_network[200] = 1;
    water_network[300] = 1;

    int clear_count = 0;

    for (int i = 0; i < 3; i++) {
        water_clear_called = 0;
        building_collapse(&buildings[i]);
        if (water_clear_called) clear_count++;
    }

    if (clear_count != 3) {
        printf("FAIL: Expected 3 water clears, got %d\n", clear_count);
        return 1;
    }

    printf("PASS: Multiple water buildings test\n");
    return 0;
}

int main(void)
{
    int failures = 0;

    printf("=== Water Building Collapse Test ===\n\n");

#ifdef BUGGY_VERSION
    printf("Testing BUGGY version (expect water network not cleared)\n\n");
#else
    printf("Testing FIXED version (should pass all tests)\n\n");
#endif

    failures += test_reservoir_collapse();
    failures += test_fountain_collapse();
    failures += test_house_collapse();
    failures += test_multiple_water_buildings();

    printf("\n=== Results: %d failures ===\n", failures);

    return failures;
}
