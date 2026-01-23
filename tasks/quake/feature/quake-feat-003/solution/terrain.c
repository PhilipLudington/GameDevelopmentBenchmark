/*
 * terrain.c - Procedural terrain system (reference implementation)
 *
 * Complete heightmap terrain system with chunked LOD.
 * All functions fully implemented and correct.
 */

#include "quakedef.h"

/* ============== HEIGHTMAP OPERATIONS ============== */

/*
 * Heightmap_Create - Allocate and initialize a heightmap
 */
void Heightmap_Create(heightmap_t *hm, int width, int height, float cellSize, float heightScale)
{
    hm->width = width;
    hm->height = height;
    hm->cellSize = cellSize;
    hm->heightScale = heightScale;
    hm->heights = (float *)calloc(width * height, sizeof(float));
    VectorSet(hm->origin, 0.0f, 0.0f, 0.0f);
}

/*
 * Heightmap_Destroy - Free heightmap memory
 */
void Heightmap_Destroy(heightmap_t *hm)
{
    if (hm->heights) {
        free(hm->heights);
        hm->heights = NULL;
    }
    hm->width = 0;
    hm->height = 0;
}

/*
 * Heightmap_SetHeight - Set height at grid position
 */
void Heightmap_SetHeight(heightmap_t *hm, int x, int z, float height)
{
    if (x < 0 || x >= hm->width || z < 0 || z >= hm->height)
        return;
    hm->heights[z * hm->width + x] = height;
}

/*
 * Heightmap_GetHeightAtGrid - Get raw height at integer grid position
 */
float Heightmap_GetHeightAtGrid(heightmap_t *hm, int x, int z)
{
    /* Clamp to valid range */
    if (x < 0) x = 0;
    if (x >= hm->width) x = hm->width - 1;
    if (z < 0) z = 0;
    if (z >= hm->height) z = hm->height - 1;

    return hm->heights[z * hm->width + x] * hm->heightScale;
}

/*
 * Bilerp - Bilinear interpolation helper
 *
 * Correctly interpolates between 4 corners:
 *   tl----tr   (z=0, top row)
 *    |    |
 *   bl----br   (z=1, bottom row)
 *
 * u goes from left to right (0 to 1)
 * v goes from top to bottom (0 to 1)
 */
static float Bilerp(float tl, float tr, float bl, float br, float u, float v)
{
    /* Interpolate along top edge */
    float top = tl + (tr - tl) * u;
    /* Interpolate along bottom edge */
    float bottom = bl + (br - bl) * u;
    /* Interpolate between top and bottom */
    return top + (bottom - top) * v;
}

/*
 * Heightmap_GetHeight - Get interpolated height at world position
 *
 * Uses bilinear interpolation between 4 neighboring grid cells.
 */
float Heightmap_GetHeight(heightmap_t *hm, float worldX, float worldZ)
{
    /* Transform world coords to grid coords */
    float gx = (worldX - hm->origin[0]) / hm->cellSize;
    float gz = (worldZ - hm->origin[2]) / hm->cellSize;

    /* Get integer grid position */
    int x0 = (int)floorf(gx);
    int z0 = (int)floorf(gz);
    int x1 = x0 + 1;
    int z1 = z0 + 1;

    /* Get fractional part */
    float u = gx - (float)x0;
    float v = gz - (float)z0;

    /* Get heights at 4 corners */
    float h00 = Heightmap_GetHeightAtGrid(hm, x0, z0);  /* Top-left */
    float h10 = Heightmap_GetHeightAtGrid(hm, x1, z0);  /* Top-right */
    float h01 = Heightmap_GetHeightAtGrid(hm, x0, z1);  /* Bottom-left */
    float h11 = Heightmap_GetHeightAtGrid(hm, x1, z1);  /* Bottom-right */

    /* Bilinear interpolation */
    return Bilerp(h00, h10, h01, h11, u, v);
}

