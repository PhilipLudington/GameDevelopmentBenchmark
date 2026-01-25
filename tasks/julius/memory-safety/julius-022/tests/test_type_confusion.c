/*
 * Test for julius-022: Type confusion with building ID cast
 *
 * This test verifies that the service worker function uses the correct
 * ID field (home_building_id, not figure id) to look up buildings.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BUILDINGS 100  /* Small for testing */
#define MAX_FIGURES 500    /* Larger than MAX_BUILDINGS to trigger bug */

typedef struct {
    int id;
    int type;
    int has_worker;
    int x, y;
} building;

typedef struct {
    int id;
    int home_building_id;
    int x, y;
    int type;
} figure;

static building *buildings = NULL;
static figure *figures = NULL;

/* Initialize test data */
void init_test_data(void)
{
    buildings = calloc(MAX_BUILDINGS, sizeof(building));
    figures = calloc(MAX_FIGURES, sizeof(figure));

    /* Initialize buildings */
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        buildings[i].id = i;
        buildings[i].type = 1;
        buildings[i].has_worker = 1;  /* All have workers initially */
    }

    /* Initialize figures - note IDs can exceed building array size */
    for (int i = 0; i < MAX_FIGURES; i++) {
        figures[i].id = i;
        figures[i].home_building_id = i % MAX_BUILDINGS;  /* Valid building ID */
    }
}

void cleanup_test_data(void)
{
    free(buildings);
    free(figures);
    buildings = NULL;
    figures = NULL;
}

/*
 * Simulates the buggy/fixed service worker function
 */
void service_worker_return_home(figure *f)
{
#ifdef BUGGY_VERSION
    /* BUG: Uses figure ID as building index */
    buildings[f->id].has_worker = 0;
#else
    /* FIXED: Use home_building_id with bounds check */
    int building_id = f->home_building_id;
    if (building_id < 0 || building_id >= MAX_BUILDINGS) {
        return;
    }
    buildings[building_id].has_worker = 0;
#endif
}

/* Test with figure ID within building range */
int test_low_figure_id(void)
{
    init_test_data();

    /* Figure with ID 50, home_building_id 50 - both within range */
    figure *f = &figures[50];
    f->home_building_id = 50;

    /* Mark building as having worker */
    buildings[50].has_worker = 1;

    service_worker_return_home(f);

    /* Check correct building was updated */
    if (buildings[50].has_worker != 0) {
        printf("FAIL: Building 50 should have has_worker = 0\n");
        cleanup_test_data();
        return 1;
    }

    cleanup_test_data();
    printf("PASS: Low figure ID test\n");
    return 0;
}

/* Test with figure ID exceeding building range - triggers OOB if buggy */
int test_high_figure_id(void)
{
    init_test_data();

    /* Figure with ID 300 (> MAX_BUILDINGS), but home_building_id 25 */
    figure *f = &figures[300];
    f->home_building_id = 25;

    printf("Testing figure ID %d (> MAX_BUILDINGS %d) with home_building %d...\n",
           f->id, MAX_BUILDINGS, f->home_building_id);

    /* Mark correct building as having worker */
    buildings[25].has_worker = 1;

    /* This will access buildings[300] if buggy - out of bounds! */
    service_worker_return_home(f);

    /* Check correct building was updated (not buildings[300]) */
    if (buildings[25].has_worker != 0) {
        printf("FAIL: Building 25 should have has_worker = 0\n");
        cleanup_test_data();
        return 1;
    }

    cleanup_test_data();
    printf("PASS: High figure ID test\n");
    return 0;
}

/* Test with maximum figure ID */
int test_max_figure_id(void)
{
    init_test_data();

    /* Figure with ID MAX_FIGURES-1 */
    figure *f = &figures[MAX_FIGURES - 1];
    f->home_building_id = 10;

    printf("Testing max figure ID %d...\n", f->id);

    buildings[10].has_worker = 1;

    service_worker_return_home(f);

    if (buildings[10].has_worker != 0) {
        printf("FAIL: Building 10 should have has_worker = 0\n");
        cleanup_test_data();
        return 1;
    }

    cleanup_test_data();
    printf("PASS: Max figure ID test\n");
    return 0;
}

/* Test with invalid home_building_id */
int test_invalid_building_id(void)
{
    init_test_data();

    figure *f = &figures[50];
    f->home_building_id = -1;  /* Invalid */

    /* Should handle gracefully without crash */
    service_worker_return_home(f);

    f->home_building_id = MAX_BUILDINGS + 100;  /* Also invalid */
    service_worker_return_home(f);

    cleanup_test_data();
    printf("PASS: Invalid building ID test\n");
    return 0;
}

/* Test that correct building is modified */
int test_correct_building_modified(void)
{
    init_test_data();

    /* Figure 200 with home_building 42 */
    figure *f = &figures[200];
    f->home_building_id = 42;

    /* Set up: building 42 has worker, others don't */
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        buildings[i].has_worker = (i == 42) ? 1 : 0;
    }

    service_worker_return_home(f);

    /* Only building 42 should be changed */
    if (buildings[42].has_worker != 0) {
        printf("FAIL: Building 42 should be updated\n");
        cleanup_test_data();
        return 1;
    }

    /* Verify no other buildings were changed (spot check) */
    if (buildings[0].has_worker != 0 || buildings[99].has_worker != 0) {
        printf("FAIL: Wrong building was modified\n");
        cleanup_test_data();
        return 1;
    }

    cleanup_test_data();
    printf("PASS: Correct building modified test\n");
    return 0;
}

int main(void)
{
    int failures = 0;

    printf("=== Type Confusion Test ===\n\n");

#ifdef BUGGY_VERSION
    printf("Testing BUGGY version (expect ASan to catch OOB access)\n\n");
#else
    printf("Testing FIXED version (should pass all tests)\n\n");
#endif

    failures += test_low_figure_id();
    failures += test_high_figure_id();
    failures += test_max_figure_id();
    failures += test_invalid_building_id();
    failures += test_correct_building_modified();

    printf("\n=== Results: %d failures ===\n", failures);

    return failures;
}
