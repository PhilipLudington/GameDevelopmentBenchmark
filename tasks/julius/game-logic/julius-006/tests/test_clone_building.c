/**
 * Test for clone building validation bug (julius-006)
 *
 * This test compiles against the actual Julius scenario/building.c and verifies
 * that scenario_building_allowed() properly checks individual building types.
 *
 * Test validation:
 * - On FIXED code: Individual building types (e.g., BUILDING_SMALL_TEMPLE_MARS)
 *   are included in their menu group's switch case
 * - On BUGGY code: Individual building types are removed from switch, falling
 *   through to default which returns 1 (always allowed)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/*
 * Header guards to prevent Julius headers from being included
 */
#define SCENARIO_BUILDING_H
#define BUILDING_TYPE_H
#define SCENARIO_DATA_H

/*
 * Complete building_type enum from Julius building/type.h
 */
typedef enum {
    BUILDING_NONE = 0,
    BUILDING_MENU_FARMS = 2,
    BUILDING_MENU_RAW_MATERIALS = 3,
    BUILDING_MENU_WORKSHOPS = 4,
    BUILDING_ROAD = 5,
    BUILDING_WALL = 6,
    BUILDING_DRAGGABLE_RESERVOIR = 7,
    BUILDING_AQUEDUCT = 8,
    BUILDING_CLEAR_LAND = 9,
    BUILDING_HOUSE_VACANT_LOT = 10,
    BUILDING_HOUSE_SMALL_TENT = 10,
    BUILDING_HOUSE_LARGE_TENT = 11,
    BUILDING_HOUSE_SMALL_SHACK = 12,
    BUILDING_HOUSE_LARGE_SHACK = 13,
    BUILDING_HOUSE_SMALL_HOVEL = 14,
    BUILDING_HOUSE_LARGE_HOVEL = 15,
    BUILDING_HOUSE_SMALL_CASA = 16,
    BUILDING_HOUSE_LARGE_CASA = 17,
    BUILDING_HOUSE_SMALL_INSULA = 18,
    BUILDING_HOUSE_MEDIUM_INSULA = 19,
    BUILDING_HOUSE_LARGE_INSULA = 20,
    BUILDING_HOUSE_GRAND_INSULA = 21,
    BUILDING_HOUSE_SMALL_VILLA = 22,
    BUILDING_HOUSE_MEDIUM_VILLA = 23,
    BUILDING_HOUSE_LARGE_VILLA = 24,
    BUILDING_HOUSE_GRAND_VILLA = 25,
    BUILDING_HOUSE_SMALL_PALACE = 26,
    BUILDING_HOUSE_MEDIUM_PALACE = 27,
    BUILDING_HOUSE_LARGE_PALACE = 28,
    BUILDING_HOUSE_LUXURY_PALACE = 29,
    BUILDING_AMPHITHEATER = 30,
    BUILDING_THEATER = 31,
    BUILDING_HIPPODROME = 32,
    BUILDING_COLOSSEUM = 33,
    BUILDING_GLADIATOR_SCHOOL = 34,
    BUILDING_LION_HOUSE = 35,
    BUILDING_ACTOR_COLONY = 36,
    BUILDING_CHARIOT_MAKER = 37,
    BUILDING_PLAZA = 38,
    BUILDING_GARDENS = 39,
    BUILDING_FORT_LEGIONARIES = 40,
    BUILDING_SMALL_STATUE = 41,
    BUILDING_MEDIUM_STATUE = 42,
    BUILDING_LARGE_STATUE = 43,
    BUILDING_FORT_JAVELIN = 44,
    BUILDING_FORT_MOUNTED = 45,
    BUILDING_DOCTOR = 46,
    BUILDING_HOSPITAL = 47,
    BUILDING_BATHHOUSE = 48,
    BUILDING_BARBER = 49,
    BUILDING_DISTRIBUTION_CENTER_UNUSED = 50,
    BUILDING_SCHOOL = 51,
    BUILDING_ACADEMY = 52,
    BUILDING_LIBRARY = 53,
    BUILDING_FORT_GROUND = 54,
    BUILDING_PREFECTURE = 55,
    BUILDING_TRIUMPHAL_ARCH = 56,
    BUILDING_FORT = 57,
    BUILDING_GATEHOUSE = 58,
    BUILDING_TOWER = 59,
    BUILDING_SMALL_TEMPLE_CERES = 60,
    BUILDING_SMALL_TEMPLE_NEPTUNE = 61,
    BUILDING_SMALL_TEMPLE_MERCURY = 62,
    BUILDING_SMALL_TEMPLE_MARS = 63,
    BUILDING_SMALL_TEMPLE_VENUS = 64,
    BUILDING_LARGE_TEMPLE_CERES = 65,
    BUILDING_LARGE_TEMPLE_NEPTUNE = 66,
    BUILDING_LARGE_TEMPLE_MERCURY = 67,
    BUILDING_LARGE_TEMPLE_MARS = 68,
    BUILDING_LARGE_TEMPLE_VENUS = 69,
    BUILDING_MARKET = 70,
    BUILDING_GRANARY = 71,
    BUILDING_WAREHOUSE = 72,
    BUILDING_WAREHOUSE_SPACE = 73,
    BUILDING_SHIPYARD = 74,
    BUILDING_DOCK = 75,
    BUILDING_WHARF = 76,
    BUILDING_GOVERNORS_HOUSE = 77,
    BUILDING_GOVERNORS_VILLA = 78,
    BUILDING_GOVERNORS_PALACE = 79,
    BUILDING_MISSION_POST = 80,
    BUILDING_ENGINEERS_POST = 81,
    BUILDING_LOW_BRIDGE = 82,
    BUILDING_SHIP_BRIDGE = 83,
    BUILDING_SENATE = 84,
    BUILDING_SENATE_UPGRADED = 85,
    BUILDING_FORUM = 86,
    BUILDING_FORUM_UPGRADED = 87,
    BUILDING_NATIVE_HUT = 88,
    BUILDING_NATIVE_MEETING = 89,
    BUILDING_RESERVOIR = 90,
    BUILDING_FOUNTAIN = 91,
    BUILDING_WELL = 92,
    BUILDING_NATIVE_CROPS = 93,
    BUILDING_MILITARY_ACADEMY = 94,
    BUILDING_BARRACKS = 95,
    BUILDING_MENU_SMALL_TEMPLES = 96,
    BUILDING_MENU_LARGE_TEMPLES = 97,
    BUILDING_ORACLE = 98,
    BUILDING_BURNING_RUIN = 99,
    BUILDING_WHEAT_FARM = 100,
    BUILDING_VEGETABLE_FARM = 101,
    BUILDING_FRUIT_FARM = 102,
    BUILDING_OLIVE_FARM = 103,
    BUILDING_VINES_FARM = 104,
    BUILDING_PIG_FARM = 105,
    BUILDING_MARBLE_QUARRY = 106,
    BUILDING_IRON_MINE = 107,
    BUILDING_TIMBER_YARD = 108,
    BUILDING_CLAY_PIT = 109,
    BUILDING_WINE_WORKSHOP = 110,
    BUILDING_OIL_WORKSHOP = 111,
    BUILDING_WEAPONS_WORKSHOP = 112,
    BUILDING_FURNITURE_WORKSHOP = 113,
    BUILDING_POTTERY_WORKSHOP = 114,
    BUILDING_TYPE_MAX = 115
} building_type;

