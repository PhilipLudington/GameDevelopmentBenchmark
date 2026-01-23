# Zone Memory Allocator Corruption on Fragmentation

## Problem Description

Players and server operators have reported random crashes after extended play sessions. The crashes occur more frequently on servers that have been running for hours with many players joining and leaving. The crash manifests as:

- Random data corruption in game state
- Crashes with invalid memory access
- Occasionally, infinite loops during level loads

Investigation has traced the issue to Quake's zone memory allocator (`zone.c`). The zone allocator is a custom memory management system that provides deterministic allocation within a fixed-size memory pool.

## Background: Quake's Zone Allocator

Quake uses a zone allocator for dynamic memory allocation. Key characteristics:

1. **Fixed pool**: Memory is allocated from a fixed-size buffer
2. **Linked list**: Free and allocated blocks form a doubly-linked list
3. **Block structure**: Each block has a header containing:
   - Size (including header)
   - Tag (indicates allocation type)
   - Pointers to previous/next blocks
4. **Coalescing**: Adjacent free blocks should be merged

```c
typedef struct memblock_s {
    int     size;           // including the header and padding
    int     tag;            // 0 = free, positive = allocated
    int     id;             // debugging id
    struct memblock_s *next;
    struct memblock_s *prev;
    int     pad;            // padding for alignment
} memblock_t;

typedef struct {
    int     size;           // total bytes malloced
    memblock_t blocklist;   // start/end sentinel
    memblock_t *rover;      // roving pointer for next-fit
} memzone_t;
```

## The Bug

The bug occurs during memory fragmentation. When:
1. Many small allocations and frees create a fragmented heap
2. The rover pointer (used for next-fit allocation) can point to freed memory
3. Block coalescing has a boundary condition error

The symptoms include:
- Rover pointing into freed/coalesced memory
- Size calculations that can overflow
- Corruption of adjacent block headers during coalesce

## Files

The buggy zone allocator is in `game/zone.c`. Focus on:
- `Z_Free()` - the free function with coalescing
- `Z_Malloc()` - the allocation function
- `Z_CheckHeap()` - heap validation (use this to debug)

## What to Look For

1. **Rover invalidation**: When freeing a block, if the rover points to an adjacent block that gets coalesced, the rover becomes invalid
2. **Size overflow**: When coalescing, the new size is computed but may overflow if blocks are large
3. **Sentinel corruption**: The block list uses a sentinel node - ensure it's never corrupted during coalesce
4. **Off-by-one**: Block boundary calculations may have off-by-one errors

## Expected Behavior

After your fix:
- The zone allocator should survive millions of alloc/free cycles
- Z_CheckHeap() should never fail
- No memory corruption even under heavy fragmentation
- Performance should remain unchanged

## Testing

```bash
# Compile the test harness
make test_zone

# Run the zone allocator stress tests
./test_zone
```

The tests include:
- Random allocation/free patterns
- Stress test with many small allocations
- Fragmentation-heavy workload
- Heap validation after each operation

## Hints

1. When coalescing blocks, update the rover if it points to a block being merged
2. Check for integer overflow when computing block sizes
3. The sentinel node at the beginning of the block list has special handling
4. Use Z_CheckHeap() liberally during debugging to catch corruption early
5. The fix likely involves 2-3 conditional checks during Z_Free()
