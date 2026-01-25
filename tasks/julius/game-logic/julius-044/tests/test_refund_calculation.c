/**
 * Test for building cost refund error (julius-044)
 *
 * This test verifies that demolition refund is based on the actual
 * construction cost paid, not just the base cost.
 *
 * Test validation:
 * - On FIXED code: Refund is 50% of actual cost (including terrain modifiers)
 * - On BUGGY code: Refund is 50% of base cost (ignoring modifiers)
 */

#include <stdio.h>
#include <stdlib.h>

/* Terrain types */
#define TERRAIN_NORMAL 0
#define TERRAIN_MARSH 1
#define TERRAIN_HILL 2

/* Building types */
#define BUILDING_RESERVOIR 1
#define BUILDING_GRANARY 2

/* Building structure */
typedef struct {
    int type;
    int x, y;
    int construction_cost;
} building;

/* External functions from stubs */
extern int get_base_building_cost(int building_type);
extern int calculate_construction_cost(int building_type, int terrain_type);
extern int calculate_demolition_refund(building *b);
extern void create_building(building *b, int type, int terrain_type);

/**
 * Test 1: Normal terrain - refund should match expected
 *
 * Base cost: 500, Actual cost: 500, Refund: 250
 */
static int test_normal_terrain_refund(void)
{
    printf("Test: Refund on normal terrain...\n");

    building b;
    create_building(&b, BUILDING_RESERVOIR, TERRAIN_NORMAL);

    int base = get_base_building_cost(BUILDING_RESERVOIR);
    int actual_cost = b.construction_cost;
    int refund = calculate_demolition_refund(&b);

    printf("  Base cost: %d\n", base);
    printf("  Actual cost paid: %d\n", actual_cost);
    printf("  Refund: %d\n", refund);

    int expected_refund = actual_cost / 2;
    if (refund != expected_refund) {
        printf("  FAIL: Expected refund %d, got %d\n", expected_refund, refund);
        return 0;
    }

    printf("  PASS: Normal terrain refund correct\n");
    return 1;
}

/**
 * Test 2: Marsh terrain - THE KEY TEST
 *
 * Base cost: 500, Marsh modifier: +50%, Actual: 750
 * Expected refund: 375 (50% of 750)
 * Buggy refund: 250 (50% of 500 base)
 */
static int test_marsh_terrain_refund(void)
{
    printf("Test: Refund on marsh terrain (with +50%% modifier)...\n");

    building b;
    create_building(&b, BUILDING_RESERVOIR, TERRAIN_MARSH);

    int base = get_base_building_cost(BUILDING_RESERVOIR);
    int actual_cost = b.construction_cost;
    int refund = calculate_demolition_refund(&b);

    printf("  Base cost: %d\n", base);
    printf("  Marsh cost (+50%%): %d\n", actual_cost);
    printf("  Refund: %d\n", refund);

    int expected_refund = actual_cost / 2;  /* 750 / 2 = 375 */
    int buggy_refund = base / 2;            /* 500 / 2 = 250 */

    printf("  Expected (50%% of actual): %d\n", expected_refund);
    printf("  Buggy (50%% of base): %d\n", buggy_refund);

    if (refund == buggy_refund && refund != expected_refund) {
        printf("  FAIL: Refund uses base cost instead of actual cost!\n");
        printf("  This is the bug - player loses money on marsh construction\n");
        return 0;
    }

    if (refund != expected_refund) {
        printf("  FAIL: Expected refund %d, got %d\n", expected_refund, refund);
        return 0;
    }

    printf("  PASS: Marsh terrain refund correctly includes modifier\n");
    return 1;
}

/**
 * Test 3: Hill terrain
 *
 * Base cost: 500, Hill modifier: +25%, Actual: 625
 * Expected refund: 312 (50% of 625)
 */
static int test_hill_terrain_refund(void)
{
    printf("Test: Refund on hill terrain (with +25%% modifier)...\n");

    building b;
    create_building(&b, BUILDING_RESERVOIR, TERRAIN_HILL);

    int actual_cost = b.construction_cost;
    int refund = calculate_demolition_refund(&b);

    printf("  Hill cost (+25%%): %d\n", actual_cost);
    printf("  Refund: %d\n", refund);

    int expected_refund = actual_cost / 2;
    if (refund != expected_refund) {
        printf("  FAIL: Expected refund %d, got %d\n", expected_refund, refund);
        return 0;
    }

    printf("  PASS: Hill terrain refund correct\n");
    return 1;
}

/**
 * Test 4: Different building type with marsh
 */
static int test_granary_marsh_refund(void)
{
    printf("Test: Granary on marsh terrain...\n");

    building b;
    create_building(&b, BUILDING_GRANARY, TERRAIN_MARSH);

    int base = get_base_building_cost(BUILDING_GRANARY);  /* 400 */
    int actual_cost = b.construction_cost;  /* 400 + 200 = 600 */
    int refund = calculate_demolition_refund(&b);

    printf("  Granary base: %d, marsh cost: %d, refund: %d\n",
           base, actual_cost, refund);

    int expected = actual_cost / 2;  /* 300 */
    if (refund != expected) {
        printf("  FAIL: Expected %d, got %d\n", expected, refund);
        return 0;
    }

    printf("  PASS: Granary marsh refund correct\n");
    return 1;
}

/**
 * Test 5: Verify refund is exactly half of construction cost
 */
static int test_refund_ratio(void)
{
    printf("Test: Refund is exactly 50%% of construction cost...\n");

    int terrains[] = {TERRAIN_NORMAL, TERRAIN_MARSH, TERRAIN_HILL};
    int num_terrains = 3;
    int all_correct = 1;

    for (int i = 0; i < num_terrains; i++) {
        building b;
        create_building(&b, BUILDING_RESERVOIR, terrains[i]);

        int cost = b.construction_cost;
        int refund = calculate_demolition_refund(&b);
        int expected = cost / 2;

        printf("  Terrain %d: cost=%d, refund=%d, expected=%d %s\n",
               terrains[i], cost, refund, expected,
               refund == expected ? "OK" : "FAIL");

        if (refund != expected) {
            all_correct = 0;
        }
    }

    if (!all_correct) {
        printf("  FAIL: Refund ratio incorrect for some terrains\n");
        return 0;
    }

    printf("  PASS: Refund is consistently 50%% of actual cost\n");
    return 1;
}

int main(void)
{
    int passed = 0;
    int total = 5;

    printf("=== Building Cost Refund Test Suite ===\n");
    printf("Testing that refund uses actual construction cost\n");
    printf("Bug: Refund ignores terrain modifiers, uses base cost\n\n");

    if (test_normal_terrain_refund()) passed++;
    printf("\n");

    if (test_marsh_terrain_refund()) passed++;
    printf("\n");

    if (test_hill_terrain_refund()) passed++;
    printf("\n");

    if (test_granary_marsh_refund()) passed++;
    printf("\n");

    if (test_refund_ratio()) passed++;

    printf("\n=== Test Results ===\n");
    printf("Results: %d/%d tests passed\n", passed, total);

    return (passed == total) ? 0 : 1;
}
