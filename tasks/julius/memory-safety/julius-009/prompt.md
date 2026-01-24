# Bug Report: Null Pointer Dereference in Building Lookup

## Summary

The building lookup function can return NULL for deleted or invalid building IDs, but callers don't check for NULL before accessing building fields, causing crashes.

## Environment

- Project: Julius (Caesar III reimplementation)
- File: `src/building/building.c`
- Severity: High (crash, denial of service)

## Bug Description

The `building_get()` function retrieves a building by its ID from an array:

```c
building *building_get(int building_id)
{
    if (building_id <= 0 || building_id >= MAX_BUILDINGS) {
        return NULL;  // Invalid ID
    }
    building *b = &all_buildings[building_id];
    if (b->state == BUILDING_STATE_DELETED) {
        return NULL;  // Deleted building
    }
    return b;
}
```

However, many callers assume building_get() always returns a valid pointer:

```c
void process_building_worker(int building_id)
{
    building *b = building_get(building_id);
    // BUG: No NULL check!
    b->num_workers++;  // CRASH if building_get returned NULL
    update_building_state(b->type);  // CRASH
}
```

This can crash when:
1. A building is demolished while workers are en route
2. Save games have stale building references
3. Invalid IDs are passed due to other bugs

## Steps to Reproduce

1. Place a building in the game
2. Send a worker to the building
3. Delete the building before the worker arrives
4. When the worker tries to process the (now deleted) building, crash occurs

Or programmatically:
```c
int invalid_id = 9999;
building *b = building_get(invalid_id);
b->num_workers++;  // SEGV - null pointer dereference
```

## Expected Behavior

Functions should check if building_get() returns NULL and handle the case gracefully (skip processing, log warning, etc.).

## Current Behavior

Direct dereference of NULL pointer causes SIGSEGV crash:
```
ASAN:DEADLYSIGNAL
==12345==ERROR: AddressSanitizer: SEGV on unknown address 0x000000000000
```

## Relevant Code

Look at `src/building/building.c`, specifically:
- `building_get()` function (returns NULL in some cases)
- `process_building_worker()` and similar functions that call building_get()
- Any code that accesses building fields after lookup

## Suggested Fix Approach

Add NULL checks after every call to building_get():

```c
void process_building_worker(int building_id)
{
    building *b = building_get(building_id);
    if (!b) {
        // Building was deleted or invalid - skip processing
        return;
    }
    b->num_workers++;
    update_building_state(b->type);
}
```

Or create a safe accessor function that returns a default/dummy building on NULL.

## Your Task

Fix the null pointer dereference by adding proper NULL checks. Your fix should:
1. Check for NULL return from building_get() before accessing fields
2. Handle the NULL case gracefully (skip, return early, or log warning)
3. Ensure no crash occurs with deleted or invalid building IDs
4. Not change the behavior for valid buildings

Provide your fix as a unified diff (patch).
