# Bug Report: Stack overflow from deep recursion in building search

## Summary

The building search function uses unbounded recursion to find connected buildings. In large cities with many connected buildings, this causes a stack overflow and crashes the game.

## Environment

- Project: Julius (Caesar III reimplementation)
- File: `src/building/building_search.c`
- Severity: Critical (crash in large cities)

## Bug Description

The building search recursively explores connected buildings without limiting the recursion depth:

```c
void search_connected_buildings(int building_id, int *count, int *building_ids)
{
    building *b = building_get(building_id);
    if (!b || b->state != BUILDING_STATE_ACTIVE) {
        return;
    }

    if (is_already_visited(building_id)) {
        return;
    }

    mark_visited(building_id);
    building_ids[*count] = building_id;
    (*count)++;

    // BUG: Unbounded recursion - each connected building causes another stack frame
    for (int i = 0; i < b->num_connections; i++) {
        search_connected_buildings(b->connections[i], count, building_ids);
    }
}
```

With thousands of connected buildings, the stack overflows.

## Steps to Reproduce

1. Build a large city with 1000+ connected buildings (roads, warehouses, etc.)
2. Trigger a search for connected buildings (e.g., resource distribution)
3. Game crashes with stack overflow (SIGSEGV)

## Expected Behavior

Building search should:
1. Work correctly for any number of buildings
2. Use iterative approach or limit recursion depth
3. Not crash regardless of city size

## Current Behavior

Deep recursion causes stack overflow:
```
Stack overflow in search_connected_buildings()
Recursion depth: 10000+
```

## Relevant Code

Look at `src/building/building_search.c`:
- `search_connected_buildings()` function
- Recursive building exploration
- Connection traversal logic

## Suggested Fix Approach

Add a depth limit to prevent stack overflow:

```c
#define MAX_SEARCH_DEPTH 500

void search_connected_buildings_impl(int building_id, int *count,
                                      int *building_ids, int depth)
{
    if (depth >= MAX_SEARCH_DEPTH) {
        return;  // Prevent stack overflow
    }

    building *b = building_get(building_id);
    if (!b || b->state != BUILDING_STATE_ACTIVE) {
        return;
    }

    if (is_already_visited(building_id)) {
        return;
    }

    mark_visited(building_id);
    building_ids[*count] = building_id;
    (*count)++;

    for (int i = 0; i < b->num_connections; i++) {
        search_connected_buildings_impl(b->connections[i], count,
                                        building_ids, depth + 1);
    }
}

void search_connected_buildings(int building_id, int *count, int *building_ids)
{
    search_connected_buildings_impl(building_id, count, building_ids, 0);
}
```

## Your Task

Fix the stack overflow by limiting recursion depth. Your fix should:

1. Add a maximum depth parameter to limit recursion
2. Stop searching when depth limit reached
3. Still find buildings within the depth limit
4. Not change the function's external interface

Provide your fix as a unified diff (patch).
