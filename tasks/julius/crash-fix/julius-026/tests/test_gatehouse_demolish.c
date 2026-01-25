/*
 * Test for julius-026: Gatehouse infinite road respawn bug
 *
 * This test verifies that demolishing a gatehouse does not cause
 * an infinite loop of road recreation.
 */

#include <stdio.h>
#include <stdlib.h>

#define BUILDING_STATE_ACTIVE 1
#define BUILDING_STATE_DELETED 0

#define BUILDING_GATEHOUSE 30
#define BUILDING_HOUSE 1
#define BUILDING_ROAD 2

#define MAX_RECURSION_LIMIT 100

typedef struct {
    int id;
    int type;
    int state;
    int grid_offset;
    int size;
} building;

/* Test tracking variables */
static int demolish_call_count = 0;
static int road_restore_count = 0;
static int network_update_count = 0;
static int demolition_in_progress = 0;

/* Forward declaration */
void demolish_building(building *b);

/* Simulated functions */
void clear_building_from_map(building *b)
{
    (void)b;
}

void restore_road_tiles(int grid_offset, int size)
{
    (void)grid_offset;
    (void)size;
    road_restore_count++;
}

/* This simulates the road network update that could trigger gatehouse rebuild */
void update_road_network(int grid_offset)
{
    (void)grid_offset;
    network_update_count++;

    /* Simulate the bug: road network update tries to recreate gatehouse */
    /* which then gets demolished again (infinite loop) */
    if (network_update_count < MAX_RECURSION_LIMIT) {
        /* Create a fake gatehouse that needs demolishing */
        building fake_gatehouse = {
            .id = 999,
            .type = BUILDING_GATEHOUSE,
            .state = BUILDING_STATE_ACTIVE,
            .grid_offset = grid_offset,
            .size = 2
        };
        /* Try to demolish the "rebuilt" gatehouse */
        demolish_building(&fake_gatehouse);
    }
}

/*
 * Demolish building function - buggy or fixed version
 */
void demolish_building(building *b)
{
    demolish_call_count++;

    /* Safety check to prevent infinite loop in buggy version */
    if (demolish_call_count > MAX_RECURSION_LIMIT) {
        return;  /* Prevent actual infinite loop in test */
    }

#ifdef BUGGY_VERSION
    /* BUG: No protection against recursive calls */
#else
    /* FIXED: Prevent recursive demolition */
    if (demolition_in_progress) {
        return;
    }
    demolition_in_progress = 1;
#endif

    int grid_offset = b->grid_offset;
    clear_building_from_map(b);
    b->state = BUILDING_STATE_DELETED;

    if (b->type == BUILDING_GATEHOUSE) {
        restore_road_tiles(grid_offset, b->size);
    }

    update_road_network(grid_offset);

#ifndef BUGGY_VERSION
    demolition_in_progress = 0;
#endif
}

/* Reset test state */
void reset_test_state(void)
{
    demolish_call_count = 0;
    road_restore_count = 0;
    network_update_count = 0;
    demolition_in_progress = 0;
}

/* Test demolishing a gatehouse */
int test_gatehouse_demolish(void)
{
    reset_test_state();

    building gatehouse = {
        .id = 1,
        .type = BUILDING_GATEHOUSE,
        .state = BUILDING_STATE_ACTIVE,
        .grid_offset = 100,
        .size = 2
    };

    demolish_building(&gatehouse);

    if (gatehouse.state != BUILDING_STATE_DELETED) {
        printf("FAIL: Gatehouse not marked as deleted\n");
        return 1;
    }

    /* In fixed version, should only be called once */
    if (demolish_call_count > 2) {  /* Allow 2 for initial + one recursive attempt */
        printf("FAIL: Recursive demolition detected (%d calls)\n", demolish_call_count);
        return 1;
    }

    if (road_restore_count != 1) {
        printf("FAIL: Road restore called %d times, expected 1\n", road_restore_count);
        return 1;
    }

    printf("PASS: Gatehouse demolish test (calls: %d)\n", demolish_call_count);
    return 0;
}

/* Test that non-gatehouse buildings are not affected */
int test_house_demolish(void)
{
    reset_test_state();

    building house = {
        .id = 2,
        .type = BUILDING_HOUSE,
        .state = BUILDING_STATE_ACTIVE,
        .grid_offset = 200,
        .size = 1
    };

    demolish_building(&house);

    if (house.state != BUILDING_STATE_DELETED) {
        printf("FAIL: House not marked as deleted\n");
        return 1;
    }

    /* Roads should not be restored for houses */
    if (road_restore_count != 0) {
        printf("FAIL: Road restore called for house\n");
        return 1;
    }

    printf("PASS: House demolish test\n");
    return 0;
}

/* Test multiple sequential demolitions */
int test_multiple_demolitions(void)
{
    reset_test_state();

    building buildings[3] = {
        {.id = 1, .type = BUILDING_GATEHOUSE, .state = BUILDING_STATE_ACTIVE, .grid_offset = 100, .size = 2},
        {.id = 2, .type = BUILDING_HOUSE, .state = BUILDING_STATE_ACTIVE, .grid_offset = 200, .size = 1},
        {.id = 3, .type = BUILDING_GATEHOUSE, .state = BUILDING_STATE_ACTIVE, .grid_offset = 300, .size = 2}
    };

    for (int i = 0; i < 3; i++) {
        reset_test_state();  /* Reset for each demolition */
        demolish_building(&buildings[i]);

        if (demolish_call_count > 2) {
            printf("FAIL: Recursive demolition in building %d\n", i);
            return 1;
        }
    }

    printf("PASS: Multiple demolitions test\n");
    return 0;
}

/* Test that the recursion guard resets properly */
int test_guard_reset(void)
{
    reset_test_state();

    building g1 = {.id = 1, .type = BUILDING_GATEHOUSE, .state = BUILDING_STATE_ACTIVE, .grid_offset = 100, .size = 2};
    building g2 = {.id = 2, .type = BUILDING_GATEHOUSE, .state = BUILDING_STATE_ACTIVE, .grid_offset = 200, .size = 2};

    demolish_building(&g1);
    int first_calls = demolish_call_count;

    reset_test_state();
    demolish_building(&g2);
    int second_calls = demolish_call_count;

    /* Both should complete successfully (guard should reset) */
    if (g1.state != BUILDING_STATE_DELETED || g2.state != BUILDING_STATE_DELETED) {
        printf("FAIL: Buildings not properly demolished\n");
        return 1;
    }

    if (first_calls > 2 || second_calls > 2) {
        printf("FAIL: Guard not resetting properly\n");
        return 1;
    }

    printf("PASS: Guard reset test\n");
    return 0;
}

int main(void)
{
    int failures = 0;

    printf("=== Gatehouse Demolition Test ===\n\n");

#ifdef BUGGY_VERSION
    printf("Testing BUGGY version (expect recursive demolition)\n\n");
#else
    printf("Testing FIXED version (should pass all tests)\n\n");
#endif

    failures += test_gatehouse_demolish();
    failures += test_house_demolish();
    failures += test_multiple_demolitions();
    failures += test_guard_reset();

    printf("\n=== Results: %d failures ===\n", failures);

    return failures;
}
