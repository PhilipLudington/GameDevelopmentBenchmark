# Bug Report: Integer underflow in resource subtraction

## Summary

The resource management system uses unsigned integers for resource counts. When more resources are subtracted than available, the counter wraps around to a very large positive value instead of stopping at zero.

## Environment

- Project: Julius (Caesar III reimplementation)
- File: `src/city/resource.c`
- Severity: High (game state corruption, exploit potential)

## Bug Description

Resource counts (wheat, pottery, weapons, etc.) are stored as unsigned integers. When the game subtracts resources (e.g., when consumed or exported), it doesn't check if the subtraction would result in a negative value. Since unsigned integers can't be negative, the result wraps around.

```c
typedef struct {
    unsigned int wheat;
    unsigned int vegetables;
    unsigned int meat;
    unsigned int wine;
    // ... other resources
} city_resources;

void resource_subtract(int resource_type, unsigned int amount)
{
    unsigned int *count = get_resource_count(resource_type);

    // BUG: No check for underflow
    *count -= amount;

    // If amount > *count, this wraps to a huge number
    // e.g., 100 - 200 = 4294967196 (not -100)
}
```

## Steps to Reproduce

1. City has 100 units of wheat
2. System tries to subtract 200 units (e.g., during export or consumption)
3. Wheat count becomes 4294967196 instead of 0
4. City suddenly has unlimited wheat

## Expected Behavior

The subtraction should either:
- Return an error if there aren't enough resources
- Clamp the count to zero
- Only subtract the available amount

## Current Behavior

The unsigned subtraction wraps around:
- Before: wheat = 100
- Subtract: 200
- After: wheat = 4294967196 (0xFFFFFF9C)

This breaks game balance and can lead to further memory corruption if the huge count is used as an array index.

## Relevant Code

Look at `src/city/resource.c`:
- `resource_subtract()` function
- Any other functions that decrease resource counts
- Consider functions that consume resources for building or trade

## Suggested Fix Approach

Add bounds checking before subtraction:

```c
void resource_subtract(int resource_type, unsigned int amount)
{
    unsigned int *count = get_resource_count(resource_type);

    // Check for underflow
    if (amount > *count) {
        *count = 0;  // Clamp to zero
    } else {
        *count -= amount;
    }
}

// Or return success/failure:
int resource_subtract_safe(int resource_type, unsigned int amount)
{
    unsigned int *count = get_resource_count(resource_type);

    if (amount > *count) {
        return 0;  // Failure - not enough resources
    }
    *count -= amount;
    return 1;  // Success
}
```

## Your Task

Fix the integer underflow by adding proper bounds checking. Your fix should:

1. Prevent the count from wrapping below zero
2. Either clamp to zero or return an error code
3. Work correctly when amount equals count exactly
4. Not change behavior when amount is less than count

Provide your fix as a unified diff (patch).
