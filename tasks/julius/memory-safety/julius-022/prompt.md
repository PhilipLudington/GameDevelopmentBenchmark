# Bug Report: Type confusion with building ID cast

## Summary

A function uses a figure ID as if it were a building ID, directly indexing into the buildings array. Since figure IDs and building IDs use different ranges and meanings, this causes out-of-bounds access when the figure ID exceeds the buildings array size.

## Environment

- Project: Julius (Caesar III reimplementation)
- File: `src/figure/service.c`
- Severity: High (memory corruption, potential code execution)

## Bug Description

The game has separate ID systems for figures (moving entities) and buildings (static structures). A service worker figure needs to know which building it came from (`home_building_id`). However, a bug causes the code to use `figure->id` (the figure's own ID) instead of `figure->home_building_id`:

```c
#define MAX_BUILDINGS 2000
#define MAX_FIGURES 5000

static building buildings[MAX_BUILDINGS];
static figure figures[MAX_FIGURES];

void service_worker_return_home(figure *f)
{
    // BUG: Uses figure ID (0-4999) as building index
    // Should use f->home_building_id instead
    building *home = &buildings[f->id];  // Type confusion!

    // If figure ID is 3000, this reads buildings[3000]
    // but buildings array only has 2000 elements
    home->has_worker = 0;
}
```

Figure IDs range from 0 to MAX_FIGURES-1 (4999), but building IDs only range from 0 to MAX_BUILDINGS-1 (1999). When a figure with ID >= 2000 is processed, this causes an out-of-bounds access.

## Steps to Reproduce

1. Create many figures (more than MAX_BUILDINGS)
2. Call `service_worker_return_home()` on a figure with high ID
3. Out-of-bounds array access occurs

## Expected Behavior

The function should use `f->home_building_id` to look up the correct building.

## Current Behavior

Using `f->id` instead of `f->home_building_id` causes:
- Accessing garbage memory when figure ID > MAX_BUILDINGS
- Corrupting unrelated memory
- Potential crash

AddressSanitizer output:
```
==12345==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x6020000f0000
READ of size 4 at 0x6020000f0000
    #0 service_worker_return_home service.c:123
```

## Relevant Code

Look at `src/figure/service.c`:
- `service_worker_return_home()` function
- Any other places where figure ID might be confused with building ID
- Check for similar patterns in other figure handling code

## Suggested Fix Approach

Use the correct field (`home_building_id`):

```c
void service_worker_return_home(figure *f)
{
    // FIXED: Use home_building_id, not figure's own ID
    if (f->home_building_id < 0 || f->home_building_id >= MAX_BUILDINGS) {
        return;  // Invalid building ID
    }
    building *home = &buildings[f->home_building_id];
    home->has_worker = 0;
}
```

## Your Task

Fix the type confusion by using the correct ID field. Your fix should:

1. Use `f->home_building_id` instead of `f->id`
2. Add bounds checking for the building ID
3. Handle invalid building IDs gracefully
4. Not change any other functionality

Provide your fix as a unified diff (patch).
