/**
 * Stub implementation for julius-035 popularity decay test
 *
 * This provides mock sentiment calculation functions for testing.
 */

#include <stdio.h>

/* Scale factor used in calculations */
#define SCALE_FACTOR 100

/* Global state for testing */
static int g_difficulty_modifier = 1;
static int g_time_factor = 1;

/* Set test parameters */
void test_set_difficulty_modifier(int modifier)
{
    g_difficulty_modifier = modifier;
}

void test_set_time_factor(int factor)
{
    g_time_factor = factor;
}

/* Mock functions */
int get_difficulty_modifier(void)
{
    return g_difficulty_modifier;
}

int get_time_factor(void)
{
    return g_time_factor;
}

/**
 * Calculate popularity decay
 *
 * BUGGY version: base_decay_rate / SCALE_FACTOR * modifier * time_factor
 *   - Division first causes truncation for small values
 *
 * FIXED version: (base_decay_rate * modifier * time_factor) / SCALE_FACTOR
 *   - Multiplication first preserves precision
 */
int calculate_popularity_decay(int current_popularity, int base_decay_rate)
{
    int modifier = get_difficulty_modifier();
    int time_factor = get_time_factor();

    /* FIXED: Multiply first, then divide to preserve precision */
    int decay = (base_decay_rate * modifier * time_factor) / SCALE_FACTOR;

    return current_popularity - decay;
}

/* Calculate raw decay value for testing */
int calculate_raw_decay(int base_decay_rate)
{
    int modifier = get_difficulty_modifier();
    int time_factor = get_time_factor();

    /* FIXED: Multiply first */
    return (base_decay_rate * modifier * time_factor) / SCALE_FACTOR;
}

/* Reset test state */
void test_reset_state(void)
{
    g_difficulty_modifier = 1;
    g_time_factor = 1;
}
