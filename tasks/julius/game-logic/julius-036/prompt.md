# Bug Report: Plague Spread Rate Integer Math Error

## Summary

The plague spread calculation has an integer overflow bug where multiplying large population densities by the spread factor overflows a 32-bit integer before the bounds check is applied, resulting in negative or wildly incorrect spread rates.

## Environment

- Project: Julius (Caesar III reimplementation)
- File: `src/city/health.c`
- Severity: High (game simulation broken, plague spreads incorrectly)

## Bug Description

When calculating how fast a plague spreads through the city, the code multiplies population density by a spread factor. On large, densely populated maps, this multiplication can overflow:

```c
int calculate_plague_spread_rate(int population_density, int base_spread_factor)
{
    // BUG: This can overflow for large densities!
    int spread_rate = population_density * base_spread_factor;

    // Bounds check happens AFTER overflow has already occurred
    if (spread_rate > MAX_SPREAD_RATE) {
        spread_rate = MAX_SPREAD_RATE;
    }
    if (spread_rate < MIN_SPREAD_RATE) {
        spread_rate = MIN_SPREAD_RATE;
    }

    return spread_rate;
}
```

With `population_density = 50000` and `base_spread_factor = 100000`:
- Expected: `5,000,000,000` (capped to MAX_SPREAD_RATE)
- Actual: Overflow! `50000 * 100000 = 705032704` (wrong due to 32-bit overflow wrap)

Or even worse, with certain values the result becomes negative, bypassing the MAX check entirely.

## Steps to Reproduce

1. Start a large map with high population
2. Trigger a plague event
3. Observe that plague spreads at an incorrect rate
4. On some maps, plague barely spreads; on others, it spreads too fast

## Expected Behavior

The calculation should:
1. Detect potential overflow before multiplying
2. Cap the result to MAX_SPREAD_RATE if overflow would occur
3. Return predictable, bounded results for all inputs

## Current Behavior

- Multiplication overflows silently
- Result wraps around to unexpected values
- Bounds check is ineffective because damage already done

## Relevant Code

Look at `src/city/health.c`:
- `calculate_plague_spread_rate()` function
- `update_plague_status()` that calls it
- MAX_SPREAD_RATE and MIN_SPREAD_RATE constants

## Suggested Fix Approach

Check for potential overflow before multiplying:

```c
int calculate_plague_spread_rate(int population_density, int base_spread_factor)
{
    // Check for overflow before multiplying
    if (population_density > 0 && base_spread_factor > 0) {
        if (population_density > MAX_SPREAD_RATE / base_spread_factor) {
            return MAX_SPREAD_RATE;  // Would overflow, cap early
        }
    }

    int spread_rate = population_density * base_spread_factor;

    if (spread_rate > MAX_SPREAD_RATE) {
        spread_rate = MAX_SPREAD_RATE;
    }
    if (spread_rate < MIN_SPREAD_RATE) {
        spread_rate = MIN_SPREAD_RATE;
    }

    return spread_rate;
}
```

Or use a wider type for intermediate calculation:
```c
    long long spread_rate_wide = (long long)population_density * base_spread_factor;
    if (spread_rate_wide > MAX_SPREAD_RATE) {
        return MAX_SPREAD_RATE;
    }
    return (int)spread_rate_wide;
```

## Your Task

Fix the integer overflow by implementing overflow-safe multiplication. Your fix should:

1. Prevent integer overflow from occurring
2. Return MAX_SPREAD_RATE when multiplication would overflow
3. Handle edge cases (zero, negative values)
4. Maintain correct behavior for normal (non-overflowing) inputs

Provide your fix as a unified diff (patch).
