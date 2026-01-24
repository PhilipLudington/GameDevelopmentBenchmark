/**
 * Stub implementation for julius-008 integer overflow test
 *
 * This provides the resource_calculate_value function for testing.
 * The actual Julius source is verified via static analysis.
 */

#include <stdio.h>
#include <stdint.h>
#include <limits.h>

/**
 * BUGGY version: Direct multiplication can overflow
 *
 * int resource_calculate_value(int quantity, int price_per_unit)
 * {
 *     int total = quantity * price_per_unit;
 *     return total;
 * }
 *
 * FIXED version: Uses wider type or checks for overflow
 */
int resource_calculate_value(int quantity, int price_per_unit)
{
    /* Use 64-bit arithmetic to detect overflow */
    int64_t result = (int64_t)quantity * (int64_t)price_per_unit;

    /* Clamp to INT_MAX if overflow would occur */
    if (result > INT_MAX) {
        return INT_MAX;
    }
    if (result < INT_MIN) {
        return INT_MIN;
    }

    return (int)result;
}
