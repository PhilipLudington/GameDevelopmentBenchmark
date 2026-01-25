# Bug Report: Uninitialized memory in figure creation

## Summary

When creating new figures (game entities), the figure struct is allocated but not properly initialized, causing fields to contain garbage values which leads to undefined behavior when the figure is processed.

## Environment

- Project: Julius (Caesar III reimplementation)
- File: `src/figure/figure.c`
- Severity: Medium (undefined behavior, intermittent crashes)

## Bug Description

Julius uses a figure system to represent all moving entities in the game - citizens, traders, soldiers, animals, etc. When a new figure is created, memory is allocated for the figure struct, but the code fails to zero-initialize the struct before setting required fields.

```c
figure *figure_create(figure_type type, int x, int y)
{
    figure *f = malloc(sizeof(figure));
    if (!f) return NULL;

    // BUG: Struct not zeroed - contains garbage from previous allocations
    // Only some fields are set, others remain uninitialized

    f->type = type;
    f->x = x;
    f->y = y;
    f->state = FIGURE_STATE_ALIVE;

    // f->destination_x, f->destination_y, f->path_length, etc.
    // are NOT initialized - contain garbage values

    return f;
}
```

When the game later processes the figure, it reads uninitialized fields like `path_length` or `wait_ticks`, causing erratic behavior.

## Steps to Reproduce

1. Create a new figure during gameplay
2. The figure may have garbage values in unset fields
3. Processing the figure reads garbage, causing crashes or wrong behavior

## Expected Behavior

All figure struct fields should be initialized to known safe default values (typically 0) before the figure is used.

## Current Behavior

Uninitialized fields contain garbage values from previous heap allocations:
- `destination_x/y` may point to invalid map locations
- `path_length` may be enormous, causing array overruns
- Boolean flags may be non-zero when they should be false

Memory Sanitizer output:
```
==12345==WARNING: MemorySanitizer: use-of-uninitialized-value
    #0 figure_update_path figure.c:234
    #1 figure_action figure.c:156
```

## Relevant Code

Look at `src/figure/figure.c`:
- `figure_create()` function
- The `figure` struct definition (check all fields)
- Other allocation sites for figures

## Suggested Fix Approach

Use `calloc()` instead of `malloc()`, or explicitly zero the struct:

```c
figure *figure_create(figure_type type, int x, int y)
{
    // Option 1: Use calloc to zero-initialize
    figure *f = calloc(1, sizeof(figure));

    // Option 2: Use malloc + memset
    // figure *f = malloc(sizeof(figure));
    // if (f) memset(f, 0, sizeof(figure));

    if (!f) return NULL;

    f->type = type;
    f->x = x;
    f->y = y;
    f->state = FIGURE_STATE_ALIVE;

    return f;
}
```

## Your Task

Fix the uninitialized memory bug by ensuring the figure struct is properly zeroed. Your fix should:

1. Zero-initialize the entire figure struct on allocation
2. Maintain the explicit field assignments that follow
3. Not change the function signature or return behavior
4. Work correctly even if the struct size changes in the future

Provide your fix as a unified diff (patch).
