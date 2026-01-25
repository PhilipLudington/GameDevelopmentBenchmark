# Bug Report: Water building data corruption on collapse

## Summary

When a water building (reservoir or fountain) collapses due to fire or earthquake, the water network connection data is not properly cleared, leaving "ghost" connections that cause visual glitches and eventual crashes when the network is recalculated.

## Environment

- Project: Julius (Caesar III reimplementation)
- File: `src/building/building_state.c`
- Severity: High (game state corruption, eventual crash)

## Bug Description

Water buildings maintain connection data to track pipes and water flow. When a water building collapses, the building is removed but its water network data remains in the global network grid. This orphaned data causes:

1. Visual glitches showing water where there shouldn't be
2. New buildings incorrectly thinking they have water
3. Crash when recalculating water network with invalid building references

```c
void building_collapse(building *b)
{
    // Remove building from map
    map_building_clear(b->grid_offset);

    // Handle special building types
    if (b->type == BUILDING_HOUSE) {
        handle_house_collapse(b);
    }

    // BUG: Water buildings not handled - network data not cleared
    // Missing:
    // if (building_is_water_type(b->type)) {
    //     water_network_clear_building(b);
    // }

    // Mark building as deleted
    b->state = BUILDING_STATE_DELETED;
}
```

## Steps to Reproduce

1. Build a reservoir connected to fountains
2. Start a fire or trigger earthquake on the reservoir
3. Reservoir collapses
4. Water network shows ghost connections
5. Eventually crash when game recalculates water

## Expected Behavior

When a water building collapses:
1. Clear its water network connections
2. Update neighboring buildings' water status
3. Recalculate water network to remove invalid references

## Current Behavior

Water network data persists after building collapse:
- Map tiles show water access incorrectly
- Building references in network become invalid
- Crash on next network recalculation

## Relevant Code

Look at `src/building/building_state.c`:
- `building_collapse()` function
- Check for water building type handling
- Look for `water_network_clear_building()` or similar

Also check:
- `src/map/water.c` for water network functions
- Building type definitions for water buildings

## Suggested Fix Approach

Add water building handling to collapse:

```c
void building_collapse(building *b)
{
    map_building_clear(b->grid_offset);

    if (b->type == BUILDING_HOUSE) {
        handle_house_collapse(b);
    }

    // ADDED: Clear water network data for water buildings
    if (building_is_water_type(b->type)) {
        water_network_clear_building(b);
        water_network_recalculate();
    }

    b->state = BUILDING_STATE_DELETED;
}
```

## Your Task

Fix the water building collapse bug by ensuring water network data is properly cleared. Your fix should:

1. Detect when a water building is collapsing
2. Clear the building's water network connections
3. Trigger a water network recalculation if needed
4. Not affect non-water building collapse behavior

Provide your fix as a unified diff (patch).
