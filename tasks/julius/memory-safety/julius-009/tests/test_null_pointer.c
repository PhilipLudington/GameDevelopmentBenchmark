/**
 * Test for null pointer dereference in building lookup
 *
 * Test validation:
 * - FIXED code: NULL check prevents crash, tests pass
 * - BUGGY code: NULL dereference causes crash/ASan error
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/* External function from stub or Julius src/building/building.c */
extern int building_get_fire_risk_unsafe(int building_id);

void test_invalid_building_id(void)
{
    printf("Test: invalid building ID (0)...\n");
    /* ID 0 is invalid - should return 0, not crash */
    int risk = building_get_fire_risk_unsafe(0);
    printf("  Fire risk for ID 0: %d\n", risk);
    assert(risk == 0);
    printf("  PASS\n");
}

void test_negative_building_id(void)
{
    printf("Test: negative building ID...\n");
    /* Negative ID is invalid - should return 0, not crash */
    int risk = building_get_fire_risk_unsafe(-1);
    printf("  Fire risk for ID -1: %d\n", risk);
    assert(risk == 0);
    printf("  PASS\n");
}

void test_out_of_range_id(void)
{
    printf("Test: out of range building ID...\n");
    /* ID beyond max buildings - should return 0, not crash */
    int risk = building_get_fire_risk_unsafe(99999);
    printf("  Fire risk for ID 99999: %d\n", risk);
    assert(risk == 0);
    printf("  PASS\n");
}

void test_deleted_building(void)
{
    printf("Test: accessing deleted building...\n");
    /* ID 1 is a deleted building (not in use) - should return 0, not crash */
    int risk = building_get_fire_risk_unsafe(1);
    printf("  Fire risk for ID 1 (deleted): %d\n", risk);
    assert(risk == 0);
    printf("  PASS\n");
}

int main(void)
{
    printf("=== Null Pointer Test Suite ===\n\n");
    test_invalid_building_id();
    test_negative_building_id();
    test_out_of_range_id();
    test_deleted_building();
    printf("\n=== All tests passed ===\n");
    return 0;
}
