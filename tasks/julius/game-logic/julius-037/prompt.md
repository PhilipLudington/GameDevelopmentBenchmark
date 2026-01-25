# Bug Report: Tower Archer Range Ignores Elevation

## Summary

Tower archers calculate firing range using only 2D distance (X and Y coordinates), completely ignoring the elevation (Z) difference between the tower and the target. This makes towers on high ground less effective than they should be, and allows enemies in valleys to be hit from much farther away than intended.

## Environment

- Project: Julius (Caesar III reimplementation)
- File: `src/figure/combat.c`
- Severity: Medium (combat balance issue, unrealistic behavior)

## Bug Description

The range calculation for tower archers uses a 2D distance formula:

```c
int calculate_archer_range(int tower_x, int tower_y, int tower_z,
                           int target_x, int target_y, int target_z)
{
    int dx = target_x - tower_x;
    int dy = target_y - tower_y;
    // BUG: dz is calculated but never used!
    int dz = target_z - tower_z;

    // Only 2D distance - ignores elevation
    int distance_squared = dx * dx + dy * dy;

    return int_sqrt(distance_squared);
}
```

The elevation difference (`dz`) is calculated but never included in the distance formula.

## Steps to Reproduce

1. Build a tower on high ground (elevation 5)
2. Place an enemy directly below on low ground (elevation 0)
3. The tower reports the enemy is within range
4. In reality, the enemy is much farther due to vertical distance

Example:
- Tower at (10, 10, 5), Target at (10, 10, 0)
- Buggy 2D distance: sqrt(0 + 0) = 0 (point blank!)
- Correct 3D distance: sqrt(0 + 0 + 25) = 5 (actual distance)

## Expected Behavior

The range calculation should use 3D Euclidean distance:
```
distance = sqrt(dx^2 + dy^2 + dz^2)
```

This accounts for elevation and provides realistic range behavior.

## Current Behavior

Only 2D distance is calculated, making elevation irrelevant to range:
- Towers on cliffs can hit targets directly below as if they were adjacent
- Archers shooting uphill have the same range as shooting downhill

## Relevant Code

Look at `src/figure/combat.c`:
- `calculate_archer_range()` function
- `tower_can_fire_at()` function
- Range comparison logic

## Suggested Fix Approach

Include the elevation component in the distance calculation:

```c
int calculate_archer_range(int tower_x, int tower_y, int tower_z,
                           int target_x, int target_y, int target_z)
{
    int dx = target_x - tower_x;
    int dy = target_y - tower_y;
    int dz = target_z - tower_z;

    // FIXED: Include elevation in distance calculation
    int distance_squared = dx * dx + dy * dy + dz * dz;

    return int_sqrt(distance_squared);
}
```

## Your Task

Fix the range calculation to account for elevation difference. Your fix should:

1. Include the dz component in the distance calculation
2. Use proper 3D Euclidean distance formula
3. Handle negative elevation differences correctly (squared value is positive)
4. Not change the function signature

Provide your fix as a unified diff (patch).
