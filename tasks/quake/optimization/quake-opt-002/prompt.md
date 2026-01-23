# Optimize Entity Linked List Traversal with Spatial Hash

## Problem Description

Servers with many entities (monsters, projectiles, items) experience severe performance degradation. The physics update becomes the bottleneck because collision detection iterates through all entities to find potential collisions.

Current behavior:
- 64 entities: 16ms physics update (acceptable)
- 128 entities: 65ms physics update (server struggles)
- 256 entities: 260ms physics update (unplayable)

The O(n²) growth is due to the naive linked list traversal in `SV_Move()` and related functions.

## Background: Quake's World Collision

Quake's `world.c` handles collision detection:

```c
// Current implementation - iterates all entities
typedef struct areanode_s {
    int         axis;       // -1 = leaf node
    float       dist;
    struct areanode_s *children[2];
    link_t      trigger_edicts;
    link_t      solid_edicts;
} areanode_t;

// Called for each moving entity to find collisions
edict_t *SV_TestEntityPosition(edict_t *ent)
{
    trace_t trace;
    trace = SV_Move(ent->v.origin, ent->v.mins, ent->v.maxs,
                    ent->v.origin, 0, ent);
    // ...
}
```

The area nodes form a BSP-like tree, but the entity lists at leaves can grow large.

## The Optimization

Replace the linked list with a spatial hash grid:

```c
// Proposed structure
#define GRID_CELL_SIZE 64.0f  // Units per cell
#define GRID_HASH_SIZE 1024   // Hash table size

typedef struct grid_cell_s {
    edict_t *entities[MAX_CELL_ENTITIES];
    int count;
    struct grid_cell_s *overflow;  // For cells with many entities
} grid_cell_t;

typedef struct {
    grid_cell_t cells[GRID_HASH_SIZE];
} spatial_hash_t;

// Hash function: combine x, y, z cell coordinates
unsigned int hash_position(vec3_t pos) {
    int cx = (int)(pos[0] / GRID_CELL_SIZE);
    int cy = (int)(pos[1] / GRID_CELL_SIZE);
    int cz = (int)(pos[2] / GRID_CELL_SIZE);
    return (cx * 73856093 ^ cy * 19349663 ^ cz * 83492791) % GRID_HASH_SIZE;
}
```

## Files

The entity management is in `game/world.c`. Modify:
- `SV_LinkEdict()` - Add entity to spatial structure
- `SV_UnlinkEdict()` - Remove entity from spatial structure
- `SV_AreaEdicts()` - Find entities in a region
- Add new spatial hash functions

## Performance Target

After optimization:
- 256 entities: < 20ms physics update
- 512 entities: < 40ms physics update
- Linear scaling O(n) instead of O(n²)

## Implementation Requirements

1. **Insertion**: O(1) - hash position, add to cell
2. **Removal**: O(k) where k is entities in cell (typically small)
3. **Query**: O(k) for nearby cells only
4. **Large entities**: Must handle entities spanning multiple cells
5. **Dynamic updates**: Moving entities must update their cell membership

## Challenges

1. **Cell boundaries**: An entity's bounding box may span multiple cells
2. **Memory management**: Cells with many entities need overflow handling
3. **Hash collisions**: Distant cells may hash to same bucket
4. **Backwards compatibility**: Must produce identical collision results

## Testing

```bash
# Compile with optimization
make test_world_perf CFLAGS="-O2"

# Run performance tests
./test_world_perf
```

Tests include:
- Correctness: collision results match original implementation
- Performance: time physics updates with varying entity counts
- Edge cases: entities at cell boundaries, very large entities
- Stress test: rapid entity creation/destruction

## Hints

1. Start by implementing the spatial hash alongside the existing system
2. Verify correctness by comparing results before switching
3. The hash table size should be tuned for typical map sizes
4. Consider a two-level structure: coarse hash + fine list
5. Profile to find the optimal cell size for your test maps
