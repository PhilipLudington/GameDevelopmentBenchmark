# Bug Report: Out-of-bounds read in sprite index lookup

## Summary

The sprite lookup function uses an index to access sprite data from an array without validating that the index is within bounds, causing out-of-bounds memory reads when given an invalid sprite ID.

## Environment

- Project: Julius (Caesar III reimplementation)
- File: `src/graphics/image.c`
- Severity: High (information leak, potential crash)

## Bug Description

Sprites (images for buildings, figures, UI elements) are stored in an array. The `image_get()` function retrieves sprite data by index, but doesn't validate that the index is within the valid range.

```c
#define MAX_SPRITES 5000

typedef struct {
    int width;
    int height;
    int offset_x;
    int offset_y;
    unsigned char *data;
} sprite_info;

static sprite_info sprites[MAX_SPRITES];
static int num_sprites = 0;

sprite_info *image_get(int sprite_id)
{
    // BUG: No bounds check on sprite_id
    return &sprites[sprite_id];
}
```

When `sprite_id` is negative or >= MAX_SPRITES, this reads memory outside the array bounds.

## Steps to Reproduce

1. Call `image_get()` with an invalid sprite ID (e.g., -1 or 10000)
2. Function accesses memory outside the sprites array
3. Returns garbage data or causes crash

This can happen when:
- Loading corrupted save files with invalid sprite references
- Mod files with incorrect sprite IDs
- Integer overflow in sprite calculation

## Expected Behavior

The function should validate the sprite ID and return NULL or a placeholder for invalid indices.

## Current Behavior

Out-of-bounds array access returns garbage memory or crashes:

AddressSanitizer output:
```
==12345==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x602000005000
READ of size 24 at 0x602000005000
    #0 image_get image.c:67
```

## Relevant Code

Look at `src/graphics/image.c`:
- `image_get()` function
- Other sprite lookup functions
- Places where sprite_id comes from external data

## Suggested Fix Approach

Add bounds checking:

```c
sprite_info *image_get(int sprite_id)
{
    // Validate sprite ID
    if (sprite_id < 0 || sprite_id >= num_sprites) {
        return NULL;  // Or return a placeholder sprite
    }
    return &sprites[sprite_id];
}
```

Callers should also check for NULL returns.

## Your Task

Fix the out-of-bounds read by adding proper bounds checking. Your fix should:

1. Check that sprite_id is >= 0
2. Check that sprite_id is < num_sprites (or < MAX_SPRITES)
3. Return NULL for invalid indices
4. Not change behavior for valid sprite IDs

Provide your fix as a unified diff (patch).
