# Bug Report: Gatehouse infinite road respawn bug

## Summary

When demolishing a gatehouse, the road tiles underneath keep getting recreated infinitely. The demolition triggers road restoration which triggers gatehouse check which triggers demolition again, creating an infinite loop that freezes the game.

## Environment

- Project: Julius (Caesar III reimplementation)
- File: `src/building/demolition.c`
- Severity: High (game freeze, infinite loop)

## Bug Description

Gatehouses are special buildings that span road tiles. When demolished, the code attempts to restore the road tiles that were under the gatehouse. However, the road restoration code checks for gatehouse placement requirements and triggers a rebuild, which then gets demolished again.

```c
void demolish_building(building *b)
{
    int grid_offset = b->grid_offset;

    // Remove building
    clear_building_from_map(b);
    b->state = BUILDING_STATE_DELETED;

    // Special handling for gatehouse
    if (b->type == BUILDING_GATEHOUSE) {
        // BUG: This restores roads, which triggers gatehouse rebuild logic
        restore_road_tiles(grid_offset, b->size);
    }

    // Check for road network updates
    update_road_network(grid_offset);  // This can recreate gatehouse
}
```

## Steps to Reproduce

1. Build a gatehouse on a road
2. Demolish the gatehouse
3. Game freezes as road keeps being created/demolished
4. Eventually crashes or requires force quit

## Expected Behavior

When a gatehouse is demolished:
1. Building is removed
2. Road tiles are restored once
3. Road network is updated without triggering gatehouse recreation
4. No infinite loop occurs

## Current Behavior

Demolition creates an infinite loop:
1. Demolish gatehouse
2. Restore road tiles
3. Road network update sees road pattern
4. Gatehouse auto-rebuild triggered
5. Gatehouse demolish called again
6. Loop repeats infinitely

## Relevant Code

Look at `src/building/demolition.c`:
- `demolish_building()` function
- Road restoration logic
- Road network update callbacks

Also check:
- `src/map/road_network.c` for network update functions
- Auto-build or gatehouse creation triggers

## Suggested Fix Approach

Add a flag to prevent recursive demolition/rebuild:

```c
static int demolition_in_progress = 0;

void demolish_building(building *b)
{
    if (demolition_in_progress) {
        return;  // Prevent recursive calls
    }

    demolition_in_progress = 1;

    int grid_offset = b->grid_offset;
    clear_building_from_map(b);
    b->state = BUILDING_STATE_DELETED;

    if (b->type == BUILDING_GATEHOUSE) {
        restore_road_tiles(grid_offset, b->size);
    }

    update_road_network(grid_offset);

    demolition_in_progress = 0;  // Clear flag when done
}
```

## Your Task

Fix the infinite road respawn bug by preventing the recursive demolition/rebuild cycle. Your fix should:

1. Prevent infinite loop during gatehouse demolition
2. Still properly restore road tiles
3. Allow normal road network updates
4. Not affect other building types

Provide your fix as a unified diff (patch).
