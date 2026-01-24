/**
 * Stub implementation for julius-009 null pointer test
 *
 * This provides mock building lookup functions for testing.
 * The actual Julius source is verified via static analysis.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Maximum number of buildings in the game */
#define MAX_BUILDINGS 4000

/* Simplified building structure */
typedef struct {
    int id;
    int type;
    int fire_risk;
    int in_use;
} building;

/* Static building storage */
static building all_buildings[MAX_BUILDINGS];
static int buildings_initialized = 0;

/* Initialize building storage */
static void init_buildings(void)
{
    if (buildings_initialized) return;
    memset(all_buildings, 0, sizeof(all_buildings));
    /* Mark most buildings as not in use (simulating deleted buildings) */
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        all_buildings[i].id = i;
        all_buildings[i].in_use = 0;
        all_buildings[i].fire_risk = 0;
    }
    buildings_initialized = 1;
}

/* Mock building_get - returns NULL for invalid/deleted buildings */
building *building_get(int building_id)
{
    init_buildings();

    if (building_id <= 0 || building_id >= MAX_BUILDINGS) {
        return NULL;
    }

    building *b = &all_buildings[building_id];
    if (!b->in_use) {
        return NULL; /* Building was deleted */
    }

    return b;
}

/**
 * BUGGY version: No NULL check - will crash if building doesn't exist
 *
 * int building_get_fire_risk_unsafe(int building_id)
 * {
 *     building *b = building_get(building_id);
 *     return b->fire_risk;  // CRASH if b is NULL
 * }
 *
 * FIXED version: Check for NULL before dereferencing
 */
int building_get_fire_risk_unsafe(int building_id)
{
    building *b = building_get(building_id);

    /* FIXED: Check for NULL before dereferencing */
    if (!b) {
        return 0; /* Return safe default for invalid building */
    }

    return b->fire_risk;
}
