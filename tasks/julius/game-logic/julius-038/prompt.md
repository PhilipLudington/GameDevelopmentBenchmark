# Bug Report: Granary Cart Pathfinding Loop

## Summary

Granary carts get stuck in an infinite pathfinding loop, continuously revisiting the same waypoints instead of reaching their destination. The bug is a variable mix-up where the arrival check compares the current position to the source (start) position instead of the destination.

## Environment

- Project: Julius (Caesar III reimplementation)
- File: `src/figure/route.c`
- Severity: High (gameplay breaking, carts never deliver goods)

## Bug Description

When a granary cart follows a route, it checks at each waypoint whether it has arrived at the destination. However, the code compares the current position to `source_x/source_y` instead of `dest_x/dest_y`:

```c
int figure_route_follow(figure *f)
{
    int source_x = f->route_start_x;
    int source_y = f->route_start_y;
    int dest_x = f->destination_x;
    int dest_y = f->destination_y;

    // Move to next waypoint
    route_waypoint *wp = get_next_waypoint(f);
    f->x = wp->x;
    f->y = wp->y;

    // BUG: Checking against SOURCE instead of DESTINATION!
    if (f->x == source_x && f->y == source_y) {
        return ROUTE_ARRIVED;
    }

    return ROUTE_CONTINUE;
}
```

This means:
1. Cart starts at source, heads toward destination
2. When cart arrives at destination, check fails (not at source)
3. Cart continues following route, which loops back
4. Cart eventually returns to source
5. Now the check passes, but cart is at wrong location!
6. Or route loops forever if it doesn't return to exact source

## Steps to Reproduce

1. Build a granary and a farm at different locations
2. Wait for a cart to spawn to collect goods
3. Observe the cart path - it reaches the farm but doesn't stop
4. Cart continues circling or gets stuck

## Expected Behavior

The arrival check should compare against the destination:
```c
if (f->x == dest_x && f->y == dest_y) {
    return ROUTE_ARRIVED;
}
```

## Current Behavior

Cart never registers arrival at destination because it's checking for arrival at the starting point.

## Relevant Code

Look at `src/figure/route.c`:
- `figure_route_follow()` function
- Variables `source_x/y` vs `dest_x/y`
- The arrival condition check

## Suggested Fix Approach

Change the comparison to use destination coordinates:

```c
int figure_route_follow(figure *f)
{
    int source_x = f->route_start_x;
    int source_y = f->route_start_y;
    int dest_x = f->destination_x;
    int dest_y = f->destination_y;

    route_waypoint *wp = get_next_waypoint(f);
    f->x = wp->x;
    f->y = wp->y;

    // FIXED: Check against DESTINATION, not source
    if (f->x == dest_x && f->y == dest_y) {
        return ROUTE_ARRIVED;
    }

    return ROUTE_CONTINUE;
}
```

## Your Task

Fix the destination check by comparing against the correct coordinates. Your fix should:

1. Change the comparison from source to destination coordinates
2. Ensure carts stop when they reach their destination
3. Not affect the source coordinate usage elsewhere
4. Handle edge cases where source equals destination

Provide your fix as a unified diff (patch).