/*
 * Heightmap_GetNormal - Calculate surface normal at grid position
 *
 * Uses central differences for gradient calculation.
 * Properly handles edge cases by clamping coordinates.
 */
void Heightmap_GetNormal(heightmap_t *hm, int x, int z, vec3_t normal)
{
    /* Clamp coordinates for edge handling */
    int xL = (x > 0) ? x - 1 : x;
    int xR = (x < hm->width - 1) ? x + 1 : x;
    int zD = (z > 0) ? z - 1 : z;
    int zU = (z < hm->height - 1) ? z + 1 : z;

    /* Get heights at neighboring points */
    float hL = Heightmap_GetHeightAtGrid(hm, xL, z);
    float hR = Heightmap_GetHeightAtGrid(hm, xR, z);
    float hD = Heightmap_GetHeightAtGrid(hm, x, zD);
    float hU = Heightmap_GetHeightAtGrid(hm, x, zU);

    /* Calculate step distances (handle edges where step might be 0) */
    float stepX = (float)(xR - xL);
    float stepZ = (float)(zU - zD);

    /* Avoid division by zero at corners */
    if (stepX < 0.5f) stepX = 1.0f;
    if (stepZ < 0.5f) stepZ = 1.0f;

    /* Central difference for gradient
     * The Y component scales with cellSize to maintain proper proportions */
    normal[0] = (hL - hR) / stepX;
    normal[1] = 2.0f * hm->cellSize;
    normal[2] = (hD - hU) / stepZ;

    VectorNormalize(normal);
}

/*
 * Heightmap_GetNormalInterpolated - Get interpolated normal at world position
 */
void Heightmap_GetNormalInterpolated(heightmap_t *hm, float worldX, float worldZ, vec3_t normal)
{
    /* Transform world coords to grid coords */
    float gx = (worldX - hm->origin[0]) / hm->cellSize;
    float gz = (worldZ - hm->origin[2]) / hm->cellSize;

    /* Get integer grid position */
    int x0 = (int)floorf(gx);
    int z0 = (int)floorf(gz);
    int x1 = x0 + 1;
    int z1 = z0 + 1;

    /* Get fractional part */
    float u = gx - (float)x0;
    float v = gz - (float)z0;

    /* Get normals at 4 corners */
    vec3_t n00, n10, n01, n11;
    Heightmap_GetNormal(hm, x0, z0, n00);
    Heightmap_GetNormal(hm, x1, z0, n10);
    Heightmap_GetNormal(hm, x0, z1, n01);
    Heightmap_GetNormal(hm, x1, z1, n11);

    /* Bilinear interpolation of normal components */
    normal[0] = Bilerp(n00[0], n10[0], n01[0], n11[0], u, v);
    normal[1] = Bilerp(n00[1], n10[1], n01[1], n11[1], u, v);
    normal[2] = Bilerp(n00[2], n10[2], n01[2], n11[2], u, v);

    VectorNormalize(normal);
}

/* ============== LOD OPERATIONS ============== */

/*
 * Terrain_CalculateLOD - Determine LOD level based on distance
 *
 * Uses <= for boundary comparisons so chunks at exact threshold distances
 * are included in the higher-detail LOD level.
 */
int Terrain_CalculateLOD(terrain_chunk_t *chunk, vec3_t cameraPos)
{
    /* Calculate chunk center */
    vec3_t center;
    center[0] = (chunk->mins[0] + chunk->maxs[0]) * 0.5f;
    center[1] = (chunk->mins[1] + chunk->maxs[1]) * 0.5f;
    center[2] = (chunk->mins[2] + chunk->maxs[2]) * 0.5f;

    float dist = VectorDistance(cameraPos, center);

    /* Use <= for proper LOD boundaries */
    if (dist <= LOD_DISTANCE_0) return 0;
    if (dist <= LOD_DISTANCE_1) return 1;
    if (dist <= LOD_DISTANCE_2) return 2;
    if (dist <= LOD_DISTANCE_3) return 3;
    return 4;
}

