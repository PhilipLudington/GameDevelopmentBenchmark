/*
 * Test for julius-020: Memory leak in error handling path
 *
 * This test verifies that scenario loading properly frees resources
 * when an error occurs during initialization.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAP_SIZE 10000
#define MAX_BUILDINGS 500
#define MAX_FIGURES 1000

typedef struct building {
    int id;
    int type;
} building;

typedef struct figure {
    int id;
    int type;
} figure;

typedef struct {
    int *map_data;
    building *buildings;
    figure *figures;
    int num_buildings;
    int num_figures;
} scenario_data;

/* Control variable to simulate failures at different stages */
static int fail_at_stage = -1;

/* Simulated file loading */
int load_scenario_file(const char *filename, scenario_data *data)
{
    (void)filename;
    if (fail_at_stage == 4) {
        return 0;  /* Simulate file load failure */
    }
    /* Simulate loading some data */
    data->num_buildings = 10;
    data->num_figures = 20;
    return 1;
}

/*
 * Simulates the buggy/fixed scenario loading
 */
int scenario_load(const char *filename, scenario_data *data)
{
    memset(data, 0, sizeof(scenario_data));

    /* Step 1: Allocate map */
    if (fail_at_stage == 1) {
        return 0;  /* Simulate malloc failure */
    }
    data->map_data = malloc(MAP_SIZE * sizeof(int));
    if (!data->map_data) {
        return 0;
    }

    /* Step 2: Allocate buildings */
    if (fail_at_stage == 2) {
#ifdef BUGGY_VERSION
        /* BUG: map_data not freed */
        return 0;
#else
        /* FIXED: Free map_data */
        free(data->map_data);
        data->map_data = NULL;
        return 0;
#endif
    }
    data->buildings = malloc(MAX_BUILDINGS * sizeof(building));
    if (!data->buildings) {
#ifdef BUGGY_VERSION
        return 0;
#else
        free(data->map_data);
        data->map_data = NULL;
        return 0;
#endif
    }

    /* Step 3: Allocate figures */
    if (fail_at_stage == 3) {
#ifdef BUGGY_VERSION
        /* BUG: map_data and buildings not freed */
        return 0;
#else
        /* FIXED: Free both */
        free(data->buildings);
        free(data->map_data);
        data->buildings = NULL;
        data->map_data = NULL;
        return 0;
#endif
    }
    data->figures = malloc(MAX_FIGURES * sizeof(figure));
    if (!data->figures) {
#ifdef BUGGY_VERSION
        return 0;
#else
        free(data->buildings);
        free(data->map_data);
        data->buildings = NULL;
        data->map_data = NULL;
        return 0;
#endif
    }

    /* Step 4: Load file data */
    if (!load_scenario_file(filename, data)) {
#ifdef BUGGY_VERSION
        /* BUG: Nothing freed */
        return 0;
#else
        /* FIXED: Free all */
        free(data->figures);
        free(data->buildings);
        free(data->map_data);
        data->figures = NULL;
        data->buildings = NULL;
        data->map_data = NULL;
        return 0;
#endif
    }

    return 1;
}

void scenario_free(scenario_data *data)
{
    if (data->figures) {
        free(data->figures);
        data->figures = NULL;
    }
    if (data->buildings) {
        free(data->buildings);
        data->buildings = NULL;
    }
    if (data->map_data) {
        free(data->map_data);
        data->map_data = NULL;
    }
}

/* Test successful load */
int test_successful_load(void)
{
    scenario_data data;
    fail_at_stage = -1;

    if (!scenario_load("test.sav", &data)) {
        printf("FAIL: Successful load failed\n");
        return 1;
    }

    if (!data.map_data || !data.buildings || !data.figures) {
        printf("FAIL: Data not allocated properly\n");
        scenario_free(&data);
        return 1;
    }

    scenario_free(&data);
    printf("PASS: Successful load test\n");
    return 0;
}

/* Test failure at stage 2 (buildings allocation) */
int test_failure_stage_2(void)
{
    scenario_data data;
    fail_at_stage = 2;

    printf("Testing failure at stage 2 (buildings)...\n");

    int result = scenario_load("test.sav", &data);

    if (result != 0) {
        printf("FAIL: Load should have failed at stage 2\n");
        scenario_free(&data);
        return 1;
    }

    /* On failure, all pointers should be NULL (if properly cleaned up) */
    /* LSan will detect if map_data was leaked */

    printf("PASS: Stage 2 failure test\n");
    return 0;
}

/* Test failure at stage 3 (figures allocation) */
int test_failure_stage_3(void)
{
    scenario_data data;
    fail_at_stage = 3;

    printf("Testing failure at stage 3 (figures)...\n");

    int result = scenario_load("test.sav", &data);

    if (result != 0) {
        printf("FAIL: Load should have failed at stage 3\n");
        scenario_free(&data);
        return 1;
    }

    /* LSan will detect if map_data or buildings were leaked */

    printf("PASS: Stage 3 failure test\n");
    return 0;
}

/* Test failure at stage 4 (file loading) */
int test_failure_stage_4(void)
{
    scenario_data data;
    fail_at_stage = 4;

    printf("Testing failure at stage 4 (file load)...\n");

    int result = scenario_load("test.sav", &data);

    if (result != 0) {
        printf("FAIL: Load should have failed at stage 4\n");
        scenario_free(&data);
        return 1;
    }

    /* LSan will detect if any allocations were leaked */

    printf("PASS: Stage 4 failure test\n");
    return 0;
}

/* Test multiple failures to check cumulative leaks */
int test_multiple_failures(void)
{
    scenario_data data;

    printf("Testing multiple failures...\n");

    for (int i = 0; i < 10; i++) {
        fail_at_stage = (i % 3) + 2;  /* Stages 2, 3, 4 */
        scenario_load("test.sav", &data);
    }

    /* LSan will catch if memory accumulated */

    printf("PASS: Multiple failures test\n");
    return 0;
}

int main(void)
{
    int failures = 0;

    printf("=== Scenario Memory Leak Test ===\n\n");

#ifdef BUGGY_VERSION
    printf("Testing BUGGY version (expect LSan to detect leaks)\n\n");
#else
    printf("Testing FIXED version (should pass all tests)\n\n");
#endif

    failures += test_successful_load();
    failures += test_failure_stage_2();
    failures += test_failure_stage_3();
    failures += test_failure_stage_4();
    failures += test_multiple_failures();

    printf("\n=== Results: %d failures ===\n", failures);

    return failures;
}