/*
 * Allowed building enum from Julius scenario/data.h
 */
typedef enum {
    ALLOWED_BUILDING_ROAD = 0,
    ALLOWED_BUILDING_AQUEDUCT = 1,
    ALLOWED_BUILDING_WELL = 2,
    ALLOWED_BUILDING_BARBER = 3,
    ALLOWED_BUILDING_BATHHOUSE = 4,
    ALLOWED_BUILDING_DOCTOR = 5,
    ALLOWED_BUILDING_HOSPITAL = 6,
    ALLOWED_BUILDING_SMALL_TEMPLES = 7,
    ALLOWED_BUILDING_LARGE_TEMPLES = 8,
    ALLOWED_BUILDING_ORACLE = 9,
    ALLOWED_BUILDING_SCHOOL = 10,
    ALLOWED_BUILDING_ACADEMY = 11,
    ALLOWED_BUILDING_LIBRARY = 12,
    ALLOWED_BUILDING_THEATER = 13,
    ALLOWED_BUILDING_AMPHITHEATER = 14,
    ALLOWED_BUILDING_COLOSSEUM = 15,
    ALLOWED_BUILDING_HIPPODROME = 16,
    ALLOWED_BUILDING_GLADIATOR_SCHOOL = 17,
    ALLOWED_BUILDING_LION_HOUSE = 18,
    ALLOWED_BUILDING_ACTOR_COLONY = 19,
    ALLOWED_BUILDING_CHARIOT_MAKER = 20,
    ALLOWED_BUILDING_FORUM = 21,
    ALLOWED_BUILDING_SENATE = 22,
    ALLOWED_BUILDING_GOVERNOR_HOME = 23,
    ALLOWED_BUILDING_STATUES = 24,
    ALLOWED_BUILDING_GARDENS = 25,
    ALLOWED_BUILDING_PLAZA = 26,
    ALLOWED_BUILDING_ENGINEERS_POST = 27,
    ALLOWED_BUILDING_MISSION_POST = 28,
    ALLOWED_BUILDING_WHARF = 29,
    ALLOWED_BUILDING_DOCK = 30,
    ALLOWED_BUILDING_WALL = 31,
    ALLOWED_BUILDING_TOWER = 32,
    ALLOWED_BUILDING_GATEHOUSE = 33,
    ALLOWED_BUILDING_PREFECTURE = 34,
    ALLOWED_BUILDING_FORT = 35,
    ALLOWED_BUILDING_MILITARY_ACADEMY = 36,
    ALLOWED_BUILDING_BARRACKS = 37,
    ALLOWED_BUILDING_DISTRIBUTION_CENTER = 38,
    ALLOWED_BUILDING_FARMS = 39,
    ALLOWED_BUILDING_RAW_MATERIALS = 40,
    ALLOWED_BUILDING_WORKSHOPS = 41,
    ALLOWED_BUILDING_MARKET = 42,
    ALLOWED_BUILDING_GRANARY = 43,
    ALLOWED_BUILDING_WAREHOUSE = 44,
    ALLOWED_BUILDING_BRIDGE = 45,
    ALLOWED_BUILDING_MAX = 46
} allowed_building;