/*
 * Terrain_GetLODStep - Get vertex step size for LOD level
 */
int Terrain_GetLODStep(int lod)
{
    return 1 << lod;  /* 1, 2, 4, 8, 16 */
}

/*
 * Terrain_GetLODGridSize - Get number of vertices per side for LOD level
 */
int Terrain_GetLODGridSize(int lod)
{
    int step = Terrain_GetLODStep(lod);
    return (TERRAIN_CHUNK_SIZE - 1) / step + 1;
}

/* ============== CHUNK OPERATIONS ============== */

/*
 * Terrain_CreateChunk - Allocate and initialize a terrain chunk
 */
void Terrain_CreateChunk(terrain_chunk_t *chunk, int gridX, int gridZ)
{
    memset(chunk, 0, sizeof(*chunk));
    chunk->gridX = gridX;
    chunk->gridZ = gridZ;
    chunk->lod = 0;
    chunk->dirty = true;

    /* Max vertices for LOD 0 */
    int maxVerts = TERRAIN_CHUNK_SIZE * TERRAIN_CHUNK_SIZE;
    chunk->vertices = (terrain_vertex_t *)calloc(maxVerts, sizeof(terrain_vertex_t));

    /* Max indices for LOD 0 (2 triangles per quad, 3 indices per triangle) */
    int maxIndices = (TERRAIN_CHUNK_SIZE - 1) * (TERRAIN_CHUNK_SIZE - 1) * 6;
    chunk->indices = (int *)calloc(maxIndices, sizeof(int));
}

/*
 * Terrain_DestroyChunk - Free chunk resources
 */
void Terrain_DestroyChunk(terrain_chunk_t *chunk)
{
    if (chunk->vertices) {
        free(chunk->vertices);
        chunk->vertices = NULL;
    }
    if (chunk->indices) {
        free(chunk->indices);
        chunk->indices = NULL;
    }
}

/*
 * Terrain_CalculateChunkBounds - Calculate chunk bounding box
 *
 * Iterates over ALL vertices in the chunk to find accurate Y bounds,
 * not just the corners.
 */
void Terrain_CalculateChunkBounds(terrain_chunk_t *chunk, heightmap_t *hm)
{
    int startX = chunk->gridX * (TERRAIN_CHUNK_SIZE - 1);
    int startZ = chunk->gridZ * (TERRAIN_CHUNK_SIZE - 1);

    /* X and Z bounds are straightforward */
    chunk->mins[0] = hm->origin[0] + startX * hm->cellSize;
    chunk->maxs[0] = hm->origin[0] + (startX + TERRAIN_CHUNK_SIZE - 1) * hm->cellSize;
    chunk->mins[2] = hm->origin[2] + startZ * hm->cellSize;
    chunk->maxs[2] = hm->origin[2] + (startZ + TERRAIN_CHUNK_SIZE - 1) * hm->cellSize;

    /* Iterate over ALL vertices to find accurate Y bounds */
    float firstHeight = Heightmap_GetHeightAtGrid(hm, startX, startZ);
    chunk->mins[1] = firstHeight;
    chunk->maxs[1] = firstHeight;

    for (int z = 0; z < TERRAIN_CHUNK_SIZE; z++) {
        for (int x = 0; x < TERRAIN_CHUNK_SIZE; x++) {
            float h = Heightmap_GetHeightAtGrid(hm, startX + x, startZ + z);
            if (h < chunk->mins[1]) chunk->mins[1] = h;
            if (h > chunk->maxs[1]) chunk->maxs[1] = h;
        }
    }
}

/*
 * Terrain_BuildChunkMesh - Generate vertices and indices for a chunk
 */
