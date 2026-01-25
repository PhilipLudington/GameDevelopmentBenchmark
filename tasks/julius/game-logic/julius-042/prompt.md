# Bug Report: Trade Caravan Follower Array Index Mismatch

## Summary

Trade caravans have followers (pack animals carrying goods). An off-by-one error causes the update loop to start at index 1 instead of 0, meaning the first follower is never updated and becomes "stuck" while the rest of the caravan moves.

## Environment

- Project: Julius (Caesar III reimplementation)
- File: `src/figure/trader.c`
- Severity: Medium (visual glitch, one follower gets left behind)

## Bug Description

Caravan followers are stored in a 0-indexed array. When creating followers, they're correctly added starting at index 0. But when updating followers, the loop incorrectly starts at 1:

```c
#define MAX_CARAVAN_FOLLOWERS 4

typedef struct {
    int figure_id;
    int x, y;
    int carrying_resource;
} caravan_follower;

static caravan_follower followers[MAX_CARAVAN_FOLLOWERS];
static int num_followers = 0;

void add_follower(int figure_id)
{
    if (num_followers >= MAX_CARAVAN_FOLLOWERS) return;
    followers[num_followers].figure_id = figure_id;  // Starts at index 0
    num_followers++;
}

void update_all_followers(void)
{
    // BUG: Loop starts at 1, skipping followers[0]!
    for (int i = 1; i < num_followers; i++) {
        update_follower_position(&followers[i]);
    }
}
```

The first follower (index 0) is never updated, so it stays at its initial position while the caravan moves away.

## Steps to Reproduce

1. Wait for a trade caravan to arrive
2. Observe the pack animals following the trader
3. The first pack animal stays at the entry point
4. Other pack animals follow correctly

## Expected Behavior

Loop should start at 0 to update all followers:
```c
for (int i = 0; i < num_followers; i++) {
```

## Current Behavior

Loop starts at 1, skipping the first follower.

## Relevant Code

Look at `src/figure/trader.c`:
- `add_follower()` function (correctly uses index 0)
- `update_all_followers()` function (incorrectly starts at 1)
- Any other loops iterating over followers

## Suggested Fix Approach

Change the loop to start at 0:

```c
void update_all_followers(void)
{
    // FIXED: Start at 0 to include first follower
    for (int i = 0; i < num_followers; i++) {
        update_follower_position(&followers[i]);
    }
}
```

## Your Task

Fix the off-by-one error by correcting the loop start index. Your fix should:

1. Start the loop at index 0
2. Update all followers including the first one
3. Not change the loop termination condition
4. Not affect other parts of the follower system

Provide your fix as a unified diff (patch).
