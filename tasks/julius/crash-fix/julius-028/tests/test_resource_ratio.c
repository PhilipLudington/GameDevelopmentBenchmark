/*
 * Test for julius-028: Division by zero in resource ratio calculation
 *
 * This test verifies that calculating resource consumption percentage
 * with zero max capacity does not cause a division by zero crash.
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>

#define MAX_RESOURCE_TYPES 20

#define RESOURCE_WHEAT 0
#define RESOURCE_TIMBER 1
#define RESOURCE_IRON 2
#define RESOURCE_WINE 3

/* For catching SIGFPE */
static jmp_buf jump_buffer;
static volatile int sigfpe_caught = 0;

void sigfpe_handler(int sig)
{
    (void)sig;
    sigfpe_caught = 1;
    longjmp(jump_buffer, 1);
}

/* Simulated resource data */
struct {
    int current_consumption;
    int max_consumption;
} resource_data[MAX_RESOURCE_TYPES];

/*
 * Get resource consumption percent - buggy or fixed version
 */
int get_resource_consumption_percent(int resource_type)
{
    int current = resource_data[resource_type].current_consumption;
    int max_capacity = resource_data[resource_type].max_consumption;

#ifdef BUGGY_VERSION
    /* BUG: No check for zero max_capacity - will crash */
    return (current * 100) / max_capacity;
#else
    /* FIXED: Check for zero max_capacity */
    if (max_capacity == 0) {
        return 0;
    }
    return (current * 100) / max_capacity;
#endif
}

/* Reset test state */
void reset_test_state(void)
{
    for (int i = 0; i < MAX_RESOURCE_TYPES; i++) {
        resource_data[i].current_consumption = 0;
        resource_data[i].max_consumption = 0;
    }
    sigfpe_caught = 0;
}

/* Test with zero max capacity */
int test_zero_capacity(void)
{
    reset_test_state();
    resource_data[RESOURCE_WHEAT].current_consumption = 0;
    resource_data[RESOURCE_WHEAT].max_consumption = 0;

    /* Set up signal handler for SIGFPE */
    struct sigaction sa, old_sa;
    sa.sa_handler = sigfpe_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGFPE, &sa, &old_sa);

    int percent = 0;
    if (setjmp(jump_buffer) == 0) {
        percent = get_resource_consumption_percent(RESOURCE_WHEAT);
    }

    /* Restore old handler */
    sigaction(SIGFPE, &old_sa, NULL);

    if (sigfpe_caught) {
        printf("FAIL: Division by zero crash with zero capacity\n");
        return 1;
    }

    if (percent != 0) {
        printf("FAIL: Expected 0%% for zero capacity, got %d%%\n", percent);
        return 1;
    }

    printf("PASS: Zero capacity test\n");
    return 0;
}

/* Test with normal consumption */
int test_normal_consumption(void)
{
    reset_test_state();
    resource_data[RESOURCE_TIMBER].current_consumption = 50;
    resource_data[RESOURCE_TIMBER].max_consumption = 100;

    int percent = get_resource_consumption_percent(RESOURCE_TIMBER);

    if (percent != 50) {
        printf("FAIL: Expected 50%% for 50/100, got %d%%\n", percent);
        return 1;
    }

    printf("PASS: Normal consumption test\n");
    return 0;
}

/* Test with full consumption */
int test_full_consumption(void)
{
    reset_test_state();
    resource_data[RESOURCE_IRON].current_consumption = 200;
    resource_data[RESOURCE_IRON].max_consumption = 200;

    int percent = get_resource_consumption_percent(RESOURCE_IRON);

    if (percent != 100) {
        printf("FAIL: Expected 100%% for full consumption, got %d%%\n", percent);
        return 1;
    }

    printf("PASS: Full consumption test\n");
    return 0;
}

/* Test with zero current consumption */
int test_zero_consumption(void)
{
    reset_test_state();
    resource_data[RESOURCE_WINE].current_consumption = 0;
    resource_data[RESOURCE_WINE].max_consumption = 100;

    int percent = get_resource_consumption_percent(RESOURCE_WINE);

    if (percent != 0) {
        printf("FAIL: Expected 0%% for zero consumption, got %d%%\n", percent);
        return 1;
    }

    printf("PASS: Zero consumption test\n");
    return 0;
}

/* Test with overconsumption (current > max) */
int test_overconsumption(void)
{
    reset_test_state();
    resource_data[RESOURCE_WHEAT].current_consumption = 150;
    resource_data[RESOURCE_WHEAT].max_consumption = 100;

    int percent = get_resource_consumption_percent(RESOURCE_WHEAT);

    if (percent != 150) {
        printf("FAIL: Expected 150%% for overconsumption, got %d%%\n", percent);
        return 1;
    }

    printf("PASS: Overconsumption test\n");
    return 0;
}

/* Test multiple resources */
int test_multiple_resources(void)
{
    reset_test_state();

    resource_data[RESOURCE_WHEAT].current_consumption = 25;
    resource_data[RESOURCE_WHEAT].max_consumption = 100;

    resource_data[RESOURCE_TIMBER].current_consumption = 75;
    resource_data[RESOURCE_TIMBER].max_consumption = 100;

    resource_data[RESOURCE_IRON].current_consumption = 0;
    resource_data[RESOURCE_IRON].max_consumption = 0;  /* No capacity */

    int wheat = get_resource_consumption_percent(RESOURCE_WHEAT);
    int timber = get_resource_consumption_percent(RESOURCE_TIMBER);
    int iron = get_resource_consumption_percent(RESOURCE_IRON);

    if (wheat != 25 || timber != 75 || iron != 0) {
        printf("FAIL: Multiple resources - wheat=%d, timber=%d, iron=%d\n",
               wheat, timber, iron);
        return 1;
    }

    printf("PASS: Multiple resources test\n");
    return 0;
}

int main(void)
{
    int failures = 0;

    printf("=== Resource Ratio Test ===\n\n");

#ifdef BUGGY_VERSION
    printf("Testing BUGGY version (expect crash on zero capacity)\n\n");
#else
    printf("Testing FIXED version (should pass all tests)\n\n");
#endif

    failures += test_zero_capacity();
    failures += test_normal_consumption();
    failures += test_full_consumption();
    failures += test_zero_consumption();
    failures += test_overconsumption();
    failures += test_multiple_resources();

    printf("\n=== Results: %d failures ===\n", failures);

    return failures;
}
