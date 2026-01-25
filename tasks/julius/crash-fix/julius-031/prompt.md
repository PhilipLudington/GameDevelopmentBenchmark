# Bug Report: Assert failure when building state invalid in debug mode

## Summary

In debug builds, the game crashes with an assertion failure when processing a building with an invalid state value. This can happen due to save file corruption or memory issues.

## Environment

- Project: Julius (Caesar III reimplementation)
- File: `src/building/building.c`
- Severity: Medium (debug build crash, potential issue in release)

## Bug Description

The building update function uses an assert to validate building state, which causes an immediate crash when the state is invalid:

```c
#define BUILDING_STATE_NONE 0
#define BUILDING_STATE_ACTIVE 1
#define BUILDING_STATE_DELETED 2
#define BUILDING_STATE_RUBBLE 3
#define BUILDING_STATE_MAX 4

void update_building(building *b)
{
    // BUG: Assert crashes in debug mode with invalid state
    assert(b->state >= BUILDING_STATE_NONE && b->state < BUILDING_STATE_MAX);

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
```

## Steps to Reproduce

1. Compile game in debug mode (with assertions enabled)
2. Load a corrupted save file OR trigger memory corruption
3. Building with state value 99 (or other invalid value) gets processed
4. Assert fails, game crashes

## Expected Behavior

Invalid building state should be:
1. Detected and logged
2. Handled gracefully (skip or fix the building)
3. Not crash the game
4. Optionally warn the user about data corruption

## Current Behavior

Assert failure crashes immediately:
```
Assertion failed: b->state >= BUILDING_STATE_NONE && b->state < BUILDING_STATE_MAX
Aborted (core dumped)
```

## Relevant Code

Look at `src/building/building.c`:
- `update_building()` function
- State validation logic
- Building state constants

## Suggested Fix Approach

Replace assert with runtime check and graceful handling:

```c
void update_building(building *b)
{
    // FIXED: Check and handle invalid state gracefully
    if (b->state < BUILDING_STATE_NONE || b->state >= BUILDING_STATE_MAX) {
        log_error("Invalid building state", b->id, b->state);
        b->state = BUILDING_STATE_DELETED;  // Fix the invalid state
        return;
    }

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
```

## Your Task

Fix the assert failure by adding proper validation. Your fix should:

1. Replace the assert with a runtime check
2. Handle invalid states gracefully
3. Log the error for debugging
4. Recover by setting a valid state
5. Not crash the game

Provide your fix as a unified diff (patch).
