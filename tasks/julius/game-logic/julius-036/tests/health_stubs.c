/**
 * Stub implementation for julius-036 plague spread rate test
 *
 * This provides mock plague spread calculation for testing overflow handling.
 */

#include <stdio.h>
#include <limits.h>

/* Constants */
#define MAX_SPREAD_RATE 10000000
#define MIN_SPREAD_RATE 1

/**
 * Calculate plague spread rate
 *
 * BUGGY version: Direct multiplication that can overflow
 * FIXED version: Check for overflow before multiplying
 */
int calculate_plague_spread_rate(int population_density, int base_spread_factor)
{
    /* FIXED: Check for overflow before multiplying */
    if (population_density > 0 && base_spread_factor > 0) {
        if (population_density > MAX_SPREAD_RATE / base_spread_factor) {
            return MAX_SPREAD_RATE;
        }
    }

    int spread_rate = population_density * base_spread_factor;

    if (spread_rate > MAX_SPREAD_RATE) {
        spread_rate = MAX_SPREAD_RATE;
    }
    if (spread_rate < MIN_SPREAD_RATE) {
        spread_rate = MIN_SPREAD_RATE;
    }

    return spread_rate;
}

/* Get constants for testing */
int get_max_spread_rate(void)
{
    return MAX_SPREAD_RATE;
}

int get_min_spread_rate(void)
{
    return MIN_SPREAD_RATE;
}
