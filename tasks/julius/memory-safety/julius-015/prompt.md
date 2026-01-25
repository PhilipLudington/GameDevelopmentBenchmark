# Bug Report: Use-after-free in building deletion callback

## Summary

When a building is deleted, the deletion process frees the building's memory before notifying registered callbacks. These callbacks still hold pointers to the freed building, causing use-after-free when they access building data.

## Environment

- Project: Julius (Caesar III reimplementation)
- File: `src/building/building.c`
- Severity: Critical (memory corruption, potential code execution)

## Bug Description

Buildings in Julius can register callbacks for events like resource delivery or worker assignment. When a building is deleted (e.g., collapsed, demolished), the system should notify callbacks before freeing the building memory. However, the current code frees the building first, then calls the callbacks.

```c
typedef void (*building_callback)(building *b, int event);

static struct {
    building *b;
    building_callback callback;
} registered_callbacks[MAX_CALLBACKS];

void building_delete(building *b)
{
    int building_id = b->id;

    // BUG: Free building memory first
    free(b);

    // Then notify callbacks - but b is now freed!
    for (int i = 0; i < num_callbacks; i++) {
        if (registered_callbacks[i].b == b) {
            // Use-after-free: callback reads from freed memory
            registered_callbacks[i].callback(b, EVENT_BUILDING_DELETED);
        }
    }

    // Clear from building list
    buildings[building_id] = NULL;
}
```

## Steps to Reproduce

1. Create a building with registered callbacks
2. Delete the building (collapse or demolish)
3. Callback accesses freed building memory
4. Crash or memory corruption

## Expected Behavior

Callbacks should be notified BEFORE the building is freed, or should receive just the building ID rather than a pointer to freed memory.

## Current Behavior

Callbacks receive a pointer to already-freed memory, causing:
- Reading garbage data from the heap
- Potential crashes
- Security vulnerabilities

AddressSanitizer output:
```
==12345==ERROR: AddressSanitizer: heap-use-after-free on address 0x602000000010
READ of size 4 at 0x602000000010
    #0 on_building_deleted_callback building.c:156
    #1 building_delete building.c:98
```

## Relevant Code

Look at `src/building/building.c`:
- `building_delete()` function
- Callback notification code
- Order of operations in deletion

## Suggested Fix Approach

Notify callbacks before freeing the building:

```c
void building_delete(building *b)
{
    int building_id = b->id;

    // Notify callbacks FIRST while building is still valid
    for (int i = 0; i < num_callbacks; i++) {
        if (registered_callbacks[i].b == b) {
            registered_callbacks[i].callback(b, EVENT_BUILDING_DELETED);
            // Clear the registration
            registered_callbacks[i].b = NULL;
            registered_callbacks[i].callback = NULL;
        }
    }

    // Now safe to free the building
    free(b);

    // Clear from building list
    buildings[building_id] = NULL;
}
```

## Your Task

Fix the use-after-free by ensuring callbacks are notified before the building is freed. Your fix should:

1. Move callback notification before the free() call
2. Clear callback registrations for the deleted building
3. Ensure the building data is still valid when callbacks access it
4. Not change callback semantics for other events

Provide your fix as a unified diff (patch).
