/*
 * Terrain System Test Harness
 *
 * Tests all components of the procedural terrain system:
 * 1. Heightmap creation and sampling
 * 2. Bilinear interpolation for height queries
 * 3. Normal calculation at grid points and interpolated
 * 4. LOD level selection based on distance
 * 5. Chunk mesh generation at different LOD levels
 * 6. Edge stitching between chunks with different LODs
 * 7. Bounding box calculation for chunks
 * 8. Terrain collision (ray intersection)
 *
 * Build:
 *   make test        - Test incomplete (game) version
 *   make test_opt    - Test complete (solution) version
 *   make compare     - Compare both versions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Include the implementation under test */
#ifdef TEST_SOLUTION
#include "../solution/terrain.c"
#else
#include "../game/terrain.c"
#endif

/* ============== TEST UTILITIES ============== */

static int tests_passed = 0;
static int tests_failed = 0;

#define EPSILON 0.01f

static int float_eq(float a, float b)
{
    return fabsf(a - b) < EPSILON;
}

static int float_near(float a, float b, float tolerance)
{
    return fabsf(a - b) < tolerance;
}

static int vec3_eq(vec3_t a, vec3_t b)
{
    return float_eq(a[0], b[0]) && float_eq(a[1], b[1]) && float_eq(a[2], b[2]);
}

static void print_vec3(const char *name, vec3_t v)
{
    printf("  %s: (%.4f, %.4f, %.4f)\n", name, v[0], v[1], v[2]);
}

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

/* ============== HEIGHTMAP TESTS ============== */

TEST(heightmap_create)
{
    heightmap_t hm;
    Heightmap_Create(&hm, 64, 64, 1.0f, 1.0f);

    ASSERT(hm.width == 64, "Width should be 64");
    ASSERT(hm.height == 64, "Height should be 64");
    ASSERT(hm.heights != NULL, "Heights array should be allocated");
    ASSERT(float_eq(hm.cellSize, 1.0f), "Cell size should be 1.0");

    Heightmap_Destroy(&hm);
    PASS();
}

TEST(heightmap_set_get)
{
    heightmap_t hm;
    Heightmap_Create(&hm, 8, 8, 1.0f, 1.0f);

    Heightmap_SetHeight(&hm, 3, 4, 5.0f);
    float h = Heightmap_GetHeightAtGrid(&hm, 3, 4);

    ASSERT(float_eq(h, 5.0f), "Retrieved height should match set height");

    Heightmap_Destroy(&hm);
    PASS();
}

TEST(heightmap_height_scale)
{
    heightmap_t hm;
    Heightmap_Create(&hm, 8, 8, 1.0f, 2.0f);  /* Height scale = 2.0 */

    Heightmap_SetHeight(&hm, 2, 2, 10.0f);
    float h = Heightmap_GetHeightAtGrid(&hm, 2, 2);

    ASSERT(float_eq(h, 20.0f), "Height should be scaled by heightScale factor");

    Heightmap_Destroy(&hm);
    PASS();
}

TEST(heightmap_bounds_clamping)
{
    heightmap_t hm;
    Heightmap_Create(&hm, 8, 8, 1.0f, 1.0f);

    /* Set corner heights */
    Heightmap_SetHeight(&hm, 0, 0, 1.0f);
    Heightmap_SetHeight(&hm, 7, 0, 2.0f);
    Heightmap_SetHeight(&hm, 0, 7, 3.0f);
    Heightmap_SetHeight(&hm, 7, 7, 4.0f);

    /* Test clamping at out-of-bounds positions */
    float h1 = Heightmap_GetHeightAtGrid(&hm, -5, 0);  /* Should clamp to 0,0 */
    float h2 = Heightmap_GetHeightAtGrid(&hm, 100, 0); /* Should clamp to 7,0 */

    ASSERT(float_eq(h1, 1.0f), "Negative X should clamp to 0");
    ASSERT(float_eq(h2, 2.0f), "X beyond width should clamp to max");

    Heightmap_Destroy(&hm);
    PASS();
}

/* ============== BILINEAR INTERPOLATION TESTS ============== */

