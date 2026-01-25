# Bug Report: Division by zero in resource ratio calculation

## Summary

The game crashes when calculating the resource consumption ratio if the maximum consumption capacity is zero. This happens when no buildings that consume a resource are built.

## Environment

- Project: Julius (Caesar III reimplementation)
- File: `src/city/resource.c`
- Severity: Critical (crash)

## Bug Description

The resource ratio calculation divides current consumption by maximum capacity without checking for zero:

```c
int get_resource_consumption_percent(int resource_type)
{
    int current = resource_data[resource_type].current_consumption;
    int max_capacity = resource_data[resource_type].max_consumption;

    // BUG: No check for zero max_capacity
    return (current * 100) / max_capacity;
}
```

When `max_capacity` is 0 (no buildings consume this resource), division by zero occurs.

## Steps to Reproduce

1. Start a new city with no industrial buildings
2. Open the resource advisor
3. View consumption statistics for wheat/timber/etc.
4. Game crashes

Alternative:
1. Demolish all buildings that consume a specific resource
2. Open resource statistics
3. Crash when viewing that resource's consumption ratio

## Expected Behavior

When no buildings consume a resource:
- Return 0% consumption (or display "N/A")
- No crash should occur

## Current Behavior

Floating point exception when max_capacity is 0:
```
Signal: SIGFPE (Floating point exception)
at get_resource_consumption_percent()
```

## Relevant Code

Look at `src/city/resource.c`:
- `get_resource_consumption_percent()` function
- Similar ratio calculations for other resource stats

## Suggested Fix Approach

Add zero check before division:

```c
int get_resource_consumption_percent(int resource_type)
{
    int current = resource_data[resource_type].current_consumption;
    int max_capacity = resource_data[resource_type].max_consumption;

    // FIXED: Check for zero denominator
    if (max_capacity == 0) {
        return 0;
    }

    return (current * 100) / max_capacity;
}
```

## Your Task

Fix the division by zero crash by adding proper validation. Your fix should:

1. Check if max_capacity is zero before dividing
2. Return 0 when there's no consumption capacity
3. Preserve normal calculation when capacity is non-zero
4. Be a minimal, targeted fix

Provide your fix as a unified diff (patch).
