/*
 * Test for julius-031: Assert failure when building state invalid in debug mode
 *
 * This test verifies that buildings with invalid state values are handled
 * gracefully without crashing.
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>

#define BUILDING_STATE_NONE 0
#define BUILDING_STATE_ACTIVE 1
#define BUILDING_STATE_DELETED 2
#define BUILDING_STATE_RUBBLE 3
#define BUILDING_STATE_MAX 4

typedef struct {
    int id;
    int state;
    int type;
} building;

/* Test tracking */
static int active_processed = 0;
static int rubble_processed = 0;
static int error_logged = 0;
static int last_error_building_id = -1;
static int last_error_state = -1;

/* For catching SIGABRT from assert */
static jmp_buf jump_buffer;
static volatile int sigabrt_caught = 0;

void sigabrt_handler(int sig)
{
    (void)sig;
    sigabrt_caught = 1;
    longjmp(jump_buffer, 1);
}

/* Simulated functions */
void process_active_building(building *b)
{
    (void)b;
    active_processed++;
}

void process_rubble(building *b)
{
    (void)b;
    rubble_processed++;
}

void log_error(const char *msg, int id, int state)
{
    (void)msg;
    error_logged++;
    last_error_building_id = id;
    last_error_state = state;
}

/* Custom assert that can be caught */
#ifdef BUGGY_VERSION
#define my_assert(expr) do { if (!(expr)) { raise(SIGABRT); } } while(0)
#else
#define my_assert(expr) ((void)0)  /* No-op in fixed version */
#endif

/*
 * Update building function - buggy or fixed version
 */
void update_building(building *b)
{
    if (!b) {
        return;
    }

#ifdef BUGGY_VERSION
    /* BUG: Assert crashes with invalid state */
    my_assert(b->state >= BUILDING_STATE_NONE && b->state < BUILDING_STATE_MAX);
#else
    /* FIXED: Graceful handling of invalid state */
    if (b->state < BUILDING_STATE_NONE || b->state >= BUILDING_STATE_MAX) {
        log_error("Invalid building state", b->id, b->state);
        b->state = BUILDING_STATE_DELETED;
        return;
    }
#endif

    switch (b->state) {
        case BUILDING_STATE_ACTIVE:
            process_active_building(b);
            break;
        case BUILDING_STATE_RUBBLE:
            process_rubble(b);
            break;
        default:
            break;
    }
}

/* Reset test state */
void reset_test_state(void)
{
    active_processed = 0;
    rubble_processed = 0;
    error_logged = 0;
    last_error_building_id = -1;
    last_error_state = -1;
    sigabrt_caught = 0;
}

/* Test valid active state */
int test_valid_active_state(void)
{
    reset_test_state();

    building b = {.id = 1, .state = BUILDING_STATE_ACTIVE, .type = 10};

    update_building(&b);

    if (active_processed != 1) {
        printf("FAIL: Active building not processed\n");
        return 1;
    }

    printf("PASS: Valid active state test\n");
    return 0;
}

/* Test valid rubble state */
int test_valid_rubble_state(void)
{
    reset_test_state();

    building b = {.id = 2, .state = BUILDING_STATE_RUBBLE, .type = 10};

    update_building(&b);

    if (rubble_processed != 1) {
        printf("FAIL: Rubble building not processed\n");
        return 1;
    }

    printf("PASS: Valid rubble state test\n");
    return 0;
}

/* Test invalid high state value */
int test_invalid_high_state(void)
{
    reset_test_state();

    building b = {.id = 3, .state = 99, .type = 10};  /* Invalid state */

    /* Set up signal handler for SIGABRT */
    struct sigaction sa, old_sa;
    sa.sa_handler = sigabrt_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGABRT, &sa, &old_sa);

    if (setjmp(jump_buffer) == 0) {
        update_building(&b);
    }

    /* Restore old handler */
    sigaction(SIGABRT, &old_sa, NULL);

    if (sigabrt_caught) {
        printf("FAIL: Assert crashed on invalid state 99\n");
        return 1;
    }

#ifndef BUGGY_VERSION
    if (error_logged != 1) {
        printf("FAIL: Error not logged for invalid state\n");
        return 1;
    }

    if (b.state != BUILDING_STATE_DELETED) {
        printf("FAIL: Invalid state not corrected to DELETED\n");
        return 1;
    }
#endif

    printf("PASS: Invalid high state test\n");
    return 0;
}

/* Test invalid negative state value */
int test_invalid_negative_state(void)
{
    reset_test_state();

    building b = {.id = 4, .state = -1, .type = 10};  /* Invalid negative */

    /* Set up signal handler for SIGABRT */
    struct sigaction sa, old_sa;
    sa.sa_handler = sigabrt_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGABRT, &sa, &old_sa);

    if (setjmp(jump_buffer) == 0) {
        update_building(&b);
    }

    /* Restore old handler */
    sigaction(SIGABRT, &old_sa, NULL);

    if (sigabrt_caught) {
        printf("FAIL: Assert crashed on negative state\n");
        return 1;
    }

#ifndef BUGGY_VERSION
    if (b.state != BUILDING_STATE_DELETED) {
        printf("FAIL: Negative state not corrected\n");
        return 1;
    }
#endif

    printf("PASS: Invalid negative state test\n");
    return 0;
}

/* Test null building pointer */
int test_null_building(void)
{
    reset_test_state();

    update_building(NULL);

    /* Should not crash */
    printf("PASS: Null building test\n");
    return 0;
}

/* Test boundary state value */
int test_boundary_state(void)
{
    reset_test_state();

    building b = {.id = 5, .state = BUILDING_STATE_MAX, .type = 10};  /* Just over */

    /* Set up signal handler for SIGABRT */
    struct sigaction sa, old_sa;
    sa.sa_handler = sigabrt_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGABRT, &sa, &old_sa);

    if (setjmp(jump_buffer) == 0) {
        update_building(&b);
    }

    /* Restore old handler */
    sigaction(SIGABRT, &old_sa, NULL);

    if (sigabrt_caught) {
        printf("FAIL: Assert crashed on boundary state\n");
        return 1;
    }

    printf("PASS: Boundary state test\n");
    return 0;
}

int main(void)
{
    int failures = 0;

    printf("=== Building State Test ===\n\n");

#ifdef BUGGY_VERSION
    printf("Testing BUGGY version (expect assert crashes)\n\n");
#else
    printf("Testing FIXED version (should pass all tests)\n\n");
#endif

    failures += test_valid_active_state();
    failures += test_valid_rubble_state();
    failures += test_null_building();
    failures += test_invalid_high_state();
    failures += test_invalid_negative_state();
    failures += test_boundary_state();

    printf("\n=== Results: %d failures ===\n", failures);

    return failures;
}
