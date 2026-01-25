/**
 * Test for fireworks timing offset error (julius-040)
 *
 * This test verifies that fireworks are scheduled AFTER the festival
 * starts, not before.
 *
 * Test validation:
 * - On FIXED code: Fireworks tick > festival start tick
 * - On BUGGY code: Fireworks tick < festival start tick
 */

#include <stdio.h>
#include <stdlib.h>

#define EVENT_FIREWORKS 1

/* External functions from stubs */
extern void test_set_fireworks_delay(int delay);
extern void schedule_festival_fireworks(int festival_start_tick);
extern int test_get_last_scheduled_tick(void);
extern int test_get_last_event_type(void);
extern void test_reset_state(void);
extern int calculate_expected_fireworks_tick(int festival_start, int delay);

/**
 * Test 1: Basic timing - fireworks should be after festival start
 *
 * Festival at tick 1000, delay 100
 * Expected: fireworks at 1100
 * Buggy: fireworks at 900
 */
static int test_basic_timing(void)
{
    printf("Test: Basic fireworks timing...\n");

    test_reset_state();
    test_set_fireworks_delay(100);

    int festival_start = 1000;
    schedule_festival_fireworks(festival_start);

    int fireworks_tick = test_get_last_scheduled_tick();
    printf("  Festival starts at tick: %d\n", festival_start);
    printf("  Fireworks delay: 100\n");
    printf("  Fireworks scheduled at tick: %d\n", fireworks_tick);

    int expected = 1100;  /* 1000 + 100 */

    if (fireworks_tick != expected) {
        printf("  FAIL: Expected tick %d, got %d\n", expected, fireworks_tick);
        if (fireworks_tick < festival_start) {
            printf("  BUG: Fireworks scheduled BEFORE festival starts!\n");
            printf("  This is wrong - subtraction used instead of addition\n");
        }
        return 0;
    }

    printf("  PASS: Fireworks correctly scheduled after festival\n");
    return 1;
}

/**
 * Test 2: Verify fireworks tick is greater than festival start
 */
static int test_fireworks_after_start(void)
{
    printf("Test: Fireworks tick > festival start tick...\n");

    test_reset_state();
    test_set_fireworks_delay(50);

    int festival_start = 500;
    schedule_festival_fireworks(festival_start);

    int fireworks_tick = test_get_last_scheduled_tick();

    printf("  Festival: %d, Fireworks: %d\n", festival_start, fireworks_tick);

    if (fireworks_tick <= festival_start) {
        printf("  FAIL: Fireworks should be scheduled AFTER festival start\n");
        return 0;
    }

    printf("  PASS: Fireworks after festival start\n");
    return 1;
}

/**
 * Test 3: Zero delay edge case
 *
 * With zero delay, fireworks should fire at festival start
 */
static int test_zero_delay(void)
{
    printf("Test: Zero delay (fireworks at festival start)...\n");

    test_reset_state();
    test_set_fireworks_delay(0);

    int festival_start = 2000;
    schedule_festival_fireworks(festival_start);

    int fireworks_tick = test_get_last_scheduled_tick();

    printf("  Festival: %d, Delay: 0, Fireworks: %d\n",
           festival_start, fireworks_tick);

    if (fireworks_tick != festival_start) {
        printf("  FAIL: With zero delay, fireworks should be at festival start\n");
        return 0;
    }

    printf("  PASS: Zero delay handled correctly\n");
    return 1;
}

/**
 * Test 4: Large delay
 */
static int test_large_delay(void)
{
    printf("Test: Large fireworks delay...\n");

    test_reset_state();
    test_set_fireworks_delay(5000);

    int festival_start = 10000;
    schedule_festival_fireworks(festival_start);

    int fireworks_tick = test_get_last_scheduled_tick();
    int expected = 15000;

    printf("  Festival: %d, Delay: 5000, Fireworks: %d\n",
           festival_start, fireworks_tick);

    if (fireworks_tick != expected) {
        printf("  FAIL: Expected %d, got %d\n", expected, fireworks_tick);
        return 0;
    }

    printf("  PASS: Large delay calculated correctly\n");
    return 1;
}

/**
 * Test 5: Event type is correct
 */
static int test_event_type(void)
{
    printf("Test: Event type is FIREWORKS...\n");

    test_reset_state();
    schedule_festival_fireworks(1000);

    int event_type = test_get_last_event_type();

    if (event_type != EVENT_FIREWORKS) {
        printf("  FAIL: Wrong event type scheduled\n");
        return 0;
    }

    printf("  PASS: Correct event type\n");
    return 1;
}

/**
 * Test 6: Early festival (low tick number)
 *
 * With buggy code, this could produce negative tick!
 */
static int test_early_festival(void)
{
    printf("Test: Early festival (low tick number)...\n");

    test_reset_state();
    test_set_fireworks_delay(100);

    int festival_start = 50;  /* Very early */
    schedule_festival_fireworks(festival_start);

    int fireworks_tick = test_get_last_scheduled_tick();

    printf("  Festival: %d, Delay: 100, Fireworks: %d\n",
           festival_start, fireworks_tick);

    if (fireworks_tick < 0) {
        printf("  FAIL: Negative tick value! Bug caused underflow\n");
        return 0;
    }

    if (fireworks_tick < festival_start) {
        printf("  FAIL: Fireworks before festival (subtraction bug)\n");
        return 0;
    }

    int expected = 150;  /* 50 + 100 */
    if (fireworks_tick != expected) {
        printf("  FAIL: Expected %d, got %d\n", expected, fireworks_tick);
        return 0;
    }

    printf("  PASS: Early festival handled correctly\n");
    return 1;
}

int main(void)
{
    int passed = 0;
    int total = 6;

    printf("=== Fireworks Timing Test Suite ===\n");
    printf("Testing festival fireworks scheduling\n");
    printf("Bug: Subtraction instead of addition for timing offset\n\n");

    if (test_basic_timing()) passed++;
    printf("\n");

    if (test_fireworks_after_start()) passed++;
    printf("\n");

    if (test_zero_delay()) passed++;
    printf("\n");

    if (test_large_delay()) passed++;
    printf("\n");

    if (test_event_type()) passed++;
    printf("\n");

    if (test_early_festival()) passed++;

    printf("\n=== Test Results ===\n");
    printf("Results: %d/%d tests passed\n", passed, total);

    return (passed == total) ? 0 : 1;
}
