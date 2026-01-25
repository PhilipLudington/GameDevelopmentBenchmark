# Bug Report: Waypoint Deletion Corrupts Route Data

## Summary

When a waypoint is deleted from the middle of a route, the remaining waypoints are not properly shifted to compact the array. This leaves stale waypoint data that causes figures to teleport to old positions or follow corrupted paths.

## Environment

- Project: Julius (Caesar III reimplementation)
- File: `src/figure/route.c`
- Severity: High (figures teleport, routes corrupted, potential crash)

## Bug Description

Routes are stored as arrays of waypoints. When a waypoint is deleted (e.g., a road segment is removed), the deletion code marks the waypoint as invalid but doesn't shift subsequent waypoints:

```c
void route_delete_waypoint(route_data *route, int index)
{
    if (index < 0 || index >= route->num_waypoints) {
        return;  // Invalid index
    }

    // BUG: Just decrements count without moving data!
    route->num_waypoints--;

    // BUG: Old data still in array!
    // Waypoints after 'index' are not shifted down
    // Element at 'index' still contains old waypoint
}
```

Example:
- Route has waypoints: [A, B, C, D, E] (count=5)
- Delete waypoint at index 2 (C)
- After buggy delete: [A, B, C, D, E] with count=4
- Now waypoint[2] is still C (should be D), waypoint[3] is D (should be E)
- Figure follows wrong path: A -> B -> C (deleted!) -> D

## Steps to Reproduce

1. Create a patrol route with multiple waypoints
2. Delete a road segment that removes a middle waypoint
3. Watch figures follow the route
4. Figures teleport or visit wrong locations

## Expected Behavior

After deleting waypoint at index i:
1. Shift all waypoints after index i down by one
2. Waypoint[i] should contain what was in waypoint[i+1]
3. Decrement the count
4. Clear the last slot (optional but good practice)

## Current Behavior

- Count is decremented
- Array data is unchanged
- Stale waypoint at deleted index causes path corruption

## Relevant Code

Look at `src/figure/route.c`:
- `route_delete_waypoint()` function
- `route_insert_waypoint()` for reference on array manipulation
- Waypoint array structure

## Suggested Fix Approach

Shift the array elements after deletion:

```c
void route_delete_waypoint(route_data *route, int index)
{
    if (index < 0 || index >= route->num_waypoints) {
        return;
    }

    // FIXED: Shift all waypoints after index down by one
    for (int i = index; i < route->num_waypoints - 1; i++) {
        route->waypoints[i] = route->waypoints[i + 1];
    }

    route->num_waypoints--;

    // Clear the now-unused last slot
    memset(&route->waypoints[route->num_waypoints], 0, sizeof(waypoint));
}
```

Or using memmove for efficiency:
```c
    int elements_to_move = route->num_waypoints - index - 1;
    if (elements_to_move > 0) {
        memmove(&route->waypoints[index],
                &route->waypoints[index + 1],
                elements_to_move * sizeof(waypoint));
    }
    route->num_waypoints--;
```

## Your Task

Fix the waypoint deletion to properly compact the array. Your fix should:

1. Shift all waypoints after the deleted index down by one position
2. Properly update the waypoint count
3. Handle edge cases (deleting first, last, only waypoint)
4. Not leave stale data accessible

Provide your fix as a unified diff (patch).