/*
 * Scenario data structure (simplified from Julius scenario/data.h)
 */
typedef struct {
    struct {
        int hut;
        int meeting;
        int crops;
    } native_images;
    int allowed_buildings[ALLOWED_BUILDING_MAX];
} scenario_data_t;

/*
 * Global scenario - this is what building.c accesses
 */
scenario_data_t scenario;

/*
 * Now include the actual building.c implementation.
 * The header guards above prevent Julius headers from processing.
 */
#include "scenario/building.c"

/*
 * Helper: Reset scenario to all buildings allowed
 */
static void reset_scenario_all_allowed(void)
{
    memset(&scenario, 0, sizeof(scenario));
    for (int i = 0; i < ALLOWED_BUILDING_MAX; i++) {
        scenario.allowed_buildings[i] = 1;
    }
}

/*
 * Test 1: Individual temple type respects menu restriction
 *
 * When small temples menu is DISABLED, calling scenario_building_allowed()
 * with an individual temple type should return 0.
 *
 * - FIXED: BUILDING_SMALL_TEMPLE_MARS matches case → returns allowed_buildings[SMALL_TEMPLES] = 0
 * - BUGGY: BUILDING_SMALL_TEMPLE_MARS falls to default → returns 1
 */
static int test_temple_restriction(void)
{
    printf("Test: Individual temple respects menu restriction...\n");

    reset_scenario_all_allowed();
    scenario.allowed_buildings[ALLOWED_BUILDING_SMALL_TEMPLES] = 0;

    int menu_result = scenario_building_allowed(BUILDING_MENU_SMALL_TEMPLES);
    printf("  BUILDING_MENU_SMALL_TEMPLES: %s\n", menu_result ? "ALLOWED" : "DISALLOWED");

    int mars_result = scenario_building_allowed(BUILDING_SMALL_TEMPLE_MARS);
    printf("  BUILDING_SMALL_TEMPLE_MARS: %s\n", mars_result ? "ALLOWED" : "DISALLOWED");

    if (mars_result != 0) {
        printf("  FAIL: Temple of Mars should be DISALLOWED when small temples are disabled\n");
        printf("  This is the clone building bug - individual types not checked!\n");
        return 0;
    }

    printf("  PASS: Temple correctly disallowed\n");
    return 1;
}

/*
 * Test 2: All individual temple types respect restriction
 */
static int test_all_temples_restricted(void)
{
    printf("Test: All temple types respect menu restriction...\n");

    reset_scenario_all_allowed();
    scenario.allowed_buildings[ALLOWED_BUILDING_SMALL_TEMPLES] = 0;

    building_type temples[] = {
        BUILDING_SMALL_TEMPLE_CERES,
        BUILDING_SMALL_TEMPLE_NEPTUNE,
        BUILDING_SMALL_TEMPLE_MERCURY,
        BUILDING_SMALL_TEMPLE_MARS,
        BUILDING_SMALL_TEMPLE_VENUS
    };

    for (size_t i = 0; i < sizeof(temples) / sizeof(temples[0]); i++) {
        int result = scenario_building_allowed(temples[i]);
        if (result != 0) {
            printf("  FAIL: Temple type %d should be DISALLOWED\n", temples[i]);
            return 0;
        }
    }

    printf("  PASS: All 5 small temples correctly restricted\n");
    return 1;
}