void Terrain_BuildChunkMesh(terrain_chunk_t *chunk, heightmap_t *hm, int lod)
{
    int step = Terrain_GetLODStep(lod);
    int gridSize = Terrain_GetLODGridSize(lod);

    int startX = chunk->gridX * (TERRAIN_CHUNK_SIZE - 1);
    int startZ = chunk->gridZ * (TERRAIN_CHUNK_SIZE - 1);

    /* Generate vertices */
    chunk->numVertices = 0;
    for (int z = 0; z < gridSize; z++) {
        for (int x = 0; x < gridSize; x++) {
            int gx = startX + x * step;
            int gz = startZ + z * step;

            terrain_vertex_t *v = &chunk->vertices[chunk->numVertices++];

            /* Position */
            v->position[0] = hm->origin[0] + gx * hm->cellSize;
            v->position[2] = hm->origin[2] + gz * hm->cellSize;
            v->position[1] = Heightmap_GetHeightAtGrid(hm, gx, gz);

            /* Normal */
            Heightmap_GetNormal(hm, gx, gz, v->normal);

            /* Texture coordinates */
            v->u = (float)x / (float)(gridSize - 1);
            v->v = (float)z / (float)(gridSize - 1);
        }
    }

    /* Generate indices (two triangles per quad) */
    chunk->numIndices = 0;
    for (int z = 0; z < gridSize - 1; z++) {
        for (int x = 0; x < gridSize - 1; x++) {
            int i00 = z * gridSize + x;
            int i10 = z * gridSize + (x + 1);
            int i01 = (z + 1) * gridSize + x;
            int i11 = (z + 1) * gridSize + (x + 1);

            /* First triangle */
            chunk->indices[chunk->numIndices++] = i00;
            chunk->indices[chunk->numIndices++] = i01;
            chunk->indices[chunk->numIndices++] = i10;

            /* Second triangle */
            chunk->indices[chunk->numIndices++] = i10;
            chunk->indices[chunk->numIndices++] = i01;
            chunk->indices[chunk->numIndices++] = i11;
        }
    }

    chunk->lod = lod;
    chunk->dirty = false;
}

/* ============== EDGE STITCHING ============== */

/*
 * Terrain_GetEdgeVertexIndex - Get index of vertex on chunk edge
 */
static int Terrain_GetEdgeVertexIndex(int edge, int position, int gridSize)
{
    switch (edge) {
        case EDGE_NORTH: return position;                           /* Top row */
        case EDGE_SOUTH: return (gridSize - 1) * gridSize + position;  /* Bottom row */
        case EDGE_WEST:  return position * gridSize;                /* Left column */
        case EDGE_EAST:  return position * gridSize + (gridSize - 1);  /* Right column */
    }
    return 0;
}

/*
 * Terrain_StitchEdge - Adjust edge vertices to match lower-LOD neighbor
 *
 * For vertices that don't exist in the lower-LOD neighbor, interpolate
 * between the surrounding vertices that DO exist.
 */
