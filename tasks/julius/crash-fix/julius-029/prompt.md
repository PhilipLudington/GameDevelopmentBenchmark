# Bug Report: Infinite loop in pathfinding when destination unreachable

## Summary

The pathfinding algorithm enters an infinite loop when attempting to find a path to an unreachable destination (e.g., an island or walled-off area). This causes the game to freeze.

## Environment

- Project: Julius (Caesar III reimplementation)
- File: `src/figure/route.c`
- Severity: Critical (game freeze)

## Bug Description

The pathfinding loop continues searching indefinitely when no path exists to the destination. There's no maximum iteration limit or check for when all reachable nodes have been explored.

```c
#define MAX_PATH_LENGTH 500

int find_path(int start_x, int start_y, int dest_x, int dest_y, path_result *result)
{
    init_path_search(start_x, start_y);

    // BUG: No limit on iterations - loops forever if destination unreachable
    while (!path_found()) {
        int next = get_next_search_node();
        if (next < 0) {
            continue;  // BUG: Should return failure, not continue
        }

        explore_neighbors(next);

        if (reached_destination(dest_x, dest_y)) {
            build_path(result);
            return 1;
        }
    }

    return 0;  // Never reached if destination unreachable
}
```

## Steps to Reproduce

1. Create a city with an isolated area (e.g., island or walled section)
2. Have a figure try to reach a building in the isolated area
3. Game freezes as pathfinding loops forever
4. Must force-quit the game

## Expected Behavior

When destination is unreachable:
1. Pathfinding should detect no path exists
2. Return failure after reasonable search
3. Figure should give up or choose alternative behavior
4. Game should not freeze

## Current Behavior

Pathfinding loops indefinitely:
- No maximum iteration count
- `get_next_search_node()` returning -1 causes `continue` instead of exit
- CPU usage spikes to 100% on one core
- Game becomes unresponsive

## Relevant Code

Look at `src/figure/route.c`:
- `find_path()` function
- Main pathfinding loop
- Termination conditions

## Suggested Fix Approach

Add iteration limit and proper termination:

```c
#define MAX_PATH_LENGTH 500
#define MAX_SEARCH_ITERATIONS 10000

int find_path(int start_x, int start_y, int dest_x, int dest_y, path_result *result)
{
    init_path_search(start_x, start_y);

    int iterations = 0;

    while (!path_found() && iterations < MAX_SEARCH_ITERATIONS) {
        iterations++;

        int next = get_next_search_node();
        if (next < 0) {
            // No more nodes to explore - destination unreachable
            return 0;
        }

        explore_neighbors(next);

        if (reached_destination(dest_x, dest_y)) {
            build_path(result);
            return 1;
        }
    }

    return 0;  // Path not found within iteration limit
}
```

## Your Task

Fix the infinite loop by adding proper termination conditions. Your fix should:

1. Add a maximum iteration limit
2. Return failure when no more nodes to explore
3. Handle unreachable destinations gracefully
4. Not affect valid pathfinding behavior

Provide your fix as a unified diff (patch).