TEST(bilerp_corners)
{
    heightmap_t hm;
    Heightmap_Create(&hm, 4, 4, 1.0f, 1.0f);

    /* Set up a simple gradient: each cell has value = x + z*10 */
    for (int z = 0; z < 4; z++) {
        for (int x = 0; x < 4; x++) {
            Heightmap_SetHeight(&hm, x, z, (float)(x + z * 10));
        }
    }

    /* Query exactly at grid points should return exact values */
    float h00 = Heightmap_GetHeight(&hm, 0.0f, 0.0f);
    float h10 = Heightmap_GetHeight(&hm, 1.0f, 0.0f);
    float h01 = Heightmap_GetHeight(&hm, 0.0f, 1.0f);
    float h11 = Heightmap_GetHeight(&hm, 1.0f, 1.0f);

    ASSERT(float_eq(h00, 0.0f), "h(0,0) should be 0");
    ASSERT(float_eq(h10, 1.0f), "h(1,0) should be 1");
    ASSERT(float_eq(h01, 10.0f), "h(0,1) should be 10");
    ASSERT(float_eq(h11, 11.0f), "h(1,1) should be 11");

    Heightmap_Destroy(&hm);
    PASS();
}

TEST(bilerp_center)
{
    heightmap_t hm;
    Heightmap_Create(&hm, 4, 4, 1.0f, 1.0f);

    /* Set up specific corner values */
    Heightmap_SetHeight(&hm, 0, 0, 0.0f);
    Heightmap_SetHeight(&hm, 1, 0, 10.0f);
    Heightmap_SetHeight(&hm, 0, 1, 20.0f);
    Heightmap_SetHeight(&hm, 1, 1, 30.0f);

    /* Query at center (0.5, 0.5) should be average = 15 */
    float h = Heightmap_GetHeight(&hm, 0.5f, 0.5f);

    ASSERT(float_eq(h, 15.0f), "Center should be average of 4 corners (15.0)");

    Heightmap_Destroy(&hm);
    PASS();
}

TEST(bilerp_edge_x)
{
    heightmap_t hm;
    Heightmap_Create(&hm, 4, 4, 1.0f, 1.0f);

    /* Set values at (0,0)=0, (1,0)=10 */
    Heightmap_SetHeight(&hm, 0, 0, 0.0f);
    Heightmap_SetHeight(&hm, 1, 0, 10.0f);
    Heightmap_SetHeight(&hm, 0, 1, 0.0f);
    Heightmap_SetHeight(&hm, 1, 1, 10.0f);

    /* Query at (0.5, 0) should be 5 (halfway between 0 and 10) */
    float h = Heightmap_GetHeight(&hm, 0.5f, 0.0f);

    ASSERT(float_eq(h, 5.0f), "Midpoint along X edge should interpolate correctly");

    Heightmap_Destroy(&hm);
    PASS();
}

TEST(bilerp_edge_z)
{
    heightmap_t hm;
    Heightmap_Create(&hm, 4, 4, 1.0f, 1.0f);

    /* Set values at (0,0)=0, (0,1)=20 */
    Heightmap_SetHeight(&hm, 0, 0, 0.0f);
    Heightmap_SetHeight(&hm, 1, 0, 0.0f);
    Heightmap_SetHeight(&hm, 0, 1, 20.0f);
    Heightmap_SetHeight(&hm, 1, 1, 20.0f);

    /* Query at (0, 0.5) should be 10 (halfway between 0 and 20) */
    float h = Heightmap_GetHeight(&hm, 0.0f, 0.5f);

    ASSERT(float_eq(h, 10.0f), "Midpoint along Z edge should interpolate correctly");

    Heightmap_Destroy(&hm);
    PASS();
}

