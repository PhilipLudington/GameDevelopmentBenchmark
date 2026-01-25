# Bug Report: Division by zero in population ratio calculation

## Summary

The game crashes with a floating point exception when calculating the worker-to-population ratio if the total population is zero. This occurs at game start or when all citizens leave the city.

## Environment

- Project: Julius (Caesar III reimplementation)
- File: `src/city/population.c`
- Severity: Critical (crash)

## Bug Description

The population ratio calculation divides by total population without checking if it's zero:

```c
int calculate_worker_ratio(void)
{
    int workers = city_data.population.working;
    int total = city_data.population.total;

    // BUG: No check for zero total population
    int ratio = (workers * 100) / total;

    return ratio;
}
```

When `total` is 0, this causes a division by zero crash.

## Steps to Reproduce

1. Start a new game (population = 0)
2. Open the population advisor before any immigrants arrive
3. Game crashes with SIGFPE (floating point exception)

Alternative:
1. Have a city with population
2. Cause all citizens to leave (fire, plague, etc.)
3. Open population statistics
4. Crash

## Expected Behavior

When population is zero:
- Return 0 for the ratio (or another sensible default)
- Do not crash
- Display appropriate message like "No population"

## Current Behavior

Division by zero causes immediate crash:
```
Signal: SIGFPE (Floating point exception)
```

## Relevant Code

Look at `src/city/population.c`:
- `calculate_worker_ratio()` function
- Any other division operations using population

Similar issues might exist in:
- Employment calculations
- Tax calculations
- Census reports

## Suggested Fix Approach

Add a zero check before division:

```c
int calculate_worker_ratio(void)
{
    int workers = city_data.population.working;
    int total = city_data.population.total;

    // FIXED: Check for zero population
    if (total == 0) {
        return 0;
    }

    int ratio = (workers * 100) / total;

    return ratio;
}
```

## Your Task

Fix the division by zero crash by adding proper validation. Your fix should:

1. Check if population is zero before dividing
2. Return a sensible default value (0) when population is zero
3. Not change the calculation when population is non-zero
4. Be defensive against similar issues

Provide your fix as a unified diff (patch).
