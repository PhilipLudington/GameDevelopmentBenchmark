# Bug Report: Popularity Decay Rounding Error

## Summary

The popularity decay calculation loses precision due to performing integer division before multiplication. This causes the decay rate to be lower than intended, making it easier to maintain high popularity than the game designers intended.

## Environment

- Project: Julius (Caesar III reimplementation)
- File: `src/city/sentiment.c`
- Severity: Medium (gameplay balance issue, simulation accuracy)

## Bug Description

Popularity in Caesar III decays over time based on various factors. The decay formula should be:

```
decay = (base_decay * modifier * time_factor) / SCALE_FACTOR
```

However, the code performs division before multiplication:

```c
int calculate_popularity_decay(int current_popularity, int base_decay_rate)
{
    int modifier = get_difficulty_modifier();  // e.g., 3
    int time_factor = get_time_factor();       // e.g., 2

    // BUG: Division happens first due to order of operations
    // (base_decay_rate / SCALE_FACTOR) truncates to 0 for small values!
    int decay = base_decay_rate / SCALE_FACTOR * modifier * time_factor;

    return current_popularity - decay;
}
```

With `SCALE_FACTOR = 100`, `base_decay_rate = 50`:
- **Buggy**: `50 / 100 * 3 * 2` = `0 * 3 * 2` = **0** (no decay!)
- **Correct**: `50 * 3 * 2 / 100` = `300 / 100` = **3**

## Steps to Reproduce

1. Start a new game with low popularity factors
2. Wait for popularity decay to occur
3. Observe that popularity decays much slower than expected
4. Over many game ticks, the error accumulates significantly

## Expected Behavior

The calculation should multiply first, then divide:
```c
int decay = base_decay_rate * modifier * time_factor / SCALE_FACTOR;
```

This preserves precision until the final division.

## Current Behavior

Integer division truncates intermediate results to zero for small decay rates, causing:
- No decay when rate is less than SCALE_FACTOR
- Reduced decay for all values due to truncation

## Relevant Code

Look at `src/city/sentiment.c`:
- `calculate_popularity_decay()` function
- Order of operations in the decay formula
- SCALE_FACTOR constant usage

## Suggested Fix Approach

Reorder the operations to multiply before dividing:

```c
int calculate_popularity_decay(int current_popularity, int base_decay_rate)
{
    int modifier = get_difficulty_modifier();
    int time_factor = get_time_factor();

    // FIXED: Multiply first to preserve precision, then divide
    int decay = (base_decay_rate * modifier * time_factor) / SCALE_FACTOR;

    return current_popularity - decay;
}
```

Be careful about potential integer overflow for large values - may need to use long long for intermediate calculation.

## Your Task

Fix the precision loss by reordering the arithmetic operations. Your fix should:

1. Perform multiplications before division
2. Ensure no integer overflow (use appropriate types if needed)
3. Produce correct decay values for all valid inputs
4. Not change the function signature or expected output range

Provide your fix as a unified diff (patch).