TEST(bilerp_asymmetric)
{
    heightmap_t hm;
    Heightmap_Create(&hm, 4, 4, 1.0f, 1.0f);

    /* Set asymmetric values to test correct corner ordering */
    Heightmap_SetHeight(&hm, 0, 0, 0.0f);   /* Top-left */
    Heightmap_SetHeight(&hm, 1, 0, 100.0f); /* Top-right */
    Heightmap_SetHeight(&hm, 0, 1, 0.0f);   /* Bottom-left */
    Heightmap_SetHeight(&hm, 1, 1, 0.0f);   /* Bottom-right */

    /* Query at (0.75, 0.25) - closer to top-right */
    /* Expected: top = 0 + (100-0)*0.75 = 75 */
    /*           bottom = 0 + (0-0)*0.75 = 0 */
    /*           result = 75 + (0-75)*0.25 = 56.25 */
    float h = Heightmap_GetHeight(&hm, 0.75f, 0.25f);

    ASSERT(float_near(h, 56.25f, 0.5f), "Asymmetric interpolation should be correct");

    Heightmap_Destroy(&hm);
    PASS();
}

/* ============== NORMAL CALCULATION TESTS ============== */

TEST(normal_flat_surface)
{
    heightmap_t hm;
    Heightmap_Create(&hm, 8, 8, 1.0f, 1.0f);

    /* All heights set to 0 (flat surface) */
    for (int z = 0; z < 8; z++) {
        for (int x = 0; x < 8; x++) {
            Heightmap_SetHeight(&hm, x, z, 0.0f);
        }
    }

    vec3_t normal;
    Heightmap_GetNormal(&hm, 4, 4, normal);

    /* Flat surface should have normal pointing up (0, 1, 0) */
    ASSERT(float_near(normal[0], 0.0f, 0.1f), "Normal X should be ~0 for flat surface");
    ASSERT(normal[1] > 0.9f, "Normal Y should be ~1 for flat surface");
    ASSERT(float_near(normal[2], 0.0f, 0.1f), "Normal Z should be ~0 for flat surface");

    Heightmap_Destroy(&hm);
    PASS();
}

TEST(normal_slope_x)
{
    heightmap_t hm;
    Heightmap_Create(&hm, 8, 8, 1.0f, 1.0f);

    /* Slope along X: height = x */
    for (int z = 0; z < 8; z++) {
        for (int x = 0; x < 8; x++) {
            Heightmap_SetHeight(&hm, x, z, (float)x);
        }
    }

    vec3_t normal;
    Heightmap_GetNormal(&hm, 4, 4, normal);

    /* Slope going up in +X should have normal pointing somewhat in -X */
    ASSERT(normal[0] < 0.0f, "Normal should have negative X component for +X slope");
    ASSERT(normal[1] > 0.0f, "Normal should have positive Y component");

    Heightmap_Destroy(&hm);
    PASS();
}

TEST(normal_edge_handling)
{
    heightmap_t hm;
    Heightmap_Create(&hm, 4, 4, 1.0f, 1.0f);

    /* Set some heights */
    for (int z = 0; z < 4; z++) {
        for (int x = 0; x < 4; x++) {
            Heightmap_SetHeight(&hm, x, z, (float)(x + z));
        }
    }

    vec3_t normalCorner, normalCenter;

    /* Get normal at corner (0,0) - tests edge case handling */
    Heightmap_GetNormal(&hm, 0, 0, normalCorner);

    /* Get normal at center (2,2) */
    Heightmap_GetNormal(&hm, 2, 2, normalCenter);

    /* Both should be valid unit vectors */
    float lenCorner = VectorLength(normalCorner);
    float lenCenter = VectorLength(normalCenter);

    ASSERT(float_near(lenCorner, 1.0f, 0.01f), "Corner normal should be unit length");
    ASSERT(float_near(lenCenter, 1.0f, 0.01f), "Center normal should be unit length");

    Heightmap_Destroy(&hm);
    PASS();
}

/* ============== LOD TESTS ============== */

TEST(lod_step_calculation)
{
    ASSERT(Terrain_GetLODStep(0) == 1, "LOD 0 step should be 1");
    ASSERT(Terrain_GetLODStep(1) == 2, "LOD 1 step should be 2");
    ASSERT(Terrain_GetLODStep(2) == 4, "LOD 2 step should be 4");
    ASSERT(Terrain_GetLODStep(3) == 8, "LOD 3 step should be 8");
    ASSERT(Terrain_GetLODStep(4) == 16, "LOD 4 step should be 16");
    PASS();
}

