# Bug Report: Building Cost Refund Calculation Error

## Summary

When a building is demolished, the refund calculation uses the base building cost instead of the actual cost paid. Since construction on difficult terrain (marshes, hills) costs extra, players lose money when demolishing buildings built on such terrain.

## Environment

- Project: Julius (Caesar III reimplementation)
- File: `src/building/construction.c`
- Severity: Medium (economy exploit/unfairness)

## Bug Description

Building construction applies terrain modifiers to the base cost:
- Marsh: +50% cost
- Hill: +25% cost
- Normal: base cost

When placing a building, the player is charged the modified cost. But when demolishing, the refund uses only the base cost:

```c
int calculate_construction_cost(int building_type, int terrain_type)
{
    int base_cost = get_base_building_cost(building_type);

    if (terrain_type == TERRAIN_MARSH) {
        return base_cost + (base_cost / 2);  // +50%
    } else if (terrain_type == TERRAIN_HILL) {
        return base_cost + (base_cost / 4);  // +25%
    }
    return base_cost;
}

int calculate_demolition_refund(building *b)
{
    // BUG: Uses base cost, not what was actually paid!
    int base_cost = get_base_building_cost(b->type);
    return base_cost / 2;  // 50% refund of BASE cost
}
```

Example with a 1000 denarii building on marsh:
- Construction cost: `1000 + 500 = 1500` (charged to player)
- Demolition refund: `1000 / 2 = 500` (based on base cost)
- Player loses: `1500 - 500 = 1000` instead of `1500 - 750 = 750`

## Steps to Reproduce

1. Build a reservoir on normal terrain (cost: 500, for example)
2. Demolish it (refund: 250)
3. Build same reservoir on marsh (cost: 750)
4. Demolish it (refund: 250 - same as normal!)
5. Player paid 250 more but gets same refund

## Expected Behavior

The refund should be based on the original construction cost, which is stored in the building data:
```c
int calculate_demolition_refund(building *b)
{
    // Use the actual cost paid, stored in building
    return b->construction_cost / 2;  // 50% of what was paid
}
```

## Current Behavior

Refund is calculated from base cost, ignoring terrain modifiers that were charged during construction.

## Relevant Code

Look at `src/building/construction.c`:
- `calculate_construction_cost()` - applies terrain modifiers (correct)
- `calculate_demolition_refund()` - ignores modifiers (buggy)
- `building` struct - should have `construction_cost` field

Also check:
- Where construction cost is stored when building is placed
- `get_base_building_cost()` function

## Suggested Fix Approach

Use the stored construction cost instead of recalculating base:

```c
int calculate_demolition_refund(building *b)
{
    // FIXED: Use the actual construction cost that was paid
    return b->construction_cost / 2;
}
```

Make sure the construction cost is stored when the building is created:
```c
void place_building(building *b, int type, int x, int y)
{
    // ... setup building ...
    int terrain = map_get_terrain(x, y);
    b->construction_cost = calculate_construction_cost(type, terrain);
    // charge player b->construction_cost
}
```

## Your Task

Fix the refund calculation to use the actual construction cost. Your fix should:

1. Use the stored construction cost, not the base cost
2. Return 50% of what was actually paid
3. Handle buildings that don't have cost stored (use base as fallback)
4. Ensure consistency between charge and refund

Provide your fix as a unified diff (patch).