void Terrain_StitchEdge(terrain_chunk_t *chunk, int edge, int neighborLOD)
{
    if (neighborLOD <= chunk->lod)
        return;  /* No stitching needed - neighbor is same or higher detail */

    int gridSize = Terrain_GetLODGridSize(chunk->lod);
    int lodDiff = neighborLOD - chunk->lod;
    int stride = 1 << lodDiff;  /* How many of our vertices per neighbor vertex */

    /* For each vertex on our edge that doesn't exist in neighbor,
       interpolate between the neighboring vertices that DO exist */
    for (int i = 0; i < gridSize; i++) {
        if (i % stride == 0)
            continue;  /* This vertex exists in neighbor, skip */

        int idx = Terrain_GetEdgeVertexIndex(edge, i, gridSize);

        /* Find surrounding vertices that exist in neighbor */
        int prevI = (i / stride) * stride;
        int nextI = prevI + stride;
        if (nextI >= gridSize) nextI = gridSize - 1;

        int prevIdx = Terrain_GetEdgeVertexIndex(edge, prevI, gridSize);
        int nextIdx = Terrain_GetEdgeVertexIndex(edge, nextI, gridSize);

        /* Correct interpolation factor: use actual distance between vertices */
        float t = (float)(i - prevI) / (float)(nextI - prevI);

        /* Interpolate position (all components for proper geomorphing) */
        terrain_vertex_t *v = &chunk->vertices[idx];
        terrain_vertex_t *vPrev = &chunk->vertices[prevIdx];
        terrain_vertex_t *vNext = &chunk->vertices[nextIdx];

        /* Interpolate Y position to match lower-LOD neighbor's edge */
        v->position[1] = vPrev->position[1] + (vNext->position[1] - vPrev->position[1]) * t;

        /* Also interpolate normals for smooth shading at LOD boundaries */
        v->normal[0] = vPrev->normal[0] + (vNext->normal[0] - vPrev->normal[0]) * t;
        v->normal[1] = vPrev->normal[1] + (vNext->normal[1] - vPrev->normal[1]) * t;
        v->normal[2] = vPrev->normal[2] + (vNext->normal[2] - vPrev->normal[2]) * t;
        VectorNormalize(v->normal);
    }
}

/*
 * Terrain_UpdateStitching - Update all edges based on neighbor LODs
 */
void Terrain_UpdateStitching(terrain_chunk_t *chunk)
{
    for (int edge = 0; edge < 4; edge++) {
        if (chunk->neighbors[edge]) {
            Terrain_StitchEdge(chunk, edge, chunk->neighbors[edge]->lod);
        }
    }
}

/* ============== TERRAIN MANAGEMENT ============== */

/*
 * Terrain_Create - Create terrain from heightmap dimensions
 */
void Terrain_Create(terrain_t *terrain, int hmWidth, int hmHeight, float cellSize, float heightScale)
{
    memset(terrain, 0, sizeof(*terrain));

    /* Initialize heightmap */
    Heightmap_Create(&terrain->heightmap, hmWidth, hmHeight, cellSize, heightScale);

    /* Calculate number of chunks needed */
    terrain->numChunksX = (hmWidth - 1) / (TERRAIN_CHUNK_SIZE - 1);
    terrain->numChunksZ = (hmHeight - 1) / (TERRAIN_CHUNK_SIZE - 1);
    if (terrain->numChunksX < 1) terrain->numChunksX = 1;
    if (terrain->numChunksZ < 1) terrain->numChunksZ = 1;

    terrain->totalChunks = terrain->numChunksX * terrain->numChunksZ;

    /* Allocate chunk array */
    terrain->chunks = (terrain_chunk_t **)calloc(terrain->totalChunks, sizeof(terrain_chunk_t *));

    /* Create individual chunks */
    for (int z = 0; z < terrain->numChunksZ; z++) {
        for (int x = 0; x < terrain->numChunksX; x++) {
            int idx = z * terrain->numChunksX + x;
            terrain->chunks[idx] = (terrain_chunk_t *)calloc(1, sizeof(terrain_chunk_t));
            Terrain_CreateChunk(terrain->chunks[idx], x, z);
        }
    }

    /* Set up neighbor pointers */
    for (int z = 0; z < terrain->numChunksZ; z++) {
        for (int x = 0; x < terrain->numChunksX; x++) {
            int idx = z * terrain->numChunksX + x;
            terrain_chunk_t *chunk = terrain->chunks[idx];

            /* North neighbor (z - 1) */
            if (z > 0)
                chunk->neighbors[EDGE_NORTH] = terrain->chunks[(z - 1) * terrain->numChunksX + x];

            /* East neighbor (x + 1) */
            if (x < terrain->numChunksX - 1)
                chunk->neighbors[EDGE_EAST] = terrain->chunks[z * terrain->numChunksX + (x + 1)];

            /* South neighbor (z + 1) */
            if (z < terrain->numChunksZ - 1)
                chunk->neighbors[EDGE_SOUTH] = terrain->chunks[(z + 1) * terrain->numChunksX + x];

            /* West neighbor (x - 1) */
            if (x > 0)
                chunk->neighbors[EDGE_WEST] = terrain->chunks[z * terrain->numChunksX + (x - 1)];
        }
    }
}

