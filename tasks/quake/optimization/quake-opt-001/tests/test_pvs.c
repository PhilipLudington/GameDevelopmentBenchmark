/*
 * PVS Decompression Test Harness
 *
 * Tests correctness and performance of PVS decompression.
 * Both the unoptimized and optimized versions should produce identical output.
 *
 * Test categories:
 * 1. Correctness - Output must match reference
 * 2. Performance - Measure decompression time
 * 3. Edge cases - Handle malformed input gracefully
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

/* ============== MOCK QUAKE TYPES ============== */

typedef unsigned char byte;
typedef float vec_t;
typedef vec_t vec3_t[3];
typedef int qboolean;

#define true 1
#define false 0
#define DotProduct(a, b) ((a)[0] * (b)[0] + (a)[1] * (b)[1] + (a)[2] * (b)[2])

#define MAX_MAP_LEAFS   65536

/* Mock BSP structures */
typedef struct mplane_s {
    vec3_t  normal;
    float   dist;
    int     type;
} mplane_t;

typedef struct mnode_s {
    int         contents;
    struct mnode_s *children[2];
    mplane_t    *plane;
} mnode_t;

typedef struct mleaf_s {
    int         contents;
    byte        *compressed_vis;
} mleaf_t;

typedef struct model_s {
    int         numleafs;
    mleaf_t     *leafs;
    mnode_t     *nodes;
} model_t;

/* ============== REFERENCE IMPLEMENTATION ============== */

static byte ref_decompressed[MAX_MAP_LEAFS/8];

/*
 * Reference implementation - the "correct" version to test against.
 * This is the original Quake algorithm.
 */
byte *Ref_DecompressVis(byte *in, model_t *model)
{
    int     c;
    byte    *out;
    int     row;

    row = (model->numleafs + 7) >> 3;
    out = ref_decompressed;

    if (!in) {
        memset(ref_decompressed, 0xFF, row);
        return ref_decompressed;
    }

    do {
        if (*in) {
            *out++ = *in++;
            continue;
        }

        c = in[1];
        in += 2;
        while (c) {
            *out++ = 0;
            c--;
        }
    } while (out - ref_decompressed < row);

    return ref_decompressed;
}

/* ============== IMPLEMENTATION UNDER TEST ============== */

static byte decompressed[MAX_MAP_LEAFS/8];
static byte mod_novis[MAX_MAP_LEAFS/8];

/* Include the implementation being tested */
#ifdef TEST_OPTIMIZED
/* Optimized version includes cache - need to mock it for fair comparison */

#define PVS_CACHE_SIZE  4

typedef struct {
    byte        *compressed_vis;
    int         numleafs;
    byte        data[MAX_MAP_LEAFS/8];
    int         age;
} pvs_cache_entry_t;

static pvs_cache_entry_t pvs_cache[PVS_CACHE_SIZE];
static int pvs_cache_age = 0;

static byte *PVS_CacheLookup(byte *compressed_vis, int numleafs)
{
    int i;
    for (i = 0; i < PVS_CACHE_SIZE; i++) {
        if (pvs_cache[i].compressed_vis == compressed_vis &&
            pvs_cache[i].numleafs == numleafs) {
            pvs_cache[i].age = ++pvs_cache_age;
            return pvs_cache[i].data;
        }
    }
    return NULL;
}

static void PVS_CacheStore(byte *compressed_vis, int numleafs, byte *data)
{
    int i;
    int oldest_idx = 0;
    int oldest_age = pvs_cache[0].age;
    int row = (numleafs + 7) >> 3;

    for (i = 1; i < PVS_CACHE_SIZE; i++) {
        if (pvs_cache[i].age < oldest_age) {
            oldest_age = pvs_cache[i].age;
            oldest_idx = i;
        }
    }

    pvs_cache[oldest_idx].compressed_vis = compressed_vis;
    pvs_cache[oldest_idx].numleafs = numleafs;
    pvs_cache[oldest_idx].age = ++pvs_cache_age;
    memcpy(pvs_cache[oldest_idx].data, data, row);
}

static void PVS_CacheClear(void)
{
    int i;
    for (i = 0; i < PVS_CACHE_SIZE; i++) {
        pvs_cache[i].compressed_vis = NULL;
        pvs_cache[i].numleafs = 0;
        pvs_cache[i].age = 0;
    }
    pvs_cache_age = 0;
}