TEST(lod_grid_size)
{
    /* TERRAIN_CHUNK_SIZE is 33 (32 quads + 1) */
    int size0 = Terrain_GetLODGridSize(0);  /* 33 */
    int size1 = Terrain_GetLODGridSize(1);  /* 17 */
    int size2 = Terrain_GetLODGridSize(2);  /* 9 */
    int size3 = Terrain_GetLODGridSize(3);  /* 5 */
    int size4 = Terrain_GetLODGridSize(4);  /* 3 */

    ASSERT(size0 == 33, "LOD 0 grid size should be 33");
    ASSERT(size1 == 17, "LOD 1 grid size should be 17");
    ASSERT(size2 == 9, "LOD 2 grid size should be 9");
    ASSERT(size3 == 5, "LOD 3 grid size should be 5");
    ASSERT(size4 == 3, "LOD 4 grid size should be 3");
    PASS();
}

TEST(lod_distance_selection)
{
    terrain_chunk_t chunk;
    memset(&chunk, 0, sizeof(chunk));

    /* Set chunk bounds to center at origin */
    VectorSet(chunk.mins, -16.0f, 0.0f, -16.0f);
    VectorSet(chunk.maxs, 16.0f, 10.0f, 16.0f);

    vec3_t cameraPos;

    /* Camera very close -> LOD 0 */
    VectorSet(cameraPos, 0.0f, 5.0f, 0.0f);
    int lod0 = Terrain_CalculateLOD(&chunk, cameraPos);
    ASSERT(lod0 == 0, "Very close camera should give LOD 0");

    /* Camera at LOD_DISTANCE_0 threshold -> should still be LOD 0 */
    VectorSet(cameraPos, 256.0f, 5.0f, 0.0f);
    int lod_at_thresh = Terrain_CalculateLOD(&chunk, cameraPos);
    ASSERT(lod_at_thresh == 0, "Camera at LOD_DISTANCE_0 should give LOD 0");

    /* Camera beyond LOD_DISTANCE_0 but within LOD_DISTANCE_1 */
    VectorSet(cameraPos, 400.0f, 5.0f, 0.0f);
    int lod1 = Terrain_CalculateLOD(&chunk, cameraPos);
    ASSERT(lod1 == 1, "Camera between LOD thresholds should give LOD 1");

    /* Camera very far away */
    VectorSet(cameraPos, 5000.0f, 5.0f, 0.0f);
    int lod4 = Terrain_CalculateLOD(&chunk, cameraPos);
    ASSERT(lod4 == 4, "Very distant camera should give LOD 4");

    PASS();
}

/* ============== CHUNK TESTS ============== */

TEST(chunk_creation)
{
    terrain_chunk_t chunk;
    Terrain_CreateChunk(&chunk, 2, 3);

    ASSERT(chunk.gridX == 2, "Chunk gridX should be 2");
    ASSERT(chunk.gridZ == 3, "Chunk gridZ should be 3");
    ASSERT(chunk.vertices != NULL, "Vertices should be allocated");
    ASSERT(chunk.indices != NULL, "Indices should be allocated");
    ASSERT(chunk.dirty == true, "New chunk should be marked dirty");

    Terrain_DestroyChunk(&chunk);
    PASS();
}

TEST(chunk_bounds_complete)
{
    /* Test that Y bounds check all vertices, not just corners */
    terrain_t terrain;
    Terrain_Create(&terrain, 65, 65, 1.0f, 1.0f);

    /* Set a high point in the middle of the chunk */
    int midX = (TERRAIN_CHUNK_SIZE - 1) / 2;
    int midZ = (TERRAIN_CHUNK_SIZE - 1) / 2;
    Heightmap_SetHeight(&terrain.heightmap, midX, midZ, 100.0f);

    /* Calculate bounds for chunk 0 */
    terrain_chunk_t *chunk = terrain.chunks[0];
    Terrain_CalculateChunkBounds(chunk, &terrain.heightmap);

    /* Max Y should include the peak at 100 */
    ASSERT(chunk->maxs[1] >= 100.0f, "Chunk max Y should include interior peak");

    Terrain_Destroy(&terrain);
    PASS();
}

