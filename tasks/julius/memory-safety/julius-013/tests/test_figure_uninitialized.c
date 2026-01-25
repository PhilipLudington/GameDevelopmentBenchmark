/*
 * Test for julius-013: Uninitialized memory in figure creation
 *
 * This test verifies that figure structs are properly initialized,
 * with all fields set to zero/null before use.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Figure type enumeration */
typedef enum {
    FIGURE_NONE = 0,
    FIGURE_CITIZEN = 1,
    FIGURE_TRADER = 2,
    FIGURE_SOLDIER = 3,
    FIGURE_ANIMAL = 4
} figure_type;

/* Figure state enumeration */
typedef enum {
    FIGURE_STATE_NONE = 0,
    FIGURE_STATE_ALIVE = 1,
    FIGURE_STATE_DEAD = 2
} figure_state;

/* Simplified figure struct matching Julius */
typedef struct {
    figure_type type;
    figure_state state;
    int x;
    int y;
    int destination_x;
    int destination_y;
    int path_length;
    int path_current;
    int wait_ticks;
    int action_counter;
    int speed;
    int direction;
    int is_ghost;
    int home_building_id;
    int destination_building_id;
    unsigned char path[500];  /* Large array - very visible if uninitialized */
} figure;

/*
 * Simulates the buggy/fixed figure creation
 */
figure *figure_create(figure_type type, int x, int y)
{
#ifdef BUGGY_VERSION
    /* BUG: malloc doesn't zero memory */
    figure *f = malloc(sizeof(figure));
#else
    /* FIXED: calloc zeros the memory */
    figure *f = calloc(1, sizeof(figure));
#endif

    if (!f) return NULL;

    f->type = type;
    f->x = x;
    f->y = y;
    f->state = FIGURE_STATE_ALIVE;

    return f;
}

/* Helper to pollute memory with non-zero values */
void pollute_heap(void)
{
    /* Allocate and fill with garbage, then free */
    void *garbage = malloc(sizeof(figure) * 10);
    if (garbage) {
        memset(garbage, 0xAB, sizeof(figure) * 10);
        free(garbage);
    }
}

/* Test that uninitialized fields are zeroed */
int test_fields_zeroed(void)
{
    /* First pollute the heap with garbage */
    pollute_heap();

    figure *f = figure_create(FIGURE_CITIZEN, 10, 20);
    if (!f) {
        printf("FAIL: Allocation failed\n");
        return 1;
    }

    int failures = 0;

    /* Check explicitly set fields */
    if (f->type != FIGURE_CITIZEN) {
        printf("FAIL: type not set correctly\n");
        failures++;
    }
    if (f->x != 10 || f->y != 20) {
        printf("FAIL: position not set correctly\n");
        failures++;
    }
    if (f->state != FIGURE_STATE_ALIVE) {
        printf("FAIL: state not set correctly\n");
        failures++;
    }

    /* Check fields that should be zero-initialized */
    if (f->destination_x != 0) {
        printf("FAIL: destination_x is %d, should be 0 (uninitialized)\n",
               f->destination_x);
        failures++;
    }
    if (f->destination_y != 0) {
        printf("FAIL: destination_y is %d, should be 0 (uninitialized)\n",
               f->destination_y);
        failures++;
    }
    if (f->path_length != 0) {
        printf("FAIL: path_length is %d, should be 0 (uninitialized)\n",
               f->path_length);
        failures++;
    }
    if (f->wait_ticks != 0) {
        printf("FAIL: wait_ticks is %d, should be 0 (uninitialized)\n",
               f->wait_ticks);
        failures++;
    }
    if (f->is_ghost != 0) {
        printf("FAIL: is_ghost is %d, should be 0 (uninitialized)\n",
               f->is_ghost);
        failures++;
    }

    /* Check that path array is zeroed */
    for (int i = 0; i < 100; i++) {
        if (f->path[i] != 0) {
            printf("FAIL: path[%d] is %d, should be 0 (uninitialized)\n",
                   i, f->path[i]);
            failures++;
            break;
        }
    }

    free(f);

    if (failures == 0) {
        printf("PASS: All uninitialized fields are properly zeroed\n");
    }

    return failures > 0 ? 1 : 0;
}

/* Test multiple figure allocations */
int test_multiple_figures(void)
{
    pollute_heap();

    figure *figures[10];
    int failures = 0;

    /* Create multiple figures */
    for (int i = 0; i < 10; i++) {
        figures[i] = figure_create(FIGURE_CITIZEN + (i % 4), i * 10, i * 5);
        if (!figures[i]) {
            printf("FAIL: Could not allocate figure %d\n", i);
            failures++;
            continue;
        }

        /* Check that uninitialized fields are zero */
        if (figures[i]->path_length != 0 ||
            figures[i]->destination_x != 0 ||
            figures[i]->wait_ticks != 0) {
            printf("FAIL: Figure %d has uninitialized garbage values\n", i);
            failures++;
        }
    }

    /* Clean up */
    for (int i = 0; i < 10; i++) {
        if (figures[i]) free(figures[i]);
    }

    if (failures == 0) {
        printf("PASS: Multiple figure allocation test\n");
    }

    return failures > 0 ? 1 : 0;
}

/* Test that path_length doesn't cause buffer overrun when used */
int test_path_usage(void)
{
    pollute_heap();

    figure *f = figure_create(FIGURE_TRADER, 100, 200);
    if (!f) {
        printf("FAIL: Allocation failed\n");
        return 1;
    }

    /* Simulate using path_length - if uninitialized, could be huge */
    printf("Testing path_length usage (value=%d)...\n", f->path_length);

    /* This could cause a crash or ASan error if path_length is garbage */
    int sum = 0;
    for (int i = 0; i < f->path_length && i < 500; i++) {
        sum += f->path[i];
    }

    free(f);

    printf("PASS: Path length usage test (sum=%d)\n", sum);
    return 0;
}

int main(void)
{
    int failures = 0;

    printf("=== Figure Uninitialized Memory Test ===\n\n");

#ifdef BUGGY_VERSION
    printf("Testing BUGGY version (expect uninitialized values)\n\n");
#else
    printf("Testing FIXED version (should pass all tests)\n\n");
#endif

    failures += test_fields_zeroed();
    failures += test_multiple_figures();
    failures += test_path_usage();

    printf("\n=== Results: %d failures ===\n", failures);

    return failures;
}