byte *Mod_DecompressVis(byte *in, model_t *model)
{
    int         c;
    byte        *out;
    byte        *end;
    int         row;
    unsigned    *out32;

    row = (model->numleafs + 7) >> 3;
    out = decompressed;
    end = decompressed + row;

    if (!in) {
        memset(decompressed, 0xFF, row);
        return decompressed;
    }

    /* Skip cache for fair performance comparison */
    #ifndef ENABLE_CACHE
    /* Cache disabled for baseline comparison */
    #else
    byte *cached = PVS_CacheLookup(in, model->numleafs);
    if (cached) {
        return cached;
    }
    #endif

    while (out < end) {
        if (*in) {
            byte b1 = *in++;
            *out++ = b1;

            if (out < end && *in) {
                *out++ = *in++;
                if (out < end && *in) {
                    *out++ = *in++;
                    if (out < end && *in) {
                        *out++ = *in++;
                    }
                }
            }
            continue;
        }

        c = in[1];
        in += 2;

        if (c <= 4) {
            switch (c) {
                case 4: *out++ = 0;
                case 3: *out++ = 0;
                case 2: *out++ = 0;
                case 1: *out++ = 0;
                case 0: break;
            }
            continue;
        }

        if (c <= 16) {
            while (((uintptr_t)out & 3) && c > 0) {
                *out++ = 0;
                c--;
            }

            out32 = (unsigned *)out;
            while (c >= 4) {
                *out32++ = 0;
                c -= 4;
            }
            out = (byte *)out32;

            while (c > 0) {
                *out++ = 0;
                c--;
            }
            continue;
        }

        memset(out, 0, c);
        out += c;
    }

    #ifdef ENABLE_CACHE
    PVS_CacheStore(in, model->numleafs, decompressed);
    #endif

    return decompressed;
}

#else
/* Unoptimized version */

byte *Mod_DecompressVis(byte *in, model_t *model)
{
    int     c;
    byte    *out;
    int     row;

    row = (model->numleafs + 7) >> 3;
    out = decompressed;

    if (!in) {
        memset(decompressed, 0xFF, row);
        return decompressed;
    }

    do {
        if (*in) {
            *out++ = *in++;
            continue;
        }

        c = in[1];
        in += 2;
        while (c) {
            *out++ = 0;
            c--;
        }
    } while (out - decompressed < row);

    return decompressed;
}

static void PVS_CacheClear(void) { /* No-op for unoptimized */ }

#endif

/* ============== TEST DATA GENERATION ============== */

/*
 * Generate compressed PVS data for testing.
 *
 * pattern:
 *   0 = Random mix of zeros and non-zeros
 *   1 = Mostly visible (indoor - few zero runs)
 *   2 = Sparse visibility (outdoor - long zero runs)
 *   3 = Alternating (worst case for branches)
 */
byte *generate_test_pvs(int numleafs, int pattern, int *compressed_size)
{
    static byte compressed[MAX_MAP_LEAFS];
    byte *out = compressed;
    int row = (numleafs + 7) >> 3;
    int pos = 0;

    srand(pattern * 12345 + numleafs);

    while (pos < row) {
        switch (pattern) {
            case 0: /* Random */
                if (rand() % 3 == 0) {
                    /* Zero run */
                    int run = 1 + rand() % 16;
                    if (pos + run > row) run = row - pos;
                    *out++ = 0;
                    *out++ = run;
                    pos += run;
                } else {
                    /* Non-zero byte */
                    *out++ = (rand() % 255) + 1;
                    pos++;
                }
                break;

            case 1: /* Mostly visible (indoor) */
                if (rand() % 10 == 0) {
                    /* Short zero run */
                    int run = 1 + rand() % 3;
                    if (pos + run > row) run = row - pos;
                    *out++ = 0;
                    *out++ = run;
                    pos += run;
                } else {
                    *out++ = 0xFF;  /* All visible */
                    pos++;
                }
                break;

            case 2: /* Sparse visibility (outdoor) */
                if (rand() % 3 == 0) {
                    /* Non-zero byte */
                    *out++ = (rand() % 255) + 1;
                    pos++;
                } else {
                    /* Long zero run */
                    int run = 8 + rand() % 48;
                    if (pos + run > row) run = row - pos;
                    *out++ = 0;
                    *out++ = run;
                    pos += run;
                }
                break;

            case 3: /* Alternating (branch stress test) */
                if (pos % 2 == 0) {
                    *out++ = 0x55;  /* Alternating bits */
                    pos++;
                } else {
                    *out++ = 0;
                    *out++ = 1;
                    pos++;
                }
                break;
        }
    }

    *compressed_size = out - compressed;
    return compressed;
}