/*
 * Test 3: Fort types respect menu restriction
 */
static int test_fort_types_restricted(void)
{
    printf("Test: Fort types respect menu restriction...\n");

    reset_scenario_all_allowed();
    scenario.allowed_buildings[ALLOWED_BUILDING_FORT] = 0;

    building_type forts[] = {
        BUILDING_FORT_LEGIONARIES,
        BUILDING_FORT_JAVELIN,
        BUILDING_FORT_MOUNTED
    };

    for (size_t i = 0; i < sizeof(forts) / sizeof(forts[0]); i++) {
        int result = scenario_building_allowed(forts[i]);
        if (result != 0) {
            printf("  FAIL: Fort type %d should be DISALLOWED\n", forts[i]);
            return 0;
        }
    }

    printf("  PASS: All 3 fort types correctly restricted\n");
    return 1;
}

/*
 * Test 4: Farm types respect menu restriction
 */
static int test_farm_types_restricted(void)
{
    printf("Test: Farm types respect menu restriction...\n");

    reset_scenario_all_allowed();
    scenario.allowed_buildings[ALLOWED_BUILDING_FARMS] = 0;

    building_type farms[] = {
        BUILDING_WHEAT_FARM,
        BUILDING_VEGETABLE_FARM,
        BUILDING_FRUIT_FARM,
        BUILDING_OLIVE_FARM,
        BUILDING_VINES_FARM,
        BUILDING_PIG_FARM
    };

    for (size_t i = 0; i < sizeof(farms) / sizeof(farms[0]); i++) {
        int result = scenario_building_allowed(farms[i]);
        if (result != 0) {
            printf("  FAIL: Farm type %d should be DISALLOWED\n", farms[i]);
            return 0;
        }
    }

    printf("  PASS: All 6 farm types correctly restricted\n");
    return 1;
}

/*
 * Test 5: Workshop types respect menu restriction
 */
static int test_workshop_types_restricted(void)
{
    printf("Test: Workshop types respect menu restriction...\n");

    reset_scenario_all_allowed();
    scenario.allowed_buildings[ALLOWED_BUILDING_WORKSHOPS] = 0;

    building_type workshops[] = {
        BUILDING_WINE_WORKSHOP,
        BUILDING_OIL_WORKSHOP,
        BUILDING_WEAPONS_WORKSHOP,
        BUILDING_FURNITURE_WORKSHOP,
        BUILDING_POTTERY_WORKSHOP
    };

    for (size_t i = 0; i < sizeof(workshops) / sizeof(workshops[0]); i++) {
        int result = scenario_building_allowed(workshops[i]);
        if (result != 0) {
            printf("  FAIL: Workshop type %d should be DISALLOWED\n", workshops[i]);
            return 0;
        }
    }

    printf("  PASS: All 5 workshop types correctly restricted\n");
    return 1;
}

/*
 * Test 6: Allowed buildings still work
 */
static int test_allowed_buildings_work(void)
{
    printf("Test: Allowed buildings are still permitted...\n");

    reset_scenario_all_allowed();
    scenario.allowed_buildings[ALLOWED_BUILDING_FARMS] = 0;

    int temple = scenario_building_allowed(BUILDING_SMALL_TEMPLE_MARS);
    if (temple != 1) {
        printf("  FAIL: Temple should be ALLOWED\n");
        return 0;
    }

    int fort = scenario_building_allowed(BUILDING_FORT_MOUNTED);
    if (fort != 1) {
        printf("  FAIL: Fort should be ALLOWED\n");
        return 0;
    }

    printf("  PASS: Other building types still allowed\n");
    return 1;
}

int main(void)
{
    int passed = 0;
    int total = 6;

    printf("=== Clone Building Validation Test Suite ===\n");
    printf("Testing scenario_building_allowed() from scenario/building.c\n");
    printf("Bug: Individual building types bypass menu restrictions\n\n");

    if (test_temple_restriction()) passed++;
    printf("\n");

    if (test_all_temples_restricted()) passed++;
    printf("\n");

    if (test_fort_types_restricted()) passed++;
    printf("\n");

    if (test_farm_types_restricted()) passed++;
    printf("\n");

    if (test_workshop_types_restricted()) passed++;
    printf("\n");

    if (test_allowed_buildings_work()) passed++;

    printf("\n=== Test Results ===\n");
    printf("Results: %d/%d tests passed\n", passed, total);

    return (passed == total) ? 0 : 1;
}
