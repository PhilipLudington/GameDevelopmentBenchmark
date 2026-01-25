# Bug Report: Memory leak in error handling path

## Summary

The scenario loading function allocates multiple resources during initialization. When an error occurs partway through, the already-allocated resources are not freed before returning, causing a memory leak.

## Environment

- Project: Julius (Caesar III reimplementation)
- File: `src/scenario/scenario.c`
- Severity: Medium (resource exhaustion over time)

## Bug Description

Loading a scenario involves multiple allocation steps: loading the map, setting up buildings, initializing figures, etc. If any step fails, the function returns early without freeing resources allocated in previous steps.

```c
typedef struct {
    int *map_data;
    building *buildings;
    figure *figures;
    int num_buildings;
    int num_figures;
} scenario_data;

int scenario_load(const char *filename, scenario_data *data)
{
    // Step 1: Allocate map
    data->map_data = malloc(MAP_SIZE * sizeof(int));
    if (!data->map_data) {
        return 0;  // Error
    }

    // Step 2: Allocate buildings
    data->buildings = malloc(MAX_BUILDINGS * sizeof(building));
    if (!data->buildings) {
        // BUG: map_data not freed!
        return 0;  // Error, but map_data leaked
    }

    // Step 3: Allocate figures
    data->figures = malloc(MAX_FIGURES * sizeof(figure));
    if (!data->figures) {
        // BUG: map_data and buildings not freed!
        return 0;  // Error, both leaked
    }

    // ... load data ...

    return 1;  // Success
}
```

Each failed scenario load leaks memory. Over multiple load attempts, memory is exhausted.

## Steps to Reproduce

1. Attempt to load an invalid scenario (or simulate allocation failure)
2. Memory is leaked
3. Repeat multiple times
4. System memory gradually decreases

## Expected Behavior

Error paths should free all previously allocated resources before returning.

## Current Behavior

Resources allocated before the error are leaked:
- If step 2 fails: map_data is leaked
- If step 3 fails: map_data and buildings are leaked

LeakSanitizer output:
```
==12345==ERROR: LeakSanitizer: detected memory leaks

Direct leak of 10000 bytes in 1 object(s) allocated from:
    #0 malloc
    #1 scenario_load scenario.c:34
```

## Relevant Code

Look at `src/scenario/scenario.c`:
- `scenario_load()` function
- All error return paths
- Any other multi-step initialization functions

## Suggested Fix Approach

Free previously allocated resources on error:

```c
int scenario_load(const char *filename, scenario_data *data)
{
    // Step 1: Allocate map
    data->map_data = malloc(MAP_SIZE * sizeof(int));
    if (!data->map_data) {
        return 0;
    }

    // Step 2: Allocate buildings
    data->buildings = malloc(MAX_BUILDINGS * sizeof(building));
    if (!data->buildings) {
        free(data->map_data);  // Clean up step 1
        data->map_data = NULL;
        return 0;
    }

    // Step 3: Allocate figures
    data->figures = malloc(MAX_FIGURES * sizeof(figure));
    if (!data->figures) {
        free(data->buildings);  // Clean up step 2
        free(data->map_data);   // Clean up step 1
        data->buildings = NULL;
        data->map_data = NULL;
        return 0;
    }

    // ... load data ...

    return 1;
}
```

Or use a centralized cleanup function with goto.

## Your Task

Fix the memory leak by ensuring all allocated resources are freed on error. Your fix should:

1. Free previously allocated resources when an error occurs
2. Set freed pointers to NULL for safety
3. Not leak any memory on any error path
4. Not change behavior on successful load

Provide your fix as a unified diff (patch).