/* ============== TESTS ============== */

int tests_run = 0;
int tests_passed = 0;

#define TEST(name) void test_##name(void)
#define RUN_TEST(name) do { \
    printf("  %-45s ", #name); \
    fflush(stdout); \
    test_##name(); \
    tests_run++; \
} while(0)

#define ASSERT(cond) do { \
    if (!(cond)) { \
        printf("FAIL\n    Assertion failed: %s\n", #cond); \
        return; \
    } \
} while(0)

#define ASSERT_MSG(cond, msg) do { \
    if (!(cond)) { \
        printf("FAIL\n    %s\n", msg); \
        return; \
    } \
} while(0)

#define PASS() do { printf("PASS\n"); tests_passed++; } while(0)

/* High resolution timer */
static double get_time_us(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1e6 + ts.tv_nsec / 1e3;
}

/* ---- Correctness Tests ---- */

TEST(basic_decompression)
{
    model_t model;
    byte compressed[] = {0xFF, 0x00, 0x02, 0x0F};  /* FF, skip 2 zeros, 0F */
    byte *result;
    byte *reference;

    model.numleafs = 32;  /* 4 bytes */

    reference = Ref_DecompressVis(compressed, &model);
    result = Mod_DecompressVis(compressed, &model);

    ASSERT(memcmp(result, reference, 4) == 0);
    PASS();
}

TEST(null_input_all_visible)
{
    model_t model;
    byte *result;
    int row;
    int i;

    model.numleafs = 64;
    row = (model.numleafs + 7) >> 3;

    result = Mod_DecompressVis(NULL, &model);

    for (i = 0; i < row; i++) {
        ASSERT_MSG(result[i] == 0xFF, "NULL input should produce all-visible");
    }
    PASS();
}

TEST(random_data_matches_reference)
{
    model_t model;
    int numleafs_tests[] = {64, 256, 1024, 4096, 8192};
    int i, j;

    for (i = 0; i < 5; i++) {
        model.numleafs = numleafs_tests[i];
        int row = (model.numleafs + 7) >> 3;

        for (j = 0; j < 4; j++) {  /* All patterns */
            int compressed_size;
            byte *compressed = generate_test_pvs(model.numleafs, j, &compressed_size);

            /* Make a copy since decompression modifies pointer */
            byte compressed_copy1[MAX_MAP_LEAFS];
            byte compressed_copy2[MAX_MAP_LEAFS];
            memcpy(compressed_copy1, compressed, compressed_size);
            memcpy(compressed_copy2, compressed, compressed_size);

            byte *reference = Ref_DecompressVis(compressed_copy1, &model);
            byte *result = Mod_DecompressVis(compressed_copy2, &model);

            if (memcmp(result, reference, row) != 0) {
                printf("FAIL\n    Mismatch for numleafs=%d pattern=%d\n",
                       model.numleafs, j);
                return;
            }
        }
    }
    PASS();
}

TEST(long_zero_runs)
{
    model_t model;
    byte compressed[256];
    byte *out = compressed;
    int i;

    model.numleafs = 2048;  /* 256 bytes */

    /* Create data with long zero runs (up to 255) */
    *out++ = 0xFF;          /* 1 byte visible */
    *out++ = 0; *out++ = 64;   /* 64 zeros */
    *out++ = 0x0F;          /* 1 byte partial */
    *out++ = 0; *out++ = 128;  /* 128 zeros */
    *out++ = 0xFF;          /* 1 byte visible */
    *out++ = 0; *out++ = 61;   /* Fill remaining (256-1-64-1-128-1 = 61) */

    byte compressed_copy1[256], compressed_copy2[256];
    memcpy(compressed_copy1, compressed, sizeof(compressed));
    memcpy(compressed_copy2, compressed, sizeof(compressed));

    byte *reference = Ref_DecompressVis(compressed_copy1, &model);
    byte *result = Mod_DecompressVis(compressed_copy2, &model);

    ASSERT(memcmp(result, reference, 256) == 0);
    PASS();
}