TEST(chunk_mesh_lod0)
{
    terrain_t terrain;
    Terrain_Create(&terrain, 65, 65, 1.0f, 1.0f);

    terrain_chunk_t *chunk = terrain.chunks[0];
    Terrain_CalculateChunkBounds(chunk, &terrain.heightmap);
    Terrain_BuildChunkMesh(chunk, &terrain.heightmap, 0);

    /* LOD 0: 33x33 vertices = 1089 vertices */
    ASSERT(chunk->numVertices == 33 * 33, "LOD 0 should have 33x33 vertices");

    /* LOD 0: 32x32 quads, 2 triangles each, 3 indices per triangle = 6144 indices */
    ASSERT(chunk->numIndices == 32 * 32 * 6, "LOD 0 should have correct index count");

    ASSERT(chunk->lod == 0, "Chunk LOD should be set to 0");
    ASSERT(chunk->dirty == false, "Chunk should not be dirty after build");

    Terrain_Destroy(&terrain);
    PASS();
}

TEST(chunk_mesh_lod2)
{
    terrain_t terrain;
    Terrain_Create(&terrain, 65, 65, 1.0f, 1.0f);

    terrain_chunk_t *chunk = terrain.chunks[0];
    Terrain_CalculateChunkBounds(chunk, &terrain.heightmap);
    Terrain_BuildChunkMesh(chunk, &terrain.heightmap, 2);

    /* LOD 2: 9x9 vertices = 81 vertices */
    ASSERT(chunk->numVertices == 9 * 9, "LOD 2 should have 9x9 vertices");

    /* LOD 2: 8x8 quads, 2 triangles each, 3 indices per triangle = 384 indices */
    ASSERT(chunk->numIndices == 8 * 8 * 6, "LOD 2 should have correct index count");

    Terrain_Destroy(&terrain);
    PASS();
}

/* ============== STITCHING TESTS ============== */

TEST(stitch_no_change_same_lod)
{
    terrain_t terrain;
    Terrain_Create(&terrain, 65, 65, 1.0f, 1.0f);

    /* Set some height data */
    for (int z = 0; z < 65; z++) {
        for (int x = 0; x < 65; x++) {
            Heightmap_SetHeight(&terrain.heightmap, x, z, (float)(x + z));
        }
    }

    terrain_chunk_t *chunk = terrain.chunks[0];
    Terrain_CalculateChunkBounds(chunk, &terrain.heightmap);
    Terrain_BuildChunkMesh(chunk, &terrain.heightmap, 0);

    /* Store original edge vertex positions */
    float originalY[33];
    int gridSize = Terrain_GetLODGridSize(0);
    for (int i = 0; i < gridSize; i++) {
        originalY[i] = chunk->vertices[i].position[1];  /* North edge */
    }

    /* Stitch with same LOD neighbor - should not change anything */
    Terrain_StitchEdge(chunk, EDGE_NORTH, 0);

    /* Verify no changes */
    int changed = 0;
    for (int i = 0; i < gridSize; i++) {
        if (!float_eq(chunk->vertices[i].position[1], originalY[i])) {
            changed = 1;
            break;
        }
    }

    ASSERT(!changed, "Stitching with same LOD should not modify vertices");

    Terrain_Destroy(&terrain);
    PASS();
}

