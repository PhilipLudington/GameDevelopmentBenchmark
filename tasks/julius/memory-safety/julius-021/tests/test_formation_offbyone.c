/*
 * Test for julius-021: Off-by-one error in array bounds check
 *
 * This test verifies that formation iteration uses correct bounds
 * and doesn't access past the end of the soldiers array.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SOLDIERS 16

typedef struct {
    int id;
    int health;
    int x, y;
    int updated;  /* Track if soldier was updated */
} soldier;

typedef struct {
    soldier soldiers[MAX_SOLDIERS];
    int num_soldiers;
} formation;

static int update_count = 0;

/* Track which soldiers were updated */
static void update_soldier(soldier *s)
{
    s->updated = 1;
    s->health += 1;  /* Simple update */
    update_count++;
}

/*
 * Simulates the buggy/fixed formation update
 */
void formation_update_soldiers(formation *f)
{
#ifdef BUGGY_VERSION
    /* BUG: <= causes one extra iteration */
    for (int i = 0; i <= f->num_soldiers; i++) {
        update_soldier(&f->soldiers[i]);
    }
#else
    /* FIXED: < is correct */
    for (int i = 0; i < f->num_soldiers; i++) {
        update_soldier(&f->soldiers[i]);
    }
#endif
}

/* Initialize a formation with given number of soldiers */
formation *formation_create(int num_soldiers)
{
    formation *f = malloc(sizeof(formation));
    if (!f) return NULL;

    memset(f, 0, sizeof(formation));
    f->num_soldiers = num_soldiers;

    /* Initialize soldiers */
    for (int i = 0; i < num_soldiers; i++) {
        f->soldiers[i].id = i + 1;
        f->soldiers[i].health = 100;
        f->soldiers[i].x = i * 10;
        f->soldiers[i].y = i * 10;
        f->soldiers[i].updated = 0;
    }

    return f;
}

/* Test with small formation */
int test_small_formation(void)
{
    formation *f = formation_create(5);
    if (!f) {
        printf("FAIL: Could not create formation\n");
        return 1;
    }

    update_count = 0;
    formation_update_soldiers(f);

    /* Should have updated exactly 5 soldiers */
    if (update_count != 5) {
        printf("FAIL: Expected 5 updates, got %d\n", update_count);
        free(f);
        return 1;
    }

    /* Verify correct soldiers were updated */
    for (int i = 0; i < 5; i++) {
        if (!f->soldiers[i].updated) {
            printf("FAIL: Soldier %d was not updated\n", i);
            free(f);
            return 1;
        }
    }

    free(f);
    printf("PASS: Small formation test (5 soldiers)\n");
    return 0;
}

/* Test with maximum formation - triggers OOB if buggy */
int test_max_formation(void)
{
    formation *f = formation_create(MAX_SOLDIERS);
    if (!f) {
        printf("FAIL: Could not create formation\n");
        return 1;
    }

    printf("Testing with MAX_SOLDIERS (%d)...\n", MAX_SOLDIERS);

    update_count = 0;

    /* This will access soldiers[MAX_SOLDIERS] if buggy */
    formation_update_soldiers(f);

    if (update_count != MAX_SOLDIERS) {
        printf("FAIL: Expected %d updates, got %d\n", MAX_SOLDIERS, update_count);
        free(f);
        return 1;
    }

    free(f);
    printf("PASS: Max formation test (%d soldiers)\n", MAX_SOLDIERS);
    return 0;
}

/* Test with empty formation */
int test_empty_formation(void)
{
    formation *f = formation_create(0);
    if (!f) {
        printf("FAIL: Could not create formation\n");
        return 1;
    }

    update_count = 0;
    formation_update_soldiers(f);

    /* With 0 soldiers, buggy version would still try to update soldiers[0] */
    if (update_count != 0) {
        printf("FAIL: Expected 0 updates for empty formation, got %d\n", update_count);
        free(f);
        return 1;
    }

    free(f);
    printf("PASS: Empty formation test\n");
    return 0;
}

/* Test with single soldier */
int test_single_soldier(void)
{
    formation *f = formation_create(1);
    if (!f) {
        printf("FAIL: Could not create formation\n");
        return 1;
    }

    update_count = 0;
    formation_update_soldiers(f);

    if (update_count != 1) {
        printf("FAIL: Expected 1 update, got %d\n", update_count);
        free(f);
        return 1;
    }

    free(f);
    printf("PASS: Single soldier test\n");
    return 0;
}

/* Test boundary case - one less than max */
int test_almost_max(void)
{
    formation *f = formation_create(MAX_SOLDIERS - 1);
    if (!f) {
        printf("FAIL: Could not create formation\n");
        return 1;
    }

    update_count = 0;
    formation_update_soldiers(f);

    if (update_count != MAX_SOLDIERS - 1) {
        printf("FAIL: Expected %d updates, got %d\n", MAX_SOLDIERS - 1, update_count);
        free(f);
        return 1;
    }

    free(f);
    printf("PASS: Almost-max formation test (%d soldiers)\n", MAX_SOLDIERS - 1);
    return 0;
}

int main(void)
{
    int failures = 0;

    printf("=== Formation Off-By-One Test ===\n\n");

#ifdef BUGGY_VERSION
    printf("Testing BUGGY version (expect ASan to catch OOB access)\n\n");
#else
    printf("Testing FIXED version (should pass all tests)\n\n");
#endif

    failures += test_small_formation();
    failures += test_single_soldier();
    failures += test_empty_formation();
    failures += test_almost_max();
    failures += test_max_formation();

    printf("\n=== Results: %d failures ===\n", failures);

    return failures;
}