TEST(single_byte_runs)
{
    model_t model;
    byte compressed[128];
    byte *out = compressed;
    int i;

    model.numleafs = 256;  /* 32 bytes */

    /* All single-byte zero runs */
    for (i = 0; i < 16; i++) {
        *out++ = 0xFF;
        *out++ = 0; *out++ = 1;
    }

    byte compressed_copy1[128], compressed_copy2[128];
    memcpy(compressed_copy1, compressed, sizeof(compressed));
    memcpy(compressed_copy2, compressed, sizeof(compressed));

    byte *reference = Ref_DecompressVis(compressed_copy1, &model);
    byte *result = Mod_DecompressVis(compressed_copy2, &model);

    ASSERT(memcmp(result, reference, 32) == 0);
    PASS();
}

TEST(all_zeros)
{
    model_t model;
    byte compressed[] = {0, 64};  /* 64 zero bytes */

    model.numleafs = 512;  /* 64 bytes */

    byte compressed_copy1[2], compressed_copy2[2];
    memcpy(compressed_copy1, compressed, 2);
    memcpy(compressed_copy2, compressed, 2);

    byte *reference = Ref_DecompressVis(compressed_copy1, &model);
    byte *result = Mod_DecompressVis(compressed_copy2, &model);

    ASSERT(memcmp(result, reference, 64) == 0);

    /* Verify all zeros */
    int i;
    for (i = 0; i < 64; i++) {
        ASSERT(result[i] == 0);
    }
    PASS();
}

/* ---- Performance Tests ---- */

TEST(performance_indoor_map)
{
    model_t model;
    int iterations = 10000;
    double start, end;
    double total_us;
    int i;

    model.numleafs = 4096;  /* Medium-sized map */

    int compressed_size;
    byte *original = generate_test_pvs(model.numleafs, 1, &compressed_size);  /* Indoor pattern */

    byte compressed[MAX_MAP_LEAFS];

    PVS_CacheClear();  /* Ensure no cache interference */

    start = get_time_us();
    for (i = 0; i < iterations; i++) {
        memcpy(compressed, original, compressed_size);
        Mod_DecompressVis(compressed, &model);
        PVS_CacheClear();  /* Clear cache each iteration */
    }
    end = get_time_us();

    total_us = end - start;
    double avg_us = total_us / iterations;

    printf("PASS (%.2f us avg, %d leaves)\n", avg_us, model.numleafs);
    tests_passed++;
}

TEST(performance_outdoor_map)
{
    model_t model;
    int iterations = 10000;
    double start, end;
    double total_us;
    int i;

    model.numleafs = 8192;  /* Large outdoor map */

    int compressed_size;
    byte *original = generate_test_pvs(model.numleafs, 2, &compressed_size);  /* Sparse pattern */

    byte compressed[MAX_MAP_LEAFS];

    PVS_CacheClear();

    start = get_time_us();
    for (i = 0; i < iterations; i++) {
        memcpy(compressed, original, compressed_size);
        Mod_DecompressVis(compressed, &model);
        PVS_CacheClear();
    }
    end = get_time_us();

    total_us = end - start;
    double avg_us = total_us / iterations;

    printf("PASS (%.2f us avg, %d leaves)\n", avg_us, model.numleafs);
    tests_passed++;
}

TEST(performance_worst_case)
{
    model_t model;
    int iterations = 10000;
    double start, end;
    double total_us;
    int i;

    model.numleafs = 4096;

    int compressed_size;
    byte *original = generate_test_pvs(model.numleafs, 3, &compressed_size);  /* Alternating */

    byte compressed[MAX_MAP_LEAFS];

    PVS_CacheClear();

    start = get_time_us();
    for (i = 0; i < iterations; i++) {
        memcpy(compressed, original, compressed_size);
        Mod_DecompressVis(compressed, &model);
        PVS_CacheClear();
    }
    end = get_time_us();

    total_us = end - start;
    double avg_us = total_us / iterations;

    printf("PASS (%.2f us avg, %d leaves)\n", avg_us, model.numleafs);
    tests_passed++;
}