TEST(stitch_interpolation_correct)
{
    terrain_t terrain;
    Terrain_Create(&terrain, 97, 97, 1.0f, 1.0f);  /* 3x3 chunks */

    /* Set heights to make interpolation testable */
    /* Edge between chunk (0,0) and (1,0) is at x=32 */
    for (int z = 0; z < 97; z++) {
        for (int x = 0; x < 97; x++) {
            /* Linear slope along X */
            Heightmap_SetHeight(&terrain.heightmap, x, z, (float)x);
        }
    }

    terrain_chunk_t *chunk0 = terrain.chunks[0];  /* LOD 0 */
    terrain_chunk_t *chunk1 = terrain.chunks[1];  /* Will be LOD 1 */

    /* Build chunk 0 at LOD 0 (high detail) */
    Terrain_CalculateChunkBounds(chunk0, &terrain.heightmap);
    Terrain_BuildChunkMesh(chunk0, &terrain.heightmap, 0);

    /* Build chunk 1 at LOD 1 (lower detail) */
    chunk1->lod = 1;

    /* At LOD 0, east edge vertex 16 (middle of edge) should have position */
    /* Before stitching: from original heightmap */
    int gridSize = Terrain_GetLODGridSize(0);
    int midIdx = 16 * gridSize + (gridSize - 1);  /* Middle vertex on east edge */
    float originalY = chunk0->vertices[midIdx].position[1];

    /* After stitching with LOD 1 neighbor, vertex 16 doesn't exist in neighbor */
    /* It should be interpolated between vertices 0 and 32 (in LOD 0 numbering) */
    /* which correspond to vertices 0 and 16 in LOD 1 */
    Terrain_StitchEdge(chunk0, EDGE_EAST, 1);

    /* Get the edge vertex positions at endpoints */
    int idx0 = 0 * gridSize + (gridSize - 1);   /* First vertex on east edge */
    int idx32 = 32 * gridSize + (gridSize - 1); /* Last vertex on east edge */

    float y0 = chunk0->vertices[idx0].position[1];
    float y32 = chunk0->vertices[idx32].position[1];
    float expectedY16 = (y0 + y32) * 0.5f;

    float actualY16 = chunk0->vertices[midIdx].position[1];

    ASSERT(float_near(actualY16, expectedY16, 0.5f),
           "Stitched middle vertex should be interpolated between endpoints");

    Terrain_Destroy(&terrain);
    PASS();
}

/* ============== TERRAIN MANAGEMENT TESTS ============== */

TEST(terrain_create)
{
    terrain_t terrain;
    Terrain_Create(&terrain, 65, 65, 1.0f, 1.0f);

    ASSERT(terrain.heightmap.width == 65, "Heightmap width should be 65");
    ASSERT(terrain.heightmap.height == 65, "Heightmap height should be 65");
    ASSERT(terrain.numChunksX == 2, "Should have 2 chunks in X");
    ASSERT(terrain.numChunksZ == 2, "Should have 2 chunks in Z");
    ASSERT(terrain.totalChunks == 4, "Should have 4 total chunks");
    ASSERT(terrain.chunks != NULL, "Chunks array should be allocated");

    Terrain_Destroy(&terrain);
    PASS();
}

TEST(terrain_neighbors)
{
    terrain_t terrain;
    Terrain_Create(&terrain, 97, 97, 1.0f, 1.0f);  /* 3x3 chunks */

    /* Center chunk should have 4 neighbors */
    terrain_chunk_t *center = terrain.chunks[4];  /* (1,1) in 3x3 grid */

    ASSERT(center->neighbors[EDGE_NORTH] != NULL, "Center chunk should have north neighbor");
    ASSERT(center->neighbors[EDGE_SOUTH] != NULL, "Center chunk should have south neighbor");
    ASSERT(center->neighbors[EDGE_EAST] != NULL, "Center chunk should have east neighbor");
    ASSERT(center->neighbors[EDGE_WEST] != NULL, "Center chunk should have west neighbor");

    /* Corner chunk should have only 2 neighbors */
    terrain_chunk_t *corner = terrain.chunks[0];  /* (0,0) */

    ASSERT(corner->neighbors[EDGE_NORTH] == NULL, "Corner chunk should not have north neighbor");
    ASSERT(corner->neighbors[EDGE_WEST] == NULL, "Corner chunk should not have west neighbor");
    ASSERT(corner->neighbors[EDGE_SOUTH] != NULL, "Corner chunk should have south neighbor");
    ASSERT(corner->neighbors[EDGE_EAST] != NULL, "Corner chunk should have east neighbor");

    Terrain_Destroy(&terrain);
    PASS();
}

