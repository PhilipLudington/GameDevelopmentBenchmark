/*
 * Test for julius-027: Division by zero in population ratio calculation
 *
 * This test verifies that calculating worker ratio with zero population
 * does not cause a division by zero crash.
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>

/* For catching SIGFPE */
static jmp_buf jump_buffer;
static volatile int sigfpe_caught = 0;

void sigfpe_handler(int sig)
{
    (void)sig;
    sigfpe_caught = 1;
    longjmp(jump_buffer, 1);
}

/* Simulated city data */
struct {
    struct {
        int working;
        int total;
    } population;
} city_data;

/*
 * Calculate worker ratio - buggy or fixed version
 */
int calculate_worker_ratio(void)
{
    int workers = city_data.population.working;
    int total = city_data.population.total;

#ifdef BUGGY_VERSION
    /* BUG: No check for zero population - will crash */
    int ratio = (workers * 100) / total;
#else
    /* FIXED: Check for zero population */
    if (total == 0) {
        return 0;
    }
    int ratio = (workers * 100) / total;
#endif

    return ratio;
}

/* Reset test state */
void reset_test_state(void)
{
    city_data.population.working = 0;
    city_data.population.total = 0;
    sigfpe_caught = 0;
}

/* Test with zero population */
int test_zero_population(void)
{
    reset_test_state();
    city_data.population.working = 0;
    city_data.population.total = 0;

    /* Set up signal handler for SIGFPE */
    struct sigaction sa, old_sa;
    sa.sa_handler = sigfpe_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGFPE, &sa, &old_sa);

    int ratio = 0;
    if (setjmp(jump_buffer) == 0) {
        ratio = calculate_worker_ratio();
    }

    /* Restore old handler */
    sigaction(SIGFPE, &old_sa, NULL);

    if (sigfpe_caught) {
        printf("FAIL: Division by zero crash with zero population\n");
        return 1;
    }

    if (ratio != 0) {
        printf("FAIL: Expected ratio 0 for zero population, got %d\n", ratio);
        return 1;
    }

    printf("PASS: Zero population test\n");
    return 0;
}

/* Test with normal population */
int test_normal_population(void)
{
    reset_test_state();
    city_data.population.working = 500;
    city_data.population.total = 1000;

    int ratio = calculate_worker_ratio();

    if (ratio != 50) {
        printf("FAIL: Expected ratio 50 for 500/1000, got %d\n", ratio);
        return 1;
    }

    printf("PASS: Normal population test\n");
    return 0;
}

/* Test with all workers */
int test_all_workers(void)
{
    reset_test_state();
    city_data.population.working = 1000;
    city_data.population.total = 1000;

    int ratio = calculate_worker_ratio();

    if (ratio != 100) {
        printf("FAIL: Expected ratio 100 for all workers, got %d\n", ratio);
        return 1;
    }

    printf("PASS: All workers test\n");
    return 0;
}

/* Test with no workers */
int test_no_workers(void)
{
    reset_test_state();
    city_data.population.working = 0;
    city_data.population.total = 1000;

    int ratio = calculate_worker_ratio();

    if (ratio != 0) {
        printf("FAIL: Expected ratio 0 for no workers, got %d\n", ratio);
        return 1;
    }

    printf("PASS: No workers test\n");
    return 0;
}

/* Test with small population */
int test_small_population(void)
{
    reset_test_state();
    city_data.population.working = 1;
    city_data.population.total = 3;

    int ratio = calculate_worker_ratio();

    /* 1 * 100 / 3 = 33 */
    if (ratio != 33) {
        printf("FAIL: Expected ratio 33 for 1/3, got %d\n", ratio);
        return 1;
    }

    printf("PASS: Small population test\n");
    return 0;
}

/* Test edge case: working > total (shouldn't happen but test anyway) */
int test_workers_exceed_population(void)
{
    reset_test_state();
    city_data.population.working = 1500;
    city_data.population.total = 1000;

    int ratio = calculate_worker_ratio();

    /* Should return 150% */
    if (ratio != 150) {
        printf("FAIL: Expected ratio 150 for 1500/1000, got %d\n", ratio);
        return 1;
    }

    printf("PASS: Workers exceed population test\n");
    return 0;
}

int main(void)
{
    int failures = 0;

    printf("=== Population Ratio Test ===\n\n");

#ifdef BUGGY_VERSION
    printf("Testing BUGGY version (expect crash on zero population)\n\n");
#else
    printf("Testing FIXED version (should pass all tests)\n\n");
#endif

    failures += test_zero_population();
    failures += test_normal_population();
    failures += test_all_workers();
    failures += test_no_workers();
    failures += test_small_population();
    failures += test_workers_exceed_population();

    printf("\n=== Results: %d failures ===\n", failures);

    return failures;
}