#ifdef ENABLE_CACHE
TEST(performance_cache_benefit)
{
    model_t model;
    int iterations = 10000;
    double start, end;
    int i;

    model.numleafs = 4096;

    int compressed_size;
    byte *original = generate_test_pvs(model.numleafs, 0, &compressed_size);

    /* Keep compressed data stable so cache works */
    static byte compressed[MAX_MAP_LEAFS];
    memcpy(compressed, original, compressed_size);

    PVS_CacheClear();

    /* First call populates cache */
    Mod_DecompressVis(compressed, &model);

    /* Subsequent calls should hit cache */
    start = get_time_us();
    for (i = 0; i < iterations; i++) {
        Mod_DecompressVis(compressed, &model);
    }
    end = get_time_us();

    double total_us = end - start;
    double avg_us = total_us / iterations;

    printf("PASS (%.2f us avg with cache)\n", avg_us);
    tests_passed++;
}
#endif

/* ---- Edge Case Tests ---- */

TEST(minimum_size_map)
{
    model_t model;
    byte compressed[] = {0xFF};

    model.numleafs = 8;  /* 1 byte */

    byte compressed_copy1[1], compressed_copy2[1];
    compressed_copy1[0] = compressed[0];
    compressed_copy2[0] = compressed[0];

    byte *reference = Ref_DecompressVis(compressed_copy1, &model);
    byte *result = Mod_DecompressVis(compressed_copy2, &model);

    ASSERT(result[0] == reference[0]);
    PASS();
}

TEST(maximum_size_map)
{
    model_t model;
    int compressed_size;
    byte *original;

    model.numleafs = MAX_MAP_LEAFS;

    original = generate_test_pvs(model.numleafs, 0, &compressed_size);

    byte *compressed1 = malloc(compressed_size);
    byte *compressed2 = malloc(compressed_size);
    memcpy(compressed1, original, compressed_size);
    memcpy(compressed2, original, compressed_size);

    byte *reference = Ref_DecompressVis(compressed1, &model);
    byte *result = Mod_DecompressVis(compressed2, &model);

    int row = (model.numleafs + 7) >> 3;
    ASSERT(memcmp(result, reference, row) == 0);

    free(compressed1);
    free(compressed2);
    PASS();
}

TEST(unaligned_leaf_count)
{
    model_t model;
    int counts[] = {7, 9, 15, 17, 31, 33, 63, 65, 127, 129};
    int i;

    for (i = 0; i < 10; i++) {
        model.numleafs = counts[i];
        int row = (model.numleafs + 7) >> 3;

        int compressed_size;
        byte *original = generate_test_pvs(model.numleafs, 0, &compressed_size);

        byte compressed1[MAX_MAP_LEAFS], compressed2[MAX_MAP_LEAFS];
        memcpy(compressed1, original, compressed_size);
        memcpy(compressed2, original, compressed_size);

        byte *reference = Ref_DecompressVis(compressed1, &model);
        byte *result = Mod_DecompressVis(compressed2, &model);

        if (memcmp(result, reference, row) != 0) {
            printf("FAIL\n    Mismatch for numleafs=%d\n", model.numleafs);
            return;
        }
    }
    PASS();
}

/* ============== MAIN ============== */

int main(int argc, char **argv)
{
    printf("PVS Decompression Tests\n");
    printf("=======================\n\n");

#ifdef TEST_OPTIMIZED
    printf("Testing: OPTIMIZED version\n");
#ifdef ENABLE_CACHE
    printf("Cache: ENABLED\n");
#else
    printf("Cache: DISABLED (for fair comparison)\n");
#endif
#else
    printf("Testing: UNOPTIMIZED version\n");
#endif
    printf("\n");

    printf("Correctness Tests:\n");
    RUN_TEST(basic_decompression);
    RUN_TEST(null_input_all_visible);
    RUN_TEST(random_data_matches_reference);
    RUN_TEST(long_zero_runs);
    RUN_TEST(single_byte_runs);
    RUN_TEST(all_zeros);
    printf("\n");

    printf("Performance Tests:\n");
    RUN_TEST(performance_indoor_map);
    RUN_TEST(performance_outdoor_map);
    RUN_TEST(performance_worst_case);
#ifdef ENABLE_CACHE
    RUN_TEST(performance_cache_benefit);
#endif
    printf("\n");

    printf("Edge Case Tests:\n");
    RUN_TEST(minimum_size_map);
    RUN_TEST(maximum_size_map);
    RUN_TEST(unaligned_leaf_count);
    printf("\n");

    printf("=======================\n");
    printf("Results: %d/%d tests passed\n", tests_passed, tests_run);

    return (tests_passed == tests_run) ? 0 : 1;
}
