/*
 * quakedef.h - Common definitions for terrain system
 *
 * Defines types, macros, and structures used by the terrain implementation.
 */

#ifndef QUAKEDEF_H
#define QUAKEDEF_H

#include <math.h>
#include <stdlib.h>
#include <string.h>

/* Basic types */
typedef float vec3_t[3];
typedef int qboolean;

#define true 1
#define false 0

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* Terrain constants */
#define TERRAIN_CHUNK_SIZE 33       /* 32 quads + 1 for seamless edges */
#define MAX_LOD_LEVELS 5
#define MAX_TERRAIN_CHUNKS 256
#define MAX_TEXTURE_LAYERS 4

/* LOD distance thresholds */
#define LOD_DISTANCE_0 256.0f       /* Full detail */
#define LOD_DISTANCE_1 512.0f
#define LOD_DISTANCE_2 1024.0f
#define LOD_DISTANCE_3 2048.0f

/* Edge indices for stitching */
#define EDGE_NORTH 0
#define EDGE_EAST  1
#define EDGE_SOUTH 2
#define EDGE_WEST  3

/* Vector operations */
#define VectorCopy(a, b) do { (b)[0] = (a)[0]; (b)[1] = (a)[1]; (b)[2] = (a)[2]; } while(0)
#define VectorSet(v, x, y, z) do { (v)[0] = (x); (v)[1] = (y); (v)[2] = (z); } while(0)
#define VectorAdd(a, b, out) do { (out)[0] = (a)[0] + (b)[0]; (out)[1] = (a)[1] + (b)[1]; (out)[2] = (a)[2] + (b)[2]; } while(0)
#define VectorSubtract(a, b, out) do { (out)[0] = (a)[0] - (b)[0]; (out)[1] = (a)[1] - (b)[1]; (out)[2] = (a)[2] - (b)[2]; } while(0)
#define VectorScale(v, s, out) do { (out)[0] = (v)[0] * (s); (out)[1] = (v)[1] * (s); (out)[2] = (v)[2] * (s); } while(0)
#define DotProduct(a, b) ((a)[0] * (b)[0] + (a)[1] * (b)[1] + (a)[2] * (b)[2])

static inline float VectorLength(vec3_t v)
{
    return sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

static inline void VectorNormalize(vec3_t v)
{
    float len = VectorLength(v);
    if (len > 0.0001f) {
        float invLen = 1.0f / len;
        v[0] *= invLen;
        v[1] *= invLen;
        v[2] *= invLen;
    }
}

static inline float VectorDistance(vec3_t a, vec3_t b)
{
    vec3_t diff;
    VectorSubtract(a, b, diff);
    return VectorLength(diff);
}

/* Terrain vertex structure */
typedef struct {
    vec3_t  position;
    vec3_t  normal;
    float   u, v;               /* Texture coordinates */
    float   splatWeights[4];    /* Texture layer weights */
} terrain_vertex_t;

/* Heightmap structure */
typedef struct {
    int     width, height;      /* Grid dimensions */
    float   *heights;           /* height[y * width + x] */
    float   cellSize;           /* World units per grid cell */
    float   heightScale;        /* Multiplier for height values */
    vec3_t  origin;             /* World position of grid corner (0,0) */
} heightmap_t;

/* Forward declaration for neighbor pointers */
struct terrain_chunk_s;

/* Terrain chunk structure */
typedef struct terrain_chunk_s {
    int                     lod;            /* Current LOD level (0 = highest detail) */
    int                     gridX, gridZ;   /* Position in chunk grid */
    vec3_t                  mins, maxs;     /* Bounding box */
    terrain_vertex_t        *vertices;      /* Vertex buffer */
    int                     numVertices;
    int                     *indices;       /* Index buffer */
    int                     numIndices;
    qboolean                dirty;          /* Needs rebuild */
    struct terrain_chunk_s  *neighbors[4];  /* N, E, S, W */
} terrain_chunk_t;

/* Main terrain structure */
typedef struct {
    heightmap_t         heightmap;
    terrain_chunk_t     **chunks;           /* 2D array of chunks */
    int                 numChunksX, numChunksZ;
    int                 totalChunks;
    unsigned int        *textureIDs;        /* OpenGL texture IDs */
    int                 numTextures;
    unsigned char       *splatmap;          /* Texture blend weights per cell */
} terrain_t;

/* Collision result structure */
typedef struct {
    qboolean    hit;
    vec3_t      point;          /* Point of collision */
    vec3_t      normal;         /* Surface normal at collision */
    float       distance;       /* Distance along ray */
} terrain_collision_t;

#endif /* QUAKEDEF_H */
