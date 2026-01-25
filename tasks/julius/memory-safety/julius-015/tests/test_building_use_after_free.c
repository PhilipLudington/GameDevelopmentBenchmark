/*
 * Test for julius-015: Use-after-free in building deletion callback
 *
 * This test verifies that building deletion notifies callbacks BEFORE
 * freeing the building memory.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BUILDINGS 100
#define MAX_CALLBACKS 50

#define EVENT_BUILDING_DELETED 1

typedef struct building {
    int id;
    int type;
    int x;
    int y;
    int workers;
    int storage;
    char name[32];
} building;

typedef void (*building_callback)(building *b, int event);

static struct {
    building *b;
    building_callback callback;
} registered_callbacks[MAX_CALLBACKS];

static int num_callbacks = 0;
static building *buildings[MAX_BUILDINGS];

/* Track callback invocations for verification */
static int callback_invoked = 0;
static int callback_building_id = -1;
static int callback_building_workers = -1;

/* Test callback that reads building data */
void on_building_deleted(building *b, int event)
{
    (void)event;
    callback_invoked = 1;

    /* This read will trigger use-after-free if building was freed first */
    callback_building_id = b->id;
    callback_building_workers = b->workers;

    printf("Callback: Building %d deleted (workers=%d)\n",
           b->id, b->workers);
}

/* Register a callback for a building */
void building_register_callback(building *b, building_callback cb)
{
    if (num_callbacks < MAX_CALLBACKS) {
        registered_callbacks[num_callbacks].b = b;
        registered_callbacks[num_callbacks].callback = cb;
        num_callbacks++;
    }
}

/* Create a new building */
building *building_create(int id, int type, int x, int y)
{
    building *b = malloc(sizeof(building));
    if (!b) return NULL;

    memset(b, 0, sizeof(building));
    b->id = id;
    b->type = type;
    b->x = x;
    b->y = y;
    b->workers = 10;
    b->storage = 100;
    snprintf(b->name, sizeof(b->name), "Building_%d", id);

    buildings[id] = b;
    return b;
}

/*
 * Building deletion - buggy or fixed version
 */
void building_delete(building *b)
{
    int building_id = b->id;

#ifdef BUGGY_VERSION
    /* BUG: Free building memory FIRST */
    free(b);

    /* Then notify callbacks - but b is now freed! */
    for (int i = 0; i < num_callbacks; i++) {
        if (registered_callbacks[i].b == b) {
            /* Use-after-free: callback reads from freed memory */
            registered_callbacks[i].callback(b, EVENT_BUILDING_DELETED);
        }
    }
#else
    /* FIXED: Notify callbacks FIRST while building is still valid */
    for (int i = 0; i < num_callbacks; i++) {
        if (registered_callbacks[i].b == b) {
            registered_callbacks[i].callback(b, EVENT_BUILDING_DELETED);
            /* Clear the registration */
            registered_callbacks[i].b = NULL;
            registered_callbacks[i].callback = NULL;
        }
    }

    /* Now safe to free the building */
    free(b);
#endif

    /* Clear from building list */
    buildings[building_id] = NULL;
}

/* Reset test state */
void reset_test_state(void)
{
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        buildings[i] = NULL;
    }
    num_callbacks = 0;
    callback_invoked = 0;
    callback_building_id = -1;
    callback_building_workers = -1;
}

/* Test basic building deletion with callback */
int test_delete_with_callback(void)
{
    reset_test_state();

    building *b = building_create(1, 5, 10, 20);
    if (!b) {
        printf("FAIL: Could not create building\n");
        return 1;
    }

    /* Set distinctive values for verification */
    b->workers = 42;
    b->storage = 500;

    /* Register callback */
    building_register_callback(b, on_building_deleted);

    printf("Deleting building (workers=%d)...\n", b->workers);

    /* This triggers use-after-free if buggy */
    building_delete(b);

    if (!callback_invoked) {
        printf("FAIL: Callback was not invoked\n");
        return 1;
    }

    /* Verify callback saw valid data */
    if (callback_building_id != 1) {
        printf("FAIL: Callback saw wrong building ID: %d\n", callback_building_id);
        return 1;
    }

    if (callback_building_workers != 42) {
        printf("FAIL: Callback saw wrong worker count: %d (expected 42)\n",
               callback_building_workers);
        return 1;
    }

    printf("PASS: Delete with callback test\n");
    return 0;
}

/* Test multiple buildings with callbacks */
int test_multiple_buildings(void)
{
    reset_test_state();

    building *b1 = building_create(1, 1, 0, 0);
    building *b2 = building_create(2, 2, 10, 10);
    building *b3 = building_create(3, 3, 20, 20);

    if (!b1 || !b2 || !b3) {
        printf("FAIL: Could not create buildings\n");
        return 1;
    }

    b1->workers = 11;
    b2->workers = 22;
    b3->workers = 33;

    building_register_callback(b1, on_building_deleted);
    building_register_callback(b2, on_building_deleted);
    building_register_callback(b3, on_building_deleted);

    /* Delete middle building */
    callback_invoked = 0;
    building_delete(b2);

    if (callback_building_workers != 22) {
        printf("FAIL: Callback saw wrong worker count for b2\n");
        return 1;
    }

    /* Delete first building */
    callback_invoked = 0;
    building_delete(b1);

    if (callback_building_workers != 11) {
        printf("FAIL: Callback saw wrong worker count for b1\n");
        return 1;
    }

    /* Delete last building */
    callback_invoked = 0;
    building_delete(b3);

    if (callback_building_workers != 33) {
        printf("FAIL: Callback saw wrong worker count for b3\n");
        return 1;
    }

    printf("PASS: Multiple buildings test\n");
    return 0;
}

/* Test building without callback */
int test_delete_without_callback(void)
{
    reset_test_state();

    building *b = building_create(5, 1, 50, 50);
    if (!b) {
        printf("FAIL: Could not create building\n");
        return 1;
    }

    callback_invoked = 0;

    /* Delete without registering callback */
    building_delete(b);

    if (callback_invoked) {
        printf("FAIL: Callback invoked when not registered\n");
        return 1;
    }

    printf("PASS: Delete without callback test\n");
    return 0;
}

int main(void)
{
    int failures = 0;

    printf("=== Building Use-After-Free Test ===\n\n");

#ifdef BUGGY_VERSION
    printf("Testing BUGGY version (expect ASan to catch use-after-free)\n\n");
#else
    printf("Testing FIXED version (should pass all tests)\n\n");
#endif

    failures += test_delete_with_callback();
    failures += test_multiple_buildings();
    failures += test_delete_without_callback();

    printf("\n=== Results: %d failures ===\n", failures);

    return failures;
}