TEST(terrain_get_chunk)
{
    terrain_t terrain;
    Terrain_Create(&terrain, 97, 97, 1.0f, 1.0f);

    terrain_chunk_t *chunk = Terrain_GetChunk(&terrain, 1, 2);
    ASSERT(chunk != NULL, "Should find chunk at valid position");
    ASSERT(chunk->gridX == 1, "Retrieved chunk gridX should match");
    ASSERT(chunk->gridZ == 2, "Retrieved chunk gridZ should match");

    terrain_chunk_t *invalid = Terrain_GetChunk(&terrain, 10, 10);
    ASSERT(invalid == NULL, "Should return NULL for invalid position");

    Terrain_Destroy(&terrain);
    PASS();
}

/* ============== COLLISION TESTS ============== */

TEST(collision_height_at)
{
    terrain_t terrain;
    Terrain_Create(&terrain, 33, 33, 1.0f, 1.0f);

    /* Set a simple height pattern */
    for (int z = 0; z < 33; z++) {
        for (int x = 0; x < 33; x++) {
            Heightmap_SetHeight(&terrain.heightmap, x, z, (float)(x + z));
        }
    }

    /* Test height query */
    float h = Terrain_GetHeightAt(&terrain, 5.0f, 10.0f);
    ASSERT(float_near(h, 15.0f, 0.1f), "Height at (5,10) should be ~15");

    Terrain_Destroy(&terrain);
    PASS();
}

TEST(collision_ray_hit)
{
    terrain_t terrain;
    Terrain_Create(&terrain, 33, 33, 1.0f, 1.0f);

    /* Flat terrain at height 0 */
    for (int z = 0; z < 33; z++) {
        for (int x = 0; x < 33; x++) {
            Heightmap_SetHeight(&terrain.heightmap, x, z, 0.0f);
        }
    }

    /* Ray from above pointing down */
    vec3_t start = {16.0f, 10.0f, 16.0f};
    vec3_t direction = {0.0f, -1.0f, 0.0f};

    terrain_collision_t result;
    qboolean hit = Terrain_RayIntersect(&terrain, start, direction, 100.0f, &result);

    ASSERT(hit, "Downward ray should hit terrain");
    ASSERT(result.hit, "Result hit flag should be set");
    ASSERT(float_near(result.point[1], 0.0f, 0.5f), "Hit point Y should be near 0");

    Terrain_Destroy(&terrain);
    PASS();
}

TEST(collision_ray_miss)
{
    terrain_t terrain;
    Terrain_Create(&terrain, 33, 33, 1.0f, 1.0f);

    /* Flat terrain at height 0 */

    /* Ray pointing up from above terrain */
    vec3_t start = {16.0f, 10.0f, 16.0f};
    vec3_t direction = {0.0f, 1.0f, 0.0f};

    terrain_collision_t result;
    qboolean hit = Terrain_RayIntersect(&terrain, start, direction, 100.0f, &result);

    ASSERT(!hit, "Upward ray should miss terrain");

    Terrain_Destroy(&terrain);
    PASS();
}

/* ============== INTEGRATION TESTS ============== */

TEST(integration_lod_update)
{
    terrain_t terrain;
    Terrain_Create(&terrain, 97, 97, 1.0f, 1.0f);

    /* Set some height data */
    for (int z = 0; z < 97; z++) {
        for (int x = 0; x < 97; x++) {
            Heightmap_SetHeight(&terrain.heightmap, x, z, sinf((float)x * 0.1f) * 5.0f);
        }
    }

    /* Update LODs with camera at origin */
    vec3_t cameraPos = {0.0f, 5.0f, 0.0f};
    Terrain_UpdateLODs(&terrain, cameraPos);

    /* Near chunk should have lower LOD number (more detail) */
    terrain_chunk_t *nearChunk = terrain.chunks[0];

    /* Far chunk should have higher LOD number */
    vec3_t farCameraPos = {3000.0f, 5.0f, 3000.0f};
    Terrain_UpdateLODs(&terrain, farCameraPos);
    terrain_chunk_t *farChunk = terrain.chunks[0];  /* Now far from camera */

    ASSERT(farChunk->lod > 0, "Far chunk should have higher LOD after update");

    Terrain_Destroy(&terrain);
    PASS();
}

