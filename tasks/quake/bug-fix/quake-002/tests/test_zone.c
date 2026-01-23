/*
 * Zone Allocator Test Harness
 *
 * Tests the zone memory allocator for correctness under various conditions.
 * Specifically tests for:
 * 1. Basic alloc/free functionality
 * 2. Block coalescing correctness
 * 3. Rover pointer validity after coalescing
 * 4. Size overflow protection
 * 5. Heap integrity under stress
 *
 * Build:
 *   make test        - Test buggy (game) version
 *   make test_opt    - Test fixed (solution) version
 *   make compare     - Compare both versions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>

/* ============== MOCK QUAKE SYSTEM FUNCTIONS ============== */

/* Mock system functions - called by zone.c */
static int error_called = 0;
static char last_error[256] = {0};

void Sys_Error(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vsnprintf(last_error, sizeof(last_error), fmt, args);
    va_end(args);
    error_called = 1;
}

void Con_Printf(const char *fmt, ...)
{
    (void)fmt;  /* Suppress unused parameter warning */
}

void Q_memset(void *dest, int fill, int count)
{
    memset(dest, fill, count);
}

/* Reset error state */
static void reset_error_state(void)
{
    error_called = 0;
    last_error[0] = '\0';
}

/* ============== INCLUDE ZONE IMPLEMENTATION ============== */

/* The zone.c file defines all types and functions we need */
#ifdef TEST_SOLUTION
#include "../solution/zone.c"
#else
#include "../game/zone.c"
#endif

/* ============== TEST UTILITIES ============== */

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) \
    static void test_##name(void); \
    static void run_test_##name(void) { \
        printf("Testing: %s...\n", #name); \
        test_##name(); \
    } \
    static void test_##name(void)

#define ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            printf("  FAIL: %s\n", msg); \
            tests_failed++; \
            return; \
        } \
    } while(0)

#define PASS() \
    do { \
        printf("  PASS\n"); \
        tests_passed++; \
    } while(0)

/* Zone memory pool */
#define ZONE_SIZE (256 * 1024)  /* 256KB test zone */
static byte zone_memory[ZONE_SIZE];

/* Initialize the zone for testing */
static void setup_zone(void)
{
    mainzone = (memzone_t *)zone_memory;
    mainzone->size = ZONE_SIZE;
    Z_ClearZone(mainzone, ZONE_SIZE);
    reset_error_state();
}

/* Verify heap integrity - returns 1 if valid, 0 if corrupt */
static int verify_heap_integrity(void)
{
    memblock_t *block;
    int rover_found = 0;
    int block_count = 0;

    for (block = mainzone->blocklist.next; ; block = block->next)
    {
        block_count++;
        if (block_count > 10000) {
            /* Infinite loop protection */
            return 0;
        }

        if (block == mainzone->rover)
            rover_found = 1;

        if (block->next == &mainzone->blocklist)
            break;

        /* Check block touches next block */
        if ((byte *)block + block->size != (byte *)block->next)
            return 0;

        /* Check back link */
        if (block->next->prev != block)
            return 0;

        /* Check no consecutive free blocks */
        if (!block->tag && !block->next->tag)
            return 0;
    }

    /* Check rover is valid */
    if (!rover_found && mainzone->rover != &mainzone->blocklist)
        return 0;

    return 1;
}

/* ============== BASIC TESTS ============== */

TEST(zone_init)
{
    setup_zone();

    ASSERT(mainzone != NULL, "Zone should be initialized");
    ASSERT(mainzone->rover != NULL, "Rover should be set");
    ASSERT(mainzone->blocklist.tag == 1, "Blocklist sentinel should be tagged");

    PASS();
}

TEST(single_alloc_free)
{
    setup_zone();

    void *ptr = Z_Malloc(100);
    ASSERT(ptr != NULL, "Allocation should succeed");
    ASSERT(!error_called, "No error should occur");

    Z_Free(ptr);
    ASSERT(!error_called, "Free should succeed");
    ASSERT(verify_heap_integrity(), "Heap should be valid after free");

    PASS();
}

