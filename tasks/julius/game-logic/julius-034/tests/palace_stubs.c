/**
 * Stub implementation for julius-034 palace upgrade validation test
 *
 * This provides mock city building functions for testing upgrade logic.
 */

#include <stdio.h>

/* Global state for testing */
static int g_has_governor_villa = 0;
static int g_has_governor_palace = 0;

/* Set city state for testing */
void test_set_has_villa(int has_villa)
{
    g_has_governor_villa = has_villa;
}

void test_set_has_palace(int has_palace)
{
    g_has_governor_palace = has_palace;
}

/* Mock city building check functions */
int city_buildings_has_governor_villa(void)
{
    return g_has_governor_villa;
}

int city_buildings_has_governor_palace(void)
{
    return g_has_governor_palace;
}

/**
 * Upgrade validation function
 *
 * BUGGY version: Uses OR (||) - allows upgrade if either doesn't exist
 * FIXED version: Uses AND (&&) - only allows if neither exists
 */
int can_upgrade_governors_house(void)
{
    int has_villa = city_buildings_has_governor_villa();
    int has_palace = city_buildings_has_governor_palace();

    /* FIXED: Use AND - only allow if neither upgraded building exists */
    if (!has_villa && !has_palace) {
        return 1;
    }

    return 0;
}

/* Reset state between tests */
void test_reset_state(void)
{
    g_has_governor_villa = 0;
    g_has_governor_palace = 0;
}
