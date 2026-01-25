/**
 * Stub implementation for julius-044 building cost refund test
 *
 * This provides mock construction cost calculation for testing refund logic.
 */

#include <stdio.h>
#include <string.h>

/* Terrain types */
#define TERRAIN_NORMAL 0
#define TERRAIN_MARSH 1
#define TERRAIN_HILL 2

/* Building types (subset for testing) */
#define BUILDING_RESERVOIR 1
#define BUILDING_GRANARY 2
#define BUILDING_WAREHOUSE 3

/* Building structure */
typedef struct {
    int type;
    int x, y;
    int construction_cost;  /* Actual cost paid including modifiers */
} building;

/* Base costs for building types */
static int base_costs[] = {
    0,      /* NONE */
    500,    /* RESERVOIR */
    400,    /* GRANARY */
    300     /* WAREHOUSE */
};

/* Get base building cost (without terrain modifiers) */
int get_base_building_cost(int building_type)
{
    if (building_type < 0 || building_type > 3) {
        return 0;
    }
    return base_costs[building_type];
}

/* Calculate construction cost with terrain modifiers */
int calculate_construction_cost(int building_type, int terrain_type)
{
    int base_cost = get_base_building_cost(building_type);

    if (terrain_type == TERRAIN_MARSH) {
        return base_cost + (base_cost / 2);  /* +50% */
    } else if (terrain_type == TERRAIN_HILL) {
        return base_cost + (base_cost / 4);  /* +25% */
    }
    return base_cost;
}

/**
 * Calculate demolition refund
 *
 * BUGGY version: Uses get_base_building_cost(b->type) / 2
 * FIXED version: Uses b->construction_cost / 2
 */
int calculate_demolition_refund(building *b)
{
    /* FIXED: Use the actual construction cost that was paid */
    return b->construction_cost / 2;
}

/* Create a building with proper cost calculation */
void create_building(building *b, int type, int terrain_type)
{
    memset(b, 0, sizeof(building));
    b->type = type;
    b->construction_cost = calculate_construction_cost(type, terrain_type);
}