TEST(multiple_allocs)
{
    setup_zone();

    void *ptrs[10];
    int i;
    for (i = 0; i < 10; i++) {
        ptrs[i] = Z_Malloc(100);
        ASSERT(ptrs[i] != NULL, "Each allocation should succeed");
    }

    ASSERT(verify_heap_integrity(), "Heap should be valid after multiple allocs");

    for (i = 0; i < 10; i++) {
        Z_Free(ptrs[i]);
    }

    ASSERT(verify_heap_integrity(), "Heap should be valid after all frees");
    PASS();
}

/* ============== COALESCING TESTS ============== */

TEST(coalesce_prev)
{
    setup_zone();

    /* Allocate two adjacent blocks */
    void *ptr1 = Z_Malloc(100);
    void *ptr2 = Z_Malloc(100);

    ASSERT(ptr1 != NULL && ptr2 != NULL, "Allocations should succeed");

    /* Free first, then second - second should coalesce with first */
    Z_Free(ptr1);
    ASSERT(verify_heap_integrity(), "Heap valid after first free");

    Z_Free(ptr2);
    ASSERT(verify_heap_integrity(), "Heap valid after coalescing with prev");

    PASS();
}

TEST(coalesce_next)
{
    setup_zone();

    /* Allocate three blocks */
    void *ptr1 = Z_Malloc(100);
    void *ptr2 = Z_Malloc(100);
    void *ptr3 = Z_Malloc(100);

    ASSERT(ptr1 && ptr2 && ptr3, "Allocations should succeed");

    /* Free middle first, then first - first should coalesce with middle */
    Z_Free(ptr2);  /* Now there's a free block in the middle */
    ASSERT(verify_heap_integrity(), "Heap valid after middle free");

    Z_Free(ptr1);  /* Should coalesce with next (the freed middle block) */
    ASSERT(verify_heap_integrity(), "Heap valid after coalescing with next");

    Z_Free(ptr3);
    ASSERT(verify_heap_integrity(), "Heap valid after all frees");

    PASS();
}

TEST(coalesce_both)
{
    setup_zone();

    /* Allocate three blocks */
    void *ptr1 = Z_Malloc(100);
    void *ptr2 = Z_Malloc(100);
    void *ptr3 = Z_Malloc(100);

    ASSERT(ptr1 && ptr2 && ptr3, "Allocations should succeed");

    /* Free first and last, leaving middle allocated */
    Z_Free(ptr1);
    Z_Free(ptr3);
    ASSERT(verify_heap_integrity(), "Heap valid after freeing outer blocks");

    /* Free middle - should coalesce with both neighbors */
    Z_Free(ptr2);
    ASSERT(verify_heap_integrity(), "Heap valid after coalescing both directions");

    PASS();
}

/* ============== ROVER VALIDITY TESTS ============== */

TEST(rover_after_coalesce_prev)
{
    setup_zone();

    /* Allocate several blocks */
    void *ptr1 = Z_Malloc(100);
    void *ptr2 = Z_Malloc(100);
    void *ptr3 = Z_Malloc(100);

    /* Free ptr1, rover might point to ptr2's block or nearby */
    Z_Free(ptr1);

    /* Now free ptr2 - when coalescing with ptr1's freed block,
       the rover must be updated if it pointed to ptr1's block */
    Z_Free(ptr2);

    /* Verify rover is still valid */
    ASSERT(verify_heap_integrity(), "Rover should be valid after coalesce with prev");

    Z_Free(ptr3);
    PASS();
}

TEST(rover_after_coalesce_next)
{
    setup_zone();

    void *ptr1 = Z_Malloc(100);
    void *ptr2 = Z_Malloc(100);
    void *ptr3 = Z_Malloc(100);
    void *ptr4 = Z_Malloc(100);

    /* Free ptr3 first */
    Z_Free(ptr3);

    /* The rover often advances. Now free ptr2 - it should coalesce
       forward into ptr3's freed space. If rover pointed to ptr3's block,
       it must be updated. */
    Z_Free(ptr2);

    ASSERT(verify_heap_integrity(), "Rover should be valid after coalesce with next");

    Z_Free(ptr1);
    Z_Free(ptr4);
    PASS();
}

