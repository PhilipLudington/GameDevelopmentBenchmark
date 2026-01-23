# Implement Procedural Terrain Generation

## Overview

Quake's BSP-based levels are ideal for indoor environments but poorly suited for large outdoor areas. The static geometry means:
- Limited level size (BSP node limits)
- No dynamic terrain modification
- Memory-intensive for open spaces

Implement a heightmap-based terrain system with LOD (Level of Detail) to enable expansive outdoor environments that seamlessly integrate with Quake's existing BSP system.

## Background: Heightmap Terrain

A heightmap is a 2D grid where each cell stores an elevation value:

```c
typedef struct {
    int         width, height;  // Grid dimensions
    float       *heights;       // height[y * width + x]
    float       cellSize;       // World units per grid cell
    float       heightScale;    // Multiplier for height values
    vec3_t      origin;         // World position of grid corner
} heightmap_t;

// Get height at world position (with bilinear interpolation)
float Heightmap_GetHeight(heightmap_t *hm, float x, float z)
{
    // Transform world coords to grid coords
    float gx = (x - hm->origin[0]) / hm->cellSize;
    float gz = (z - hm->origin[2]) / hm->cellSize;

    // Bilinear interpolation between 4 neighboring cells
    // ...
}
```

## Implementation Requirements

### 1. Terrain Data Structure

```c
#define TERRAIN_CHUNK_SIZE 33  // 32 quads + 1 for seamless edges
#define MAX_LOD_LEVELS 5

typedef struct terrain_chunk_s {
    int             lod;            // Current LOD level
    int             gridX, gridZ;   // Position in chunk grid
    vec3_t          mins, maxs;     // Bounding box
    vertex_t        *vertices;      // Vertex buffer
    int             *indices;       // Index buffer
    int             numIndices;
    qboolean        dirty;          // Needs rebuild
    struct terrain_chunk_s *neighbors[4];  // N, E, S, W
} terrain_chunk_t;

typedef struct {
    heightmap_t     heightmap;
    terrain_chunk_t **chunks;
    int             numChunksX, numChunksZ;
    texture_t       *textures[4];   // Terrain layer textures
    byte            *splatmap;      // Which texture for each cell
} terrain_t;
```

### 2. LOD Mesh Generation

Different LOD levels have different vertex densities:
- LOD 0: Full detail (32x32 quads per chunk)
- LOD 1: Half detail (16x16 quads)
- LOD 2: Quarter detail (8x8 quads)
- etc.

```c
void Terrain_BuildChunkMesh(terrain_chunk_t *chunk, heightmap_t *hm, int lod)
{
    int step = 1 << lod;  // 1, 2, 4, 8...
    int size = (TERRAIN_CHUNK_SIZE - 1) / step + 1;

    // Generate vertices
    for (int z = 0; z < size; z++) {
        for (int x = 0; x < size; x++) {
            int gx = chunk->gridX * (TERRAIN_CHUNK_SIZE - 1) + x * step;
            int gz = chunk->gridZ * (TERRAIN_CHUNK_SIZE - 1) + z * step;

            vec3_t pos;
            pos[0] = hm->origin[0] + gx * hm->cellSize;
            pos[2] = hm->origin[2] + gz * hm->cellSize;
            pos[1] = hm->heights[gz * hm->width + gx] * hm->heightScale;

            // Calculate normal from neighboring heights
            // ...
        }
    }

    // Generate indices (triangles)
    // ...
}
```

### 3. LOD Selection

Choose LOD based on distance from camera:

```c
int Terrain_CalculateLOD(terrain_chunk_t *chunk, vec3_t cameraPos)
{
    // Distance to chunk center
    vec3_t center;
    center[0] = (chunk->mins[0] + chunk->maxs[0]) * 0.5f;
    center[1] = (chunk->mins[1] + chunk->maxs[1]) * 0.5f;
    center[2] = (chunk->mins[2] + chunk->maxs[2]) * 0.5f;

    float dist = VectorDistance(cameraPos, center);

    // LOD thresholds
    if (dist < 256)  return 0;  // Full detail
    if (dist < 512)  return 1;
    if (dist < 1024) return 2;
    if (dist < 2048) return 3;
    return 4;
}
```

### 4. Seamless LOD Transitions (Stitching)

When adjacent chunks have different LOD levels, gaps appear at boundaries. Fix with:

```c
// Adjust edge vertices to match lower-LOD neighbor
void Terrain_StitchEdge(terrain_chunk_t *chunk, int edge, int neighborLOD)
{
    if (neighborLOD <= chunk->lod)
        return;  // No stitching needed

    // For each vertex on this edge that doesn't exist in neighbor,
    // interpolate between the neighboring vertices that DO exist
}
```

### 5. Integration with BSP

- Terrain exists outside BSP leafs (in "outdoor" areas)
- Use a special leaf type to indicate terrain region
- Collision detection samples heightmap directly

## Files to Modify

- `game/r_main.c` - Main rendering, add terrain pass
- `game/gl_rmain.c` - OpenGL terrain rendering
- `game/model.c` - Loading terrain data
- Create new: `game/terrain.c` - Terrain system

## Mathematics Reference

### Bilinear Interpolation
```c
float Bilerp(float tl, float tr, float bl, float br, float u, float v)
{
    float top = tl + (tr - tl) * u;
    float bottom = bl + (br - bl) * u;
    return top + (bottom - top) * v;
}
```

### Normal from Heightmap
```c
void Heightmap_GetNormal(heightmap_t *hm, int x, int z, vec3_t normal)
{
    float hL = hm->heights[z * hm->width + (x-1)];
    float hR = hm->heights[z * hm->width + (x+1)];
    float hD = hm->heights[(z-1) * hm->width + x];
    float hU = hm->heights[(z+1) * hm->width + x];

    normal[0] = hL - hR;
    normal[1] = 2.0f * hm->cellSize;
    normal[2] = hD - hU;
    VectorNormalize(normal);
}
```

## Testing

```bash
# Compile and run tests
make test_terrain

./test_terrain
```

Tests include:
- Height sampling accuracy
- LOD selection distance thresholds
- Edge stitching (no gaps at LOD boundaries)
- Collision with terrain surface
- Performance with various chunk counts
- Visual regression for terrain rendering

## Hints

1. Start with a single chunk, no LOD - get rendering working first
2. Use vertex normals for lighting, not face normals (smoother appearance)
3. For texturing, start with a single texture before implementing splatmaps
4. The index buffer for each LOD can be pre-computed and reused
5. Consider using triangle strips instead of triangle lists for better cache performance
6. Geomorphing (smooth LOD transitions) requires storing both current and target positions
