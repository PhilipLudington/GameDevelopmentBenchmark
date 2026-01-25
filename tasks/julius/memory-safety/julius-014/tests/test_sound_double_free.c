/*
 * Test for julius-014: Double free in sound system cleanup
 *
 * This test verifies that the sound system properly handles cleanup
 * after an initialization failure without causing a double-free.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SOUNDS 10
#define SOUND_BUFFER_SIZE 1024

/* Control variable to simulate allocation failure */
static int fail_at_allocation = -1;
static int allocation_count = 0;

typedef struct {
    unsigned char *data;
    int size;
    int loaded;
} sound_buffer;

static sound_buffer sounds[MAX_SOUNDS];

/* Wrapper malloc that can simulate failure */
static void *test_malloc(size_t size)
{
    if (fail_at_allocation >= 0 && allocation_count >= fail_at_allocation) {
        return NULL;  /* Simulate failure */
    }
    allocation_count++;
    return malloc(size);
}

/*
 * Simulates the buggy/fixed sound initialization
 */
int sound_init(void)
{
    for (int i = 0; i < MAX_SOUNDS; i++) {
        sounds[i].data = test_malloc(SOUND_BUFFER_SIZE);
        if (!sounds[i].data) {
            /* Error - cleanup already allocated buffers */
            for (int j = 0; j < i; j++) {
#ifdef BUGGY_VERSION
                free(sounds[j].data);  /* BUG: No NULL assignment */
#else
                free(sounds[j].data);
                sounds[j].data = NULL;  /* FIXED: Nullify after free */
#endif
            }
            return 0;
        }
        sounds[i].size = SOUND_BUFFER_SIZE;
        sounds[i].loaded = 1;
    }
    return 1;
}

/*
 * Simulates the buggy/fixed sound cleanup
 */
void sound_cleanup(void)
{
    for (int i = 0; i < MAX_SOUNDS; i++) {
#ifdef BUGGY_VERSION
        free(sounds[i].data);  /* BUG: No NULL check */
#else
        if (sounds[i].data) {  /* FIXED: Check before free */
            free(sounds[i].data);
            sounds[i].data = NULL;
        }
#endif
    }
}

/* Reset test state */
void reset_test_state(void)
{
    memset(sounds, 0, sizeof(sounds));
    allocation_count = 0;
    fail_at_allocation = -1;
}

/* Test normal initialization and cleanup */
int test_normal_lifecycle(void)
{
    reset_test_state();

    if (!sound_init()) {
        printf("FAIL: Normal init failed unexpectedly\n");
        return 1;
    }

    /* Verify all buffers allocated */
    for (int i = 0; i < MAX_SOUNDS; i++) {
        if (!sounds[i].data) {
            printf("FAIL: Sound buffer %d not allocated\n", i);
            return 1;
        }
    }

    /* This should work without double-free */
    sound_cleanup();

    printf("PASS: Normal lifecycle test\n");
    return 0;
}

/* Test initialization failure and cleanup - triggers double free if buggy */
int test_init_failure_cleanup(void)
{
    reset_test_state();

    /* Fail on the 5th allocation (index 4) */
    fail_at_allocation = 5;

    printf("Testing init failure at allocation %d...\n", fail_at_allocation);

    int init_result = sound_init();
    if (init_result != 0) {
        printf("FAIL: Init should have failed but returned success\n");
        return 1;
    }

    printf("Init failed as expected, now testing cleanup...\n");

    /* This will trigger double-free if the error path didn't NULL pointers */
    sound_cleanup();

    printf("PASS: Init failure + cleanup test (no double-free)\n");
    return 0;
}

/* Test multiple cleanup calls */
int test_multiple_cleanup(void)
{
    reset_test_state();

    if (!sound_init()) {
        printf("FAIL: Init failed in multiple cleanup test\n");
        return 1;
    }

    /* Call cleanup multiple times - should be safe with NULL checks */
    sound_cleanup();
    sound_cleanup();
    sound_cleanup();

    printf("PASS: Multiple cleanup calls test\n");
    return 0;
}

/* Test cleanup without init */
int test_cleanup_without_init(void)
{
    reset_test_state();

    /* Don't call init, just cleanup */
    /* All pointers should be NULL from memset */
    sound_cleanup();

    printf("PASS: Cleanup without init test\n");
    return 0;
}

/* Test failure at first allocation */
int test_fail_first_allocation(void)
{
    reset_test_state();
    fail_at_allocation = 0;

    int init_result = sound_init();
    if (init_result != 0) {
        printf("FAIL: Init should have failed at first allocation\n");
        return 1;
    }

    sound_cleanup();

    printf("PASS: Fail at first allocation test\n");
    return 0;
}

/* Test failure at last allocation */
int test_fail_last_allocation(void)
{
    reset_test_state();
    fail_at_allocation = MAX_SOUNDS - 1;

    printf("Testing failure at last allocation (index %d)...\n",
           fail_at_allocation);

    int init_result = sound_init();
    if (init_result != 0) {
        printf("FAIL: Init should have failed at last allocation\n");
        return 1;
    }

    /* This is the most likely to trigger double-free */
    sound_cleanup();

    printf("PASS: Fail at last allocation test\n");
    return 0;
}

int main(void)
{
    int failures = 0;

    printf("=== Sound System Double-Free Test ===\n\n");

#ifdef BUGGY_VERSION
    printf("Testing BUGGY version (expect ASan to catch double-free)\n\n");
#else
    printf("Testing FIXED version (should pass all tests)\n\n");
#endif

    failures += test_normal_lifecycle();
    failures += test_init_failure_cleanup();
    failures += test_multiple_cleanup();
    failures += test_cleanup_without_init();
    failures += test_fail_first_allocation();
    failures += test_fail_last_allocation();

    printf("\n=== Results: %d failures ===\n", failures);

    return failures;
}