/*
 * Terrain_Destroy - Free all terrain resources
 */
void Terrain_Destroy(terrain_t *terrain)
{
    if (terrain->chunks) {
        for (int i = 0; i < terrain->totalChunks; i++) {
            if (terrain->chunks[i]) {
                Terrain_DestroyChunk(terrain->chunks[i]);
                free(terrain->chunks[i]);
            }
        }
        free(terrain->chunks);
        terrain->chunks = NULL;
    }

    Heightmap_Destroy(&terrain->heightmap);

    if (terrain->splatmap) {
        free(terrain->splatmap);
        terrain->splatmap = NULL;
    }
}

/*
 * Terrain_GetChunk - Get chunk at grid position
 */
terrain_chunk_t *Terrain_GetChunk(terrain_t *terrain, int gridX, int gridZ)
{
    if (gridX < 0 || gridX >= terrain->numChunksX ||
        gridZ < 0 || gridZ >= terrain->numChunksZ)
        return NULL;

    return terrain->chunks[gridZ * terrain->numChunksX + gridX];
}

/*
 * Terrain_UpdateLODs - Update all chunk LOD levels based on camera position
 */
void Terrain_UpdateLODs(terrain_t *terrain, vec3_t cameraPos)
{
    for (int i = 0; i < terrain->totalChunks; i++) {
        terrain_chunk_t *chunk = terrain->chunks[i];

        /* Ensure bounds are calculated */
        if (chunk->dirty) {
            Terrain_CalculateChunkBounds(chunk, &terrain->heightmap);
        }

        int newLOD = Terrain_CalculateLOD(chunk, cameraPos);

        if (newLOD != chunk->lod || chunk->dirty) {
            Terrain_BuildChunkMesh(chunk, &terrain->heightmap, newLOD);
        }
    }

    /* Update stitching after all LODs are determined */
    for (int i = 0; i < terrain->totalChunks; i++) {
        Terrain_UpdateStitching(terrain->chunks[i]);
    }
}

/* ============== COLLISION ============== */

/*
 * Terrain_GetHeightAt - Get terrain height at world position
 */
float Terrain_GetHeightAt(terrain_t *terrain, float x, float z)
{
    return Heightmap_GetHeight(&terrain->heightmap, x, z);
}

/*
 * Terrain_RayIntersect - Test ray intersection with terrain
 *
 * Simple stepping algorithm - walks along ray testing height
 */
qboolean Terrain_RayIntersect(terrain_t *terrain, vec3_t start, vec3_t direction,
                              float maxDistance, terrain_collision_t *result)
{
    float stepSize = terrain->heightmap.cellSize * 0.5f;
    int maxSteps = (int)(maxDistance / stepSize);

    vec3_t pos;
    VectorCopy(start, pos);

    vec3_t step;
    VectorNormalize(direction);
    VectorScale(direction, stepSize, step);

    for (int i = 0; i < maxSteps; i++) {
        float terrainHeight = Terrain_GetHeightAt(terrain, pos[0], pos[2]);

        if (pos[1] < terrainHeight) {
            /* Hit terrain */
            result->hit = true;
            VectorCopy(pos, result->point);
            result->point[1] = terrainHeight;
            Heightmap_GetNormalInterpolated(&terrain->heightmap, pos[0], pos[2], result->normal);
            result->distance = i * stepSize;
            return true;
        }

        VectorAdd(pos, step, pos);
    }

    result->hit = false;
    return false;
}
