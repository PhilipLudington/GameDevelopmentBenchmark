/**
 * Stub implementation for julius-040 fireworks timing test
 *
 * This provides mock festival and event scheduling for testing.
 */

#include <stdio.h>

/* Event types */
#define EVENT_FIREWORKS 1

/* Global state for testing */
static int g_fireworks_delay = 100;
static int g_last_scheduled_tick = -1;
static int g_last_event_type = -1;

/* Set fireworks delay for testing */
void test_set_fireworks_delay(int delay)
{
    g_fireworks_delay = delay;
}

/* Get configured fireworks delay */
int get_fireworks_delay(void)
{
    return g_fireworks_delay;
}

/* Mock event scheduler - records what was scheduled */
void event_schedule(int event_type, int tick)
{
    g_last_event_type = event_type;
    g_last_scheduled_tick = tick;
}

/* Get last scheduled tick for testing */
int test_get_last_scheduled_tick(void)
{
    return g_last_scheduled_tick;
}

/* Get last event type for testing */
int test_get_last_event_type(void)
{
    return g_last_event_type;
}

/* Reset test state */
void test_reset_state(void)
{
    g_fireworks_delay = 100;
    g_last_scheduled_tick = -1;
    g_last_event_type = -1;
}

/**
 * Schedule fireworks for a festival
 *
 * BUGGY version: festival_start_tick - fireworks_delay
 * FIXED version: festival_start_tick + fireworks_delay
 */
void schedule_festival_fireworks(int festival_start_tick)
{
    int fireworks_delay = get_fireworks_delay();

    /* FIXED: Add delay to fire AFTER festival starts */
    int fireworks_tick = festival_start_tick + fireworks_delay;

    event_schedule(EVENT_FIREWORKS, fireworks_tick);
}

/* Calculate when fireworks should ideally fire */
int calculate_expected_fireworks_tick(int festival_start, int delay)
{
    return festival_start + delay;  /* Correct calculation */
}
