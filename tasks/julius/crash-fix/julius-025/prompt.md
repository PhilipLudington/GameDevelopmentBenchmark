# Bug Report: Reservoir placement double-charge bug

## Summary

When placing a reservoir on water tiles (which is allowed and desirable), the building cost is charged twice - once during the water check and again during normal placement. This incorrectly drains player funds.

## Environment

- Project: Julius (Caesar III reimplementation)
- File: `src/building/construction.c`
- Severity: Medium (incorrect gameplay, fund loss)

## Bug Description

Reservoirs can be placed on water tiles, which provides better water supply. The placement code has special handling for water placement that charges the cost, but then the normal placement flow also charges the cost again.

```c
#define RESERVOIR_COST 500

int place_reservoir(int grid_offset, int funds)
{
    // Check if placing on water
    if (map_terrain_is_water(grid_offset)) {
        // BUG: Cost charged here
        if (funds < RESERVOIR_COST) {
            return 0;  // Not enough funds
        }
        city_finance_charge(RESERVOIR_COST);  // First charge

        set_water_reservoir_flag(grid_offset);
    }

    // Normal placement - cost charged again!
    if (!try_place_building(BUILDING_RESERVOIR, grid_offset)) {
        return 0;
    }

    city_finance_charge(RESERVOIR_COST);  // Second charge - BUG!

    return 1;
}
```

## Steps to Reproduce

1. Start a game with 1000 denarii
2. Place a reservoir on a water tile (cost: 500)
3. Observe that 1000 denarii was deducted instead of 500
4. Player now has 0 denarii instead of 500

## Expected Behavior

Placing a reservoir should only charge the cost once:
- On water: 500 denarii
- On land: 500 denarii
- Never 1000 denarii

## Current Behavior

Placing on water charges twice:
- First charge in water placement handling
- Second charge in normal placement
- Total: 1000 denarii for a 500 cost building

## Relevant Code

Look at `src/building/construction.c`:
- `place_reservoir()` function
- Water tile placement handling
- Fund deduction calls

## Suggested Fix Approach

Only charge once, either by:
1. Not charging in the water special case, or
2. Skipping the normal charge if already charged for water

```c
int place_reservoir(int grid_offset, int funds)
{
    if (map_terrain_is_water(grid_offset)) {
        if (funds < RESERVOIR_COST) {
            return 0;
        }
        // Don't charge here - let normal flow handle it
        set_water_reservoir_flag(grid_offset);
    }

    if (!try_place_building(BUILDING_RESERVOIR, grid_offset)) {
        return 0;
    }

    city_finance_charge(RESERVOIR_COST);  // Only charge point

    return 1;
}
```

## Your Task

Fix the double-charge bug by ensuring the reservoir cost is only deducted once. Your fix should:

1. Remove the duplicate charge
2. Still properly check if player has enough funds
3. Correctly handle both water and land placement
4. Not change the reservoir cost amount

Provide your fix as a unified diff (patch).
