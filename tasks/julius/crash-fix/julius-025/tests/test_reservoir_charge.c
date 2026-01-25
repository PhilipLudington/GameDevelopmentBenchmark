/*
 * Test for julius-025: Reservoir placement double-charge bug
 *
 * This test verifies that placing a reservoir only charges the cost once,
 * even when placed on water tiles.
 */

#include <stdio.h>
#include <stdlib.h>

#define RESERVOIR_COST 500
#define BUILDING_RESERVOIR 50

#define TERRAIN_WATER 1
#define TERRAIN_LAND 0

/* Simulated game state */
static int player_funds = 0;
static int total_charges = 0;
static int terrain_type[1000];
static int water_reservoir_flags[1000];

/* Reset test state */
void reset_test_state(void)
{
    player_funds = 0;
    total_charges = 0;
    for (int i = 0; i < 1000; i++) {
        terrain_type[i] = TERRAIN_LAND;
        water_reservoir_flags[i] = 0;
    }
}

/* Simulated functions */
int map_terrain_is_water(int grid_offset)
{
    return terrain_type[grid_offset] == TERRAIN_WATER;
}

void set_water_reservoir_flag(int grid_offset)
{
    water_reservoir_flags[grid_offset] = 1;
}

void city_finance_charge(int amount)
{
    player_funds -= amount;
    total_charges++;
}

int try_place_building(int type, int grid_offset)
{
    (void)type;
    (void)grid_offset;
    return 1;  /* Always succeeds for test */
}

/*
 * Place reservoir function - buggy or fixed version
 */
int place_reservoir(int grid_offset, int funds)
{
    if (map_terrain_is_water(grid_offset)) {
        if (funds < RESERVOIR_COST) {
            return 0;
        }
#ifdef BUGGY_VERSION
        /* BUG: Charging here causes double charge */
        city_finance_charge(RESERVOIR_COST);
#endif
        set_water_reservoir_flag(grid_offset);
    }

    if (!try_place_building(BUILDING_RESERVOIR, grid_offset)) {
        return 0;
    }

    city_finance_charge(RESERVOIR_COST);

    return 1;
}

/* Test placing reservoir on land */
int test_land_placement(void)
{
    reset_test_state();
    player_funds = 1000;
    terrain_type[100] = TERRAIN_LAND;

    int result = place_reservoir(100, player_funds);

    if (!result) {
        printf("FAIL: Land placement should succeed\n");
        return 1;
    }

    if (player_funds != 500) {
        printf("FAIL: Land placement charged %d, expected 500\n", 1000 - player_funds);
        return 1;
    }

    if (total_charges != 1) {
        printf("FAIL: Land placement had %d charges, expected 1\n", total_charges);
        return 1;
    }

    printf("PASS: Land placement test\n");
    return 0;
}

/* Test placing reservoir on water */
int test_water_placement(void)
{
    reset_test_state();
    player_funds = 1000;
    terrain_type[200] = TERRAIN_WATER;

    int result = place_reservoir(200, player_funds);

    if (!result) {
        printf("FAIL: Water placement should succeed\n");
        return 1;
    }

    if (player_funds != 500) {
        printf("FAIL: Water placement charged %d, expected 500 (got funds=%d)\n",
               1000 - player_funds, player_funds);
        return 1;
    }

    if (total_charges != 1) {
        printf("FAIL: Water placement had %d charges, expected 1\n", total_charges);
        return 1;
    }

    if (!water_reservoir_flags[200]) {
        printf("FAIL: Water reservoir flag not set\n");
        return 1;
    }

    printf("PASS: Water placement test\n");
    return 0;
}

/* Test insufficient funds */
int test_insufficient_funds(void)
{
    reset_test_state();
    player_funds = 400;  /* Less than RESERVOIR_COST */
    terrain_type[300] = TERRAIN_WATER;

    int result = place_reservoir(300, player_funds);

    if (result) {
        printf("FAIL: Should fail with insufficient funds\n");
        return 1;
    }

    if (player_funds != 400) {
        printf("FAIL: Funds changed despite failed placement\n");
        return 1;
    }

    printf("PASS: Insufficient funds test\n");
    return 0;
}

/* Test exact funds */
int test_exact_funds(void)
{
    reset_test_state();
    player_funds = 500;  /* Exactly RESERVOIR_COST */
    terrain_type[400] = TERRAIN_WATER;

    int result = place_reservoir(400, player_funds);

    if (!result) {
        printf("FAIL: Should succeed with exact funds\n");
        return 1;
    }

    if (player_funds != 0) {
        printf("FAIL: Exact funds test - charged %d, expected 500\n", 500 - player_funds);
        return 1;
    }

    printf("PASS: Exact funds test\n");
    return 0;
}

/* Test multiple placements */
int test_multiple_placements(void)
{
    reset_test_state();
    player_funds = 2000;

    terrain_type[500] = TERRAIN_LAND;
    terrain_type[501] = TERRAIN_WATER;
    terrain_type[502] = TERRAIN_LAND;

    place_reservoir(500, player_funds);
    place_reservoir(501, player_funds);
    place_reservoir(502, player_funds);

    if (total_charges != 3) {
        printf("FAIL: Multiple placements had %d charges, expected 3\n", total_charges);
        return 1;
    }

    if (player_funds != 500) {
        printf("FAIL: Multiple placements - funds=%d, expected 500\n", player_funds);
        return 1;
    }

    printf("PASS: Multiple placements test\n");
    return 0;
}

int main(void)
{
    int failures = 0;

    printf("=== Reservoir Placement Charge Test ===\n\n");

#ifdef BUGGY_VERSION
    printf("Testing BUGGY version (expect double charge on water)\n\n");
#else
    printf("Testing FIXED version (should pass all tests)\n\n");
#endif

    failures += test_land_placement();
    failures += test_water_placement();
    failures += test_insufficient_funds();
    failures += test_exact_funds();
    failures += test_multiple_placements();

    printf("\n=== Results: %d failures ===\n", failures);

    return failures;
}
