# Bug Report: Enemy Formation Rank Overflow on Large Maps

## Summary

The enemy formation system uses a fixed-size array for tracking formation ranks. On very large maps with many enemy formations, the rank assignment can exceed the array bounds, causing memory corruption and unpredictable behavior.

## Environment

- Project: Julius (Caesar III reimplementation)
- File: `src/figure/formation.c`
- Severity: High (memory corruption, potential crash)

## Bug Description

Enemy formations are organized into ranks (rows) for combat. The rank data is stored in a fixed-size array:

```c
#define MAX_FORMATION_RANKS 16

typedef struct {
    int formation_id;
    int rank;
    int position;
} formation_rank_entry;

static formation_rank_entry rank_assignments[MAX_FORMATION_RANKS];
static int num_ranks = 0;

void assign_formation_rank(int formation_id, int rank)
{
    // BUG: No bounds check!
    rank_assignments[num_ranks].formation_id = formation_id;
    rank_assignments[num_ranks].rank = rank;
    rank_assignments[num_ranks].position = calculate_position(rank);
    num_ranks++;  // Can exceed MAX_FORMATION_RANKS!
}
```

When more than 16 formations try to get ranks, the array bounds are exceeded:
- Write to `rank_assignments[16]` is out of bounds
- Corrupts whatever memory follows the array
- Causes erratic behavior or crashes

## Steps to Reproduce

1. Load a very large map with aggressive enemy AI
2. Wait for multiple waves of enemies to spawn
3. When more than 16 formations exist, memory corruption occurs
4. Game may crash or exhibit strange formation behavior

## Expected Behavior

The function should check bounds before writing:
```c
void assign_formation_rank(int formation_id, int rank)
{
    if (num_ranks >= MAX_FORMATION_RANKS) {
        // Array full - cannot assign more ranks
        return;
    }

    rank_assignments[num_ranks].formation_id = formation_id;
    // ...
    num_ranks++;
}
```

## Current Behavior

No bounds check, allowing writes past the end of the array and corrupting adjacent memory.

## Relevant Code

Look at `src/figure/formation.c`:
- `assign_formation_rank()` function
- `MAX_FORMATION_RANKS` constant
- `rank_assignments` array
- `num_ranks` counter

## Suggested Fix Approach

Add bounds checking before array access:

```c
void assign_formation_rank(int formation_id, int rank)
{
    // FIXED: Check array bounds
    if (num_ranks >= MAX_FORMATION_RANKS) {
        log_warning("Formation rank array full");
        return;  // Cannot assign more
    }

    rank_assignments[num_ranks].formation_id = formation_id;
    rank_assignments[num_ranks].rank = rank;
    rank_assignments[num_ranks].position = calculate_position(rank);
    num_ranks++;
}
```

Consider also returning a status code to indicate success/failure.

## Your Task

Fix the bounds overflow by adding proper array bounds checking. Your fix should:

1. Check if the array is full before writing
2. Handle the "full" case gracefully (skip or log warning)
3. Prevent any writes past MAX_FORMATION_RANKS
4. Not change the function's behavior when space is available

Provide your fix as a unified diff (patch).