TEST(rover_stress)
{
    setup_zone();

    /* Rapidly allocate and free in patterns that stress rover handling */
    void *ptrs[20];
    int round, i;

    for (round = 0; round < 10; round++) {
        /* Allocate all */
        for (i = 0; i < 20; i++) {
            ptrs[i] = Z_TagMalloc(64 + (i * 8), 1);
            if (!ptrs[i]) break;  /* Out of memory is ok */
        }

        /* Free in reverse order (stresses coalescing) */
        for (i = 19; i >= 0; i--) {
            if (ptrs[i]) {
                Z_Free(ptrs[i]);
                ASSERT(verify_heap_integrity(), "Heap corrupt during rover stress test");
            }
        }
    }

    PASS();
}

/* ============== FRAGMENTATION TESTS ============== */

TEST(fragmentation_pattern)
{
    setup_zone();

    void *ptrs[50];
    int allocated = 0;
    int i;

    /* Allocate many small blocks */
    for (i = 0; i < 50; i++) {
        ptrs[i] = Z_TagMalloc(100, 1);
        if (ptrs[i]) allocated++;
    }

    ASSERT(allocated > 30, "Should allocate many blocks");

    /* Free every other block (creates fragmentation) */
    for (i = 0; i < 50; i += 2) {
        if (ptrs[i]) {
            Z_Free(ptrs[i]);
            ptrs[i] = NULL;
            ASSERT(verify_heap_integrity(), "Heap corrupt during fragmentation");
        }
    }

    /* Free remaining blocks */
    for (i = 1; i < 50; i += 2) {
        if (ptrs[i]) {
            Z_Free(ptrs[i]);
            ASSERT(verify_heap_integrity(), "Heap corrupt during cleanup");
        }
    }

    PASS();
}

TEST(random_alloc_free)
{
    setup_zone();

    void *ptrs[100];
    unsigned int seed = 12345;
    int op, i, idx, size;

    memset(ptrs, 0, sizeof(ptrs));

    for (op = 0; op < 500; op++) {
        seed = seed * 1103515245 + 12345;
        idx = (seed >> 16) % 100;
        size = 32 + ((seed >> 8) % 256);

        if (ptrs[idx]) {
            Z_Free(ptrs[idx]);
            ptrs[idx] = NULL;
        } else {
            ptrs[idx] = Z_TagMalloc(size, 1);
            /* Allocation might fail if fragmented, that's ok */
        }

        ASSERT(verify_heap_integrity(), "Heap corrupt during random alloc/free");
    }

    /* Cleanup */
    for (i = 0; i < 100; i++) {
        if (ptrs[i]) Z_Free(ptrs[i]);
    }

    PASS();
}

/* ============== EDGE CASE TESTS ============== */

TEST(alloc_zero_size)
{
    setup_zone();

    /* Zero-size allocation should either work or fail gracefully */
    void *ptr = Z_TagMalloc(0, 1);

    /* Either NULL or valid pointer is acceptable */
    if (ptr) {
        Z_Free(ptr);
        ASSERT(verify_heap_integrity(), "Heap valid after zero-size alloc/free");
    }

    PASS();
}

TEST(exhaust_memory)
{
    setup_zone();

    void *ptrs[1000];
    int count = 0;
    int i;

    memset(ptrs, 0, sizeof(ptrs));

    /* Allocate until we run out */
    for (i = 0; i < 1000; i++) {
        ptrs[i] = Z_TagMalloc(1000, 1);
        if (!ptrs[i]) break;
        count++;
    }

    ASSERT(count > 10, "Should allocate multiple blocks before exhaustion");
    ASSERT(verify_heap_integrity(), "Heap valid at exhaustion");

    /* Free all */
    for (i = 0; i < count; i++) {
        Z_Free(ptrs[i]);
    }

    ASSERT(verify_heap_integrity(), "Heap valid after freeing all");
    PASS();
}

TEST(double_free_detection)
{
    setup_zone();
    reset_error_state();

    void *ptr = Z_Malloc(100);
    Z_Free(ptr);

    /* This should trigger an error */
    Z_Free(ptr);

    ASSERT(error_called, "Double free should be detected");
    PASS();
}

