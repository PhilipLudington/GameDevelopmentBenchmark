# Bug Report: Integer Overflow in Resource Calculation

## Summary

The resource calculation code has an integer overflow vulnerability. When calculating total resource values (quantity * price), large quantities can cause the multiplication to overflow a 32-bit integer, wrapping around to incorrect values.

## Environment

- Project: Julius (Caesar III reimplementation)
- File: `src/city/resource.c`
- Severity: Medium (gameplay issue, can cause economic exploits)

## Bug Description

The resource value calculation multiplies quantity by price using 32-bit integers:

```c
int calculate_resource_value(int quantity, int price_per_unit)
{
    return quantity * price_per_unit;  // Can overflow!
}
```

In Caesar III, warehouses can store up to 3200 units per resource type, and cities can have many warehouses. With trade route bonuses and inflation, prices can reach 1000+ denarii per unit.

Example overflow scenario:
- Quantity: 50,000 units (multiple warehouses)
- Price: 50,000 denarii per unit (extreme case)
- Expected result: 2,500,000,000
- Actual result: -1,794,967,296 (32-bit overflow!)

The signed integer overflow can produce:
1. Negative values (treating unsigned as signed)
2. Small positive values (wrapping around)
3. Undefined behavior (C standard)

## Steps to Reproduce

1. Accumulate a large stockpile of expensive resources
2. Calculate total trade value
3. Observe incorrect (negative or wrapped) values

Or programmatically:
```c
int value = calculate_resource_value(50000, 50000);
// Expected: 2,500,000,000
// Actual: undefined/negative due to overflow
```

## Expected Behavior

The function should:
1. Detect potential overflow before it occurs
2. Either cap at INT_MAX or use wider types for intermediate calculations
3. Never return incorrect results due to overflow

## Current Behavior

Silently overflows, returning incorrect (possibly negative) values.

## Relevant Code

Look at `src/city/resource.c`, specifically:
- `calculate_resource_value()` function
- Any code multiplying quantities by prices
- Trade calculation functions

## Suggested Fix Approach

Option 1: Use 64-bit intermediates:
```c
int calculate_resource_value(int quantity, int price_per_unit)
{
    int64_t result = (int64_t)quantity * price_per_unit;
    if (result > INT_MAX) return INT_MAX;
    if (result < INT_MIN) return INT_MIN;
    return (int)result;
}
```

Option 2: Check before multiplying:
```c
int calculate_resource_value(int quantity, int price_per_unit)
{
    if (quantity > 0 && price_per_unit > INT_MAX / quantity) {
        return INT_MAX;  // Would overflow
    }
    if (quantity < 0 && price_per_unit < INT_MAX / quantity) {
        return INT_MIN;  // Would underflow
    }
    return quantity * price_per_unit;
}
```

## Your Task

Fix the integer overflow by implementing proper overflow detection. Your fix should:
1. Detect overflow before it occurs
2. Handle both positive and negative values correctly
3. Return a reasonable capped value (INT_MAX or INT_MIN) on overflow
4. Not rely on undefined behavior

Provide your fix as a unified diff (patch).
