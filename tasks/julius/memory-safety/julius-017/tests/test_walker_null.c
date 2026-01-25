/*
 * Test for julius-017: Null pointer dereference in empty walker list
 *
 * This test verifies that walker list iteration properly handles
 * an empty list without crashing.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct walker {
    int id;
    int x, y;
    int destination_x, destination_y;
    struct walker *next;
} walker;

static walker *walker_list_head = NULL;
static int update_count = 0;

/* Simulated walker update */
void walker_update_position(walker *w)
{
    update_count++;
    printf("  Updating walker %d at (%d, %d)\n", w->id, w->x, w->y);
}

/*
 * Simulates the buggy/fixed walker update function
 */
void walker_update_all(void)
{
    walker *current = walker_list_head;

#ifdef BUGGY_VERSION
    /* BUG: Accesses current->next without null check */
    while (current->next != NULL) {
        walker_update_position(current);
        current = current->next;
    }
#else
    /* FIXED: Check current != NULL first */
    while (current != NULL) {
        walker_update_position(current);
        current = current->next;
    }
#endif
}

/* Add a walker to the list */
walker *walker_create(int id, int x, int y)
{
    walker *w = malloc(sizeof(walker));
    if (!w) return NULL;

    w->id = id;
    w->x = x;
    w->y = y;
    w->destination_x = x + 10;
    w->destination_y = y + 10;
    w->next = walker_list_head;
    walker_list_head = w;

    return w;
}

/* Clear all walkers */
void walker_clear_all(void)
{
    walker *current = walker_list_head;
    while (current != NULL) {
        walker *next = current->next;
        free(current);
        current = next;
    }
    walker_list_head = NULL;
}

/* Test updating an empty list - triggers crash if buggy */
int test_empty_list(void)
{
    walker_clear_all();
    update_count = 0;

    printf("Testing update on empty list...\n");

    /* This will crash if the null check is missing */
    walker_update_all();

    if (update_count != 0) {
        printf("FAIL: Expected 0 updates on empty list, got %d\n", update_count);
        return 1;
    }

    printf("PASS: Empty list handled safely\n");
    return 0;
}

/* Test updating a list with one walker */
int test_single_walker(void)
{
    walker_clear_all();
    update_count = 0;

    walker_create(1, 10, 20);

    printf("Testing update on single walker list...\n");
    walker_update_all();

    if (update_count != 1) {
        printf("FAIL: Expected 1 update, got %d\n", update_count);
        walker_clear_all();
        return 1;
    }

    walker_clear_all();
    printf("PASS: Single walker list\n");
    return 0;
}

/* Test updating a list with multiple walkers */
int test_multiple_walkers(void)
{
    walker_clear_all();
    update_count = 0;

    walker_create(1, 10, 10);
    walker_create(2, 20, 20);
    walker_create(3, 30, 30);
    walker_create(4, 40, 40);
    walker_create(5, 50, 50);

    printf("Testing update on 5-walker list...\n");
    walker_update_all();

    if (update_count != 5) {
        printf("FAIL: Expected 5 updates, got %d\n", update_count);
        walker_clear_all();
        return 1;
    }

    walker_clear_all();
    printf("PASS: Multiple walkers list\n");
    return 0;
}

/* Test clearing and re-updating */
int test_clear_and_update(void)
{
    walker_clear_all();

    /* Add some walkers */
    walker_create(1, 0, 0);
    walker_create(2, 0, 0);

    update_count = 0;
    walker_update_all();

    if (update_count != 2) {
        printf("FAIL: Expected 2 updates before clear\n");
        walker_clear_all();
        return 1;
    }

    /* Clear all */
    walker_clear_all();

    /* Update empty list - should not crash */
    update_count = 0;
    printf("Testing update after clearing list...\n");
    walker_update_all();

    if (update_count != 0) {
        printf("FAIL: Expected 0 updates after clear\n");
        return 1;
    }

    printf("PASS: Clear and update test\n");
    return 0;
}

int main(void)
{
    int failures = 0;

    printf("=== Walker Null Pointer Test ===\n\n");

#ifdef BUGGY_VERSION
    printf("Testing BUGGY version (expect SEGV on empty list)\n\n");
#else
    printf("Testing FIXED version (should pass all tests)\n\n");
#endif

    failures += test_empty_list();
    failures += test_single_walker();
    failures += test_multiple_walkers();
    failures += test_clear_and_update();

    printf("\n=== Results: %d failures ===\n", failures);

    return failures;
}
