/*
 * Test for julius-016: Integer underflow in resource subtraction
 *
 * This test verifies that resource subtraction properly handles
 * cases where the subtracted amount exceeds the current count.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define RESOURCE_WHEAT 0
#define RESOURCE_VEGETABLES 1
#define RESOURCE_MEAT 2
#define RESOURCE_WINE 3
#define MAX_RESOURCES 10

typedef struct {
    unsigned int counts[MAX_RESOURCES];
} city_resources;

static city_resources resources;

/* Initialize resources */
void resource_init(void)
{
    memset(&resources, 0, sizeof(resources));
}

/* Get pointer to a resource count */
unsigned int *get_resource_count(int resource_type)
{
    if (resource_type >= 0 && resource_type < MAX_RESOURCES) {
        return &resources.counts[resource_type];
    }
    return &resources.counts[0];  /* Fallback */
}

/* Get current count (read-only) */
unsigned int resource_get(int resource_type)
{
    return *get_resource_count(resource_type);
}

/* Add resources */
void resource_add(int resource_type, unsigned int amount)
{
    unsigned int *count = get_resource_count(resource_type);
    *count += amount;
}

/*
 * Subtract resources - buggy or fixed version
 */
void resource_subtract(int resource_type, unsigned int amount)
{
    unsigned int *count = get_resource_count(resource_type);

#ifdef BUGGY_VERSION
    /* BUG: No check for underflow */
    *count -= amount;
#else
    /* FIXED: Prevent underflow */
    if (amount > *count) {
        *count = 0;
    } else {
        *count -= amount;
    }
#endif
}

/* Test normal subtraction */
int test_normal_subtraction(void)
{
    resource_init();
    resource_add(RESOURCE_WHEAT, 1000);

    resource_subtract(RESOURCE_WHEAT, 300);

    unsigned int count = resource_get(RESOURCE_WHEAT);
    if (count != 700) {
        printf("FAIL: Normal subtraction - expected 700, got %u\n", count);
        return 1;
    }

    printf("PASS: Normal subtraction (1000 - 300 = 700)\n");
    return 0;
}

/* Test exact subtraction (count becomes zero) */
int test_exact_subtraction(void)
{
    resource_init();
    resource_add(RESOURCE_VEGETABLES, 500);

    resource_subtract(RESOURCE_VEGETABLES, 500);

    unsigned int count = resource_get(RESOURCE_VEGETABLES);
    if (count != 0) {
        printf("FAIL: Exact subtraction - expected 0, got %u\n", count);
        return 1;
    }

    printf("PASS: Exact subtraction (500 - 500 = 0)\n");
    return 0;
}

/* Test underflow case - subtracting more than available */
int test_underflow_prevention(void)
{
    resource_init();
    resource_add(RESOURCE_MEAT, 100);

    printf("Testing underflow: 100 - 200...\n");

    resource_subtract(RESOURCE_MEAT, 200);

    unsigned int count = resource_get(RESOURCE_MEAT);

    /* If buggy, count wraps to ~4294967196 */
    if (count > 100) {
        printf("FAIL: Underflow occurred - got %u (wrapped around!)\n", count);
        return 1;
    }

    if (count != 0) {
        printf("FAIL: Expected 0 after underflow clamp, got %u\n", count);
        return 1;
    }

    printf("PASS: Underflow prevented (100 - 200 clamped to 0)\n");
    return 0;
}

/* Test subtracting from zero */
int test_subtract_from_zero(void)
{
    resource_init();
    /* Don't add anything - count is 0 */

    resource_subtract(RESOURCE_WINE, 50);

    unsigned int count = resource_get(RESOURCE_WINE);

    if (count != 0) {
        printf("FAIL: Subtract from zero - expected 0, got %u\n", count);
        return 1;
    }

    printf("PASS: Subtract from zero (0 - 50 = 0)\n");
    return 0;
}

/* Test large subtraction */
int test_large_subtraction(void)
{
    resource_init();
    resource_add(RESOURCE_WHEAT, 1);

    /* Try to subtract a huge amount */
    resource_subtract(RESOURCE_WHEAT, UINT_MAX);

    unsigned int count = resource_get(RESOURCE_WHEAT);

    if (count != 0) {
        printf("FAIL: Large subtraction - expected 0, got %u\n", count);
        return 1;
    }

    printf("PASS: Large subtraction (1 - UINT_MAX clamped to 0)\n");
    return 0;
}

/* Test multiple sequential operations */
int test_sequential_operations(void)
{
    resource_init();
    resource_add(RESOURCE_WHEAT, 100);

    resource_subtract(RESOURCE_WHEAT, 30);  /* 100 - 30 = 70 */
    resource_subtract(RESOURCE_WHEAT, 50);  /* 70 - 50 = 20 */
    resource_subtract(RESOURCE_WHEAT, 100); /* 20 - 100 = 0 (clamped) */

    unsigned int count = resource_get(RESOURCE_WHEAT);

    if (count != 0) {
        printf("FAIL: Sequential operations - expected 0, got %u\n", count);
        return 1;
    }

    printf("PASS: Sequential operations\n");
    return 0;
}

int main(void)
{
    int failures = 0;

    printf("=== Resource Integer Underflow Test ===\n\n");

#ifdef BUGGY_VERSION
    printf("Testing BUGGY version (expect underflow)\n\n");
#else
    printf("Testing FIXED version (should pass all tests)\n\n");
#endif

    failures += test_normal_subtraction();
    failures += test_exact_subtraction();
    failures += test_underflow_prevention();
    failures += test_subtract_from_zero();
    failures += test_large_subtraction();
    failures += test_sequential_operations();

    printf("\n=== Results: %d failures ===\n", failures);

    return failures;
}
