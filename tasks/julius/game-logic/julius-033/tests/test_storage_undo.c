/**
 * Test for storage building undo linking corruption (julius-033)
 *
 * This test verifies that removing a storage building from the doubly-linked
 * list properly updates both prev->next and next->prev pointers.
 *
 * Test validation:
 * - On FIXED code: List integrity maintained, forward/backward counts match
 * - On BUGGY code: prev pointer corruption causes count mismatch
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/* Storage building structure */
typedef struct storage_building {
    int building_id;
    int capacity;
    struct storage_building *prev;
    struct storage_building *next;
} storage_building;

/* External functions from stubs */
extern storage_building *storage_building_create(int building_id, int capacity);
extern void storage_building_add(storage_building *storage);
extern void storage_building_remove(storage_building *storage);
extern storage_building *storage_get_list_head(void);
extern void storage_clear_all(void);
extern int storage_count_forward(void);
extern int storage_count_backward_from(storage_building *start);
extern storage_building *storage_find_last(void);
extern storage_building *storage_find_by_id(int building_id);
extern int storage_validate_list(void);

/**
 * Test 1: Remove from middle of list
 *
 * Create A -> B -> C, remove B, verify A <-> C
 */
static int test_remove_middle(void)
{
    printf("Test: Remove storage building from middle of list...\n");

    storage_clear_all();

    /* Add three buildings (added to head, so C is first) */
    storage_building *a = storage_building_create(1, 100);
    storage_building *b = storage_building_create(2, 100);
    storage_building *c = storage_building_create(3, 100);

    storage_building_add(a);
    storage_building_add(b);
    storage_building_add(c);

    /* List is now: C -> B -> A */
    printf("  Before removal: %d buildings forward\n", storage_count_forward());

    /* Remove B (middle) */
    storage_building_remove(b);

    printf("  After removing B: %d buildings forward\n", storage_count_forward());

    /* Validate list integrity */
    if (!storage_validate_list()) {
        printf("  FAIL: List integrity check failed (prev pointer corruption)\n");
        return 0;
    }

    /* Count should be 2 both ways */
    int forward = storage_count_forward();
    storage_building *last = storage_find_last();
    int backward = storage_count_backward_from(last);

    printf("  Forward count: %d, Backward count: %d\n", forward, backward);

    if (forward != 2 || backward != 2) {
        printf("  FAIL: Count mismatch (expected 2 both ways)\n");
        return 0;
    }

    printf("  PASS: Middle removal maintains list integrity\n");
    storage_clear_all();
    return 1;
}

/**
 * Test 2: Remove from head of list
 */
static int test_remove_head(void)
{
    printf("Test: Remove storage building from head of list...\n");

    storage_clear_all();

    storage_building *a = storage_building_create(1, 100);
    storage_building *b = storage_building_create(2, 100);

    storage_building_add(a);
    storage_building_add(b);

    /* List is: B -> A, remove B (head) */
    storage_building *head = storage_get_list_head();
    storage_building_remove(head);

    if (!storage_validate_list()) {
        printf("  FAIL: List integrity check failed\n");
        return 0;
    }

    int count = storage_count_forward();
    if (count != 1) {
        printf("  FAIL: Expected 1 building, got %d\n", count);
        return 0;
    }

    printf("  PASS: Head removal works correctly\n");
    storage_clear_all();
    return 1;
}

/**
 * Test 3: Remove from tail of list
 */
static int test_remove_tail(void)
{
    printf("Test: Remove storage building from tail of list...\n");

    storage_clear_all();

    storage_building *a = storage_building_create(1, 100);
    storage_building *b = storage_building_create(2, 100);

    storage_building_add(a);
    storage_building_add(b);

    /* List is: B -> A, remove A (tail) */
    storage_building *tail = storage_find_last();
    storage_building_remove(tail);

    if (!storage_validate_list()) {
        printf("  FAIL: List integrity check failed\n");
        return 0;
    }

    int count = storage_count_forward();
    if (count != 1) {
        printf("  FAIL: Expected 1 building, got %d\n", count);
        return 0;
    }

    printf("  PASS: Tail removal works correctly\n");
    storage_clear_all();
    return 1;
}

/**
 * Test 4: Remove only element
 */
static int test_remove_only(void)
{
    printf("Test: Remove only storage building in list...\n");

    storage_clear_all();

    storage_building *a = storage_building_create(1, 100);
    storage_building_add(a);

    storage_building_remove(a);

    storage_building *head = storage_get_list_head();
    if (head != NULL) {
        printf("  FAIL: Head should be NULL after removing only element\n");
        return 0;
    }

    int count = storage_count_forward();
    if (count != 0) {
        printf("  FAIL: Expected 0 buildings, got %d\n", count);
        return 0;
    }

    printf("  PASS: Single element removal works correctly\n");
    return 1;
}

/**
 * Test 5: Backward traversal after removal (the key bug test)
 *
 * This specifically tests that next->prev is updated correctly.
 */
static int test_backward_traversal(void)
{
    printf("Test: Backward traversal after middle removal...\n");

    storage_clear_all();

    /* Create A, B, C */
    storage_building *a = storage_building_create(1, 100);
    storage_building *b = storage_building_create(2, 100);
    storage_building *c = storage_building_create(3, 100);

    storage_building_add(a);
    storage_building_add(b);
    storage_building_add(c);

    /* List: C(head) -> B -> A(tail) */

    /* Remove B */
    storage_building_remove(b);

    /* Now traverse backward from A */
    storage_building *last = storage_find_last();
    if (!last) {
        printf("  FAIL: Could not find last element\n");
        return 0;
    }

    printf("  Last building ID: %d\n", last->building_id);

    /* A->prev should now point to C (the head), not to freed B */
    if (last->prev == NULL) {
        printf("  FAIL: last->prev is NULL (should point to head)\n");
        return 0;
    }

    /* Check that we can traverse back to head */
    storage_building *head = storage_get_list_head();
    if (last->prev != head) {
        printf("  FAIL: last->prev does not point to head (prev pointer corruption)\n");
        printf("  This is the bug - next->prev was not updated during removal!\n");
        return 0;
    }

    printf("  last->prev correctly points to head (ID %d)\n", last->prev->building_id);
    printf("  PASS: Backward traversal works after removal\n");
    storage_clear_all();
    return 1;
}

int main(void)
{
    int passed = 0;
    int total = 5;

    printf("=== Storage Building Undo Linking Test Suite ===\n");
    printf("Testing doubly-linked list integrity after removal\n");
    printf("Bug: next->prev pointer not updated during removal\n\n");

    if (test_remove_middle()) passed++;
    printf("\n");

    if (test_remove_head()) passed++;
    printf("\n");

    if (test_remove_tail()) passed++;
    printf("\n");

    if (test_remove_only()) passed++;
    printf("\n");

    if (test_backward_traversal()) passed++;

    printf("\n=== Test Results ===\n");
    printf("Results: %d/%d tests passed\n", passed, total);

    return (passed == total) ? 0 : 1;
}
