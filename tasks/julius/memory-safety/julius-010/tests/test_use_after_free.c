/**
 * Test for use-after-free in UI callback system
 *
 * Test validation:
 * - FIXED code: Data remains valid after trigger, tests pass
 * - BUGGY code: Data freed prematurely, ASan detects use-after-free
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* External functions from stub or Julius src/window/city.c */
extern void ui_trigger_callback_unsafe(void *data);
extern void *ui_get_callback_data(void);
extern void ui_cleanup_callback_data(void);

void test_callback_data_valid(void)
{
    printf("Test: callback data remains valid after trigger...\n");

    /* Allocate and initialize test data */
    char *data = malloc(100);
    assert(data != NULL);
    strcpy(data, "Important callback data");
    printf("  Original data: %s\n", data);

    /* Trigger callback - transfers ownership */
    ui_trigger_callback_unsafe(data);

    /* Retrieve data - should still be valid in fixed version */
    void *retrieved = ui_get_callback_data();
    assert(retrieved != NULL);
    printf("  Retrieved pointer: %p\n", retrieved);

    /* CRITICAL: Access the data - this is where use-after-free would occur */
    /* In buggy code, this would access freed memory */
    char *str = (char *)retrieved;
    printf("  Retrieved data: %s\n", str);

    /* Verify data is intact */
    assert(strcmp(str, "Important callback data") == 0);

    /* Clean up properly */
    ui_cleanup_callback_data();

    printf("  PASS\n");
}

void test_multiple_triggers(void)
{
    printf("Test: multiple callback triggers...\n");

    /* First trigger */
    char *data1 = malloc(50);
    strcpy(data1, "First callback");
    ui_trigger_callback_unsafe(data1);

    void *retrieved1 = ui_get_callback_data();
    assert(strcmp((char *)retrieved1, "First callback") == 0);
    printf("  First callback verified\n");

    /* Clean up first before second trigger */
    ui_cleanup_callback_data();

    /* Second trigger */
    char *data2 = malloc(50);
    strcpy(data2, "Second callback");
    ui_trigger_callback_unsafe(data2);

    void *retrieved2 = ui_get_callback_data();
    assert(strcmp((char *)retrieved2, "Second callback") == 0);
    printf("  Second callback verified\n");

    ui_cleanup_callback_data();

    printf("  PASS\n");
}

void test_null_data(void)
{
    printf("Test: NULL data handling...\n");

    /* Should handle NULL gracefully */
    ui_trigger_callback_unsafe(NULL);
    void *retrieved = ui_get_callback_data();

    /* NULL should be stored as-is */
    assert(retrieved == NULL);

    printf("  PASS\n");
}

int main(void)
{
    printf("=== Use-After-Free Test Suite ===\n\n");
    test_null_data();
    test_callback_data_valid();
    test_multiple_triggers();
    printf("\n=== All tests passed ===\n");
    return 0;
}