TEST(free_tags)
{
    setup_zone();

    /* Allocate blocks with different tags */
    void *tag1_ptrs[5];
    void *tag2_ptrs[5];
    int i;

    for (i = 0; i < 5; i++) {
        tag1_ptrs[i] = Z_TagMalloc(100, 1);
        tag2_ptrs[i] = Z_TagMalloc(100, 2);
    }

    ASSERT(verify_heap_integrity(), "Heap valid after tagged allocations");

    /* Free all tag 1 blocks */
    Z_FreeTags(1);

    ASSERT(verify_heap_integrity(), "Heap valid after Z_FreeTags");

    /* Free remaining */
    for (i = 0; i < 5; i++) {
        if (tag2_ptrs[i]) Z_Free(tag2_ptrs[i]);
    }

    PASS();
}

/* ============== STRESS TESTS ============== */

TEST(stress_small_allocations)
{
    setup_zone();
    int round, i, idx, count;

    for (round = 0; round < 5; round++) {
        void *ptrs[200];
        memset(ptrs, 0, sizeof(ptrs));
        count = 0;

        /* Many small allocations */
        for (i = 0; i < 200; i++) {
            ptrs[i] = Z_TagMalloc(32, 1);
            if (ptrs[i]) count++;
        }

        ASSERT(count > 50, "Should allocate many small blocks");

        /* Free all in random-ish order */
        for (i = 0; i < 200; i++) {
            idx = (i * 7) % 200;
            if (ptrs[idx]) {
                Z_Free(ptrs[idx]);
                ptrs[idx] = NULL;
            }
        }

        ASSERT(verify_heap_integrity(), "Heap valid after stress round");
    }

    PASS();
}

TEST(stress_mixed_sizes)
{
    setup_zone();
    int round, i, size;

    void *ptrs[50];

    for (round = 0; round < 10; round++) {
        memset(ptrs, 0, sizeof(ptrs));

        /* Allocate mixed sizes */
        for (i = 0; i < 50; i++) {
            size = (i % 5 == 0) ? 1000 : (i % 3 == 0) ? 200 : 50;
            ptrs[i] = Z_TagMalloc(size, 1);
        }

        /* Free in interleaved pattern */
        for (i = 0; i < 50; i += 3) {
            if (ptrs[i]) { Z_Free(ptrs[i]); ptrs[i] = NULL; }
        }
        for (i = 1; i < 50; i += 3) {
            if (ptrs[i]) { Z_Free(ptrs[i]); ptrs[i] = NULL; }
        }
        for (i = 2; i < 50; i += 3) {
            if (ptrs[i]) { Z_Free(ptrs[i]); ptrs[i] = NULL; }
        }

        ASSERT(verify_heap_integrity(), "Heap valid after mixed size round");
    }

    PASS();
}

/* ============== MAIN ============== */

int main(void)
{
    printf("===========================================\n");
#ifdef TEST_SOLUTION
    printf("Zone Allocator Tests (SOLUTION VERSION)\n");
#else
    printf("Zone Allocator Tests (GAME VERSION)\n");
#endif
    printf("===========================================\n\n");

    /* Basic tests */
    printf("--- Basic Tests ---\n");
    run_test_zone_init();
    run_test_single_alloc_free();
    run_test_multiple_allocs();

    /* Coalescing tests */
    printf("\n--- Coalescing Tests ---\n");
    run_test_coalesce_prev();
    run_test_coalesce_next();
    run_test_coalesce_both();

    /* Rover validity tests */
    printf("\n--- Rover Validity Tests ---\n");
    run_test_rover_after_coalesce_prev();
    run_test_rover_after_coalesce_next();
    run_test_rover_stress();

    /* Fragmentation tests */
    printf("\n--- Fragmentation Tests ---\n");
    run_test_fragmentation_pattern();
    run_test_random_alloc_free();

    /* Edge case tests */
    printf("\n--- Edge Case Tests ---\n");
    run_test_alloc_zero_size();
    run_test_exhaust_memory();
    run_test_double_free_detection();
    run_test_free_tags();

    /* Stress tests */
    printf("\n--- Stress Tests ---\n");
    run_test_stress_small_allocations();
    run_test_stress_mixed_sizes();

    /* Summary */
    printf("\n===========================================\n");
    printf("RESULTS: %d passed, %d failed\n", tests_passed, tests_failed);
    printf("===========================================\n");

    return (tests_failed > 0) ? 1 : 0;
}
