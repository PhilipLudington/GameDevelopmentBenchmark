/*
 * Test for julius-018: Out-of-bounds read in sprite index lookup
 *
 * This test verifies that sprite lookup properly validates indices
 * and doesn't access memory out of bounds.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SPRITES 100  /* Small for testing */

typedef struct {
    int width;
    int height;
    int offset_x;
    int offset_y;
    unsigned char *data;
} sprite_info;

static sprite_info *sprites = NULL;
static int num_sprites = 0;

/* Initialize sprite array */
void sprites_init(int count)
{
    sprites = calloc(count, sizeof(sprite_info));
    num_sprites = count;

    /* Fill with test data */
    for (int i = 0; i < count; i++) {
        sprites[i].width = 32 + i;
        sprites[i].height = 32 + i;
        sprites[i].offset_x = i;
        sprites[i].offset_y = i;
        sprites[i].data = NULL;
    }
}

/* Cleanup */
void sprites_cleanup(void)
{
    free(sprites);
    sprites = NULL;
    num_sprites = 0;
}

/*
 * Simulates the buggy/fixed sprite lookup
 */
sprite_info *image_get(int sprite_id)
{
#ifdef BUGGY_VERSION
    /* BUG: No bounds check */
    return &sprites[sprite_id];
#else
    /* FIXED: Validate sprite ID */
    if (sprite_id < 0 || sprite_id >= num_sprites) {
        return NULL;
    }
    return &sprites[sprite_id];
#endif
}

/* Test valid sprite access */
int test_valid_access(void)
{
    sprites_init(50);

    sprite_info *sprite = image_get(25);

    if (sprite == NULL) {
        printf("FAIL: Valid sprite returned NULL\n");
        sprites_cleanup();
        return 1;
    }

    if (sprite->width != 32 + 25) {
        printf("FAIL: Sprite has wrong width: %d\n", sprite->width);
        sprites_cleanup();
        return 1;
    }

    sprites_cleanup();
    printf("PASS: Valid sprite access\n");
    return 0;
}

/* Test first and last valid indices */
int test_boundary_access(void)
{
    sprites_init(50);

    /* First sprite (index 0) */
    sprite_info *first = image_get(0);
    if (first == NULL || first->width != 32) {
        printf("FAIL: First sprite access failed\n");
        sprites_cleanup();
        return 1;
    }

    /* Last sprite (index 49) */
    sprite_info *last = image_get(49);
    if (last == NULL || last->width != 32 + 49) {
        printf("FAIL: Last sprite access failed\n");
        sprites_cleanup();
        return 1;
    }

    sprites_cleanup();
    printf("PASS: Boundary sprite access\n");
    return 0;
}

/* Test negative index - triggers OOB if buggy */
int test_negative_index(void)
{
    sprites_init(50);

    printf("Testing negative sprite index (-1)...\n");

    sprite_info *sprite = image_get(-1);

    if (sprite != NULL) {
        printf("FAIL: Negative index should return NULL\n");
        sprites_cleanup();
        return 1;
    }

    sprites_cleanup();
    printf("PASS: Negative index handled\n");
    return 0;
}

/* Test out-of-bounds high index - triggers OOB if buggy */
int test_oob_high_index(void)
{
    sprites_init(50);

    printf("Testing out-of-bounds index (100, max is 50)...\n");

    /* This will trigger heap-buffer-overflow if buggy */
    sprite_info *sprite = image_get(100);

    if (sprite != NULL) {
        printf("FAIL: OOB index should return NULL\n");
        sprites_cleanup();
        return 1;
    }

    sprites_cleanup();
    printf("PASS: OOB high index handled\n");
    return 0;
}

/* Test exactly at the boundary (num_sprites) */
int test_exact_boundary(void)
{
    sprites_init(50);

    /* Index 50 is out of bounds (valid range is 0-49) */
    sprite_info *sprite = image_get(50);

    if (sprite != NULL) {
        printf("FAIL: Index at num_sprites should return NULL\n");
        sprites_cleanup();
        return 1;
    }

    sprites_cleanup();
    printf("PASS: Exact boundary handled\n");
    return 0;
}

/* Test very large index */
int test_very_large_index(void)
{
    sprites_init(50);

    printf("Testing very large sprite index (999999)...\n");

    sprite_info *sprite = image_get(999999);

    if (sprite != NULL) {
        printf("FAIL: Very large index should return NULL\n");
        sprites_cleanup();
        return 1;
    }

    sprites_cleanup();
    printf("PASS: Very large index handled\n");
    return 0;
}

int main(void)
{
    int failures = 0;

    printf("=== Sprite Out-of-Bounds Test ===\n\n");

#ifdef BUGGY_VERSION
    printf("Testing BUGGY version (expect ASan to catch OOB read)\n\n");
#else
    printf("Testing FIXED version (should pass all tests)\n\n");
#endif

    failures += test_valid_access();
    failures += test_boundary_access();
    failures += test_negative_index();
    failures += test_oob_high_index();
    failures += test_exact_boundary();
    failures += test_very_large_index();

    printf("\n=== Results: %d failures ===\n", failures);

    return failures;
}