TEST(integration_full_pipeline)
{
    /* Test the complete terrain creation and querying pipeline */
    terrain_t terrain;
    Terrain_Create(&terrain, 65, 65, 2.0f, 0.5f);  /* 2 unit cells, 0.5 height scale */

    /* Create a hill in the middle */
    for (int z = 0; z < 65; z++) {
        for (int x = 0; x < 65; x++) {
            float dx = (float)(x - 32);
            float dz = (float)(z - 32);
            float dist = sqrtf(dx * dx + dz * dz);
            float height = 100.0f * expf(-dist * dist / 200.0f);  /* Gaussian hill */
            Heightmap_SetHeight(&terrain.heightmap, x, z, height);
        }
    }

    /* Update LODs */
    vec3_t cameraPos = {64.0f, 50.0f, 64.0f};  /* Center of terrain */
    Terrain_UpdateLODs(&terrain, cameraPos);

    /* Test height at center (should be peak of hill) */
    float peakHeight = Terrain_GetHeightAt(&terrain, 64.0f, 64.0f);
    ASSERT(peakHeight > 40.0f, "Peak should be high (scaled by 0.5)");

    /* Test ray collision */
    vec3_t start = {64.0f, 100.0f, 64.0f};
    vec3_t dir = {0.0f, -1.0f, 0.0f};
    terrain_collision_t result;
    qboolean hit = Terrain_RayIntersect(&terrain, start, dir, 200.0f, &result);

    ASSERT(hit, "Ray should hit the hill");
    ASSERT(result.normal[1] > 0.5f, "Normal should point mostly up at peak");

    Terrain_Destroy(&terrain);
    PASS();
}

/* ============== MAIN ============== */

int main(void)
{
    printf("===========================================\n");
#ifdef TEST_SOLUTION
    printf("Terrain System Tests (SOLUTION VERSION)\n");
#else
    printf("Terrain System Tests (GAME VERSION)\n");
#endif
    printf("===========================================\n\n");

    /* Heightmap tests */
    printf("--- Heightmap Tests ---\n");
    run_test_heightmap_create();
    run_test_heightmap_set_get();
    run_test_heightmap_height_scale();
    run_test_heightmap_bounds_clamping();

    /* Bilinear interpolation tests */
    printf("\n--- Bilinear Interpolation Tests ---\n");
    run_test_bilerp_corners();
    run_test_bilerp_center();
    run_test_bilerp_edge_x();
    run_test_bilerp_edge_z();
    run_test_bilerp_asymmetric();

    /* Normal calculation tests */
    printf("\n--- Normal Calculation Tests ---\n");
    run_test_normal_flat_surface();
    run_test_normal_slope_x();
    run_test_normal_edge_handling();

    /* LOD tests */
    printf("\n--- LOD Tests ---\n");
    run_test_lod_step_calculation();
    run_test_lod_grid_size();
    run_test_lod_distance_selection();

    /* Chunk tests */
    printf("\n--- Chunk Tests ---\n");
    run_test_chunk_creation();
    run_test_chunk_bounds_complete();
    run_test_chunk_mesh_lod0();
    run_test_chunk_mesh_lod2();

    /* Stitching tests */
    printf("\n--- Stitching Tests ---\n");
    run_test_stitch_no_change_same_lod();
    run_test_stitch_interpolation_correct();

    /* Terrain management tests */
    printf("\n--- Terrain Management Tests ---\n");
    run_test_terrain_create();
    run_test_terrain_neighbors();
    run_test_terrain_get_chunk();

    /* Collision tests */
    printf("\n--- Collision Tests ---\n");
    run_test_collision_height_at();
    run_test_collision_ray_hit();
    run_test_collision_ray_miss();

    /* Integration tests */
    printf("\n--- Integration Tests ---\n");
    run_test_integration_lod_update();
    run_test_integration_full_pipeline();

    /* Summary */
    printf("\n===========================================\n");
    printf("RESULTS: %d passed, %d failed\n", tests_passed, tests_failed);
    printf("===========================================\n");

    return (tests_failed > 0) ? 1 : 0;
}
