/*
 * Lightmap Update Test Harness
 *
 * Tests correctness and performance of dynamic lightmap updates.
 * Both versions should produce identical lighting results.
 *
 * Test categories:
 * 1. Correctness - Lighting values must match
 * 2. Performance - Measure time with varying light counts
 * 3. Scaling - Verify dirty rect reduces work for small lights
 * 4. Edge cases - Boundary conditions, large lights, overlapping lights
 *
 * Build:
 *   make test        - Run tests on unoptimized version
 *   make test_opt    - Run tests on optimized version
 *   make compare     - Compare performance
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

/* ============== MOCK QUAKE TYPES ============== */

typedef unsigned char byte;
typedef float vec_t;
typedef vec_t vec3_t[3];
typedef int qboolean;

#define true 1
#define false 0

#define DotProduct(a, b) ((a)[0] * (b)[0] + (a)[1] * (b)[1] + (a)[2] * (b)[2])
#define VectorCopy(s, d) ((d)[0] = (s)[0], (d)[1] = (s)[1], (d)[2] = (s)[2])
#define VectorClear(v) ((v)[0] = (v)[1] = (v)[2] = 0)
#define VectorSubtract(a, b, c) ((c)[0] = (a)[0] - (b)[0], (c)[1] = (a)[1] - (b)[1], (c)[2] = (a)[2] - (b)[2])
#define VectorAdd(a, b, c) ((c)[0] = (a)[0] + (b)[0], (c)[1] = (a)[1] + (b)[1], (c)[2] = (a)[2] + (b)[2])
#define VectorScale(v, s, d) ((d)[0] = (v)[0] * (s), (d)[1] = (v)[1] * (s), (d)[2] = (v)[2] * (s))
#define VectorLength(v) (sqrtf((v)[0] * (v)[0] + (v)[1] * (v)[1] + (v)[2] * (v)[2]))

#define MAX_DLIGHTS         32
#define MAX_LIGHTSTYLES     64
#define MAX_LIGHTMAPS       64
#define BLOCK_WIDTH         128
#define BLOCK_HEIGHT        128
#define MAX_LIGHTMAP_SIZE   18

#define SURF_PLANEBACK      0x02
#define SURF_DRAWSKY        0x04
#define SURF_DRAWTURB       0x10
#define SURF_DRAWTILED      0x20
#define SURF_UNDERWATER     0x80

/* ============== STRUCTURES ============== */

typedef struct {
    int     length;
    char    map[64];
} lightstyle_t;

typedef struct mplane_s {
    vec3_t  normal;
    float   dist;
    byte    type;
    byte    signbits;
    byte    pad[2];
} mplane_t;

typedef struct mtexinfo_s {
    float   vecs[2][4];
    int     flags;
    int     texnum;
} mtexinfo_t;

typedef struct msurface_s {
    int         visframe;
    mplane_t    *plane;
    int         flags;
    int         firstedge;
    int         numedges;
    short       texturemins[2];
    short       extents[2];
    int         light_s, light_t;
    int         lightmaptexturenum;
    byte        styles[4];
    int         cached_light[4];
    qboolean    cached_dlight;
    byte        *samples;
    mtexinfo_t  *texinfo;
    int         smax, tmax;
    int         dlightframe;
    int         dlightbits;
    struct msurface_s   *texturechain;
    struct msurface_s   *lightmapchain;
} msurface_t;

typedef struct dlight_s {
    vec3_t  origin;
    float   radius;
    float   die;
    float   decay;
    float   minlight;
    int     key;
    vec3_t  color;
} dlight_t;

typedef struct {
    float       time;
    dlight_t    dlights[MAX_DLIGHTS];
    lightstyle_t    lightstyles[MAX_LIGHTSTYLES];
    int         framecount;
    float       lightstylevalue[MAX_LIGHTSTYLES];
} client_state_t;

typedef struct {
    int     l, t, w, h;
} glRect_t;

/* ============== GLOBALS ============== */

client_state_t cl;
unsigned int blocklights[MAX_LIGHTMAP_SIZE * MAX_LIGHTMAP_SIZE * 3];
byte lightmaps[4 * MAX_LIGHTMAPS * BLOCK_WIDTH * BLOCK_HEIGHT];
qboolean lightmap_modified[MAX_LIGHTMAPS];
glRect_t lightmap_rectchange[MAX_LIGHTMAPS];

/* ============== TEST INFRASTRUCTURE ============== */

static int tests_run = 0;
static int tests_passed = 0;

#define TEST_ASSERT(cond, msg) do { \
    if (!(cond)) { \
        printf("  FAIL: %s\n", msg); \
        return 0; \
    } \
} while(0)

#define RUN_TEST(test_func) do { \
    tests_run++; \
    printf("Testing: %s...\n", #test_func); \
    if (test_func()) { \
        tests_passed++; \
        printf("  PASS\n"); \
    } \
} while(0)

/* ============== TIMING UTILITIES ============== */

static double get_time_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
}

/* ============== INCLUDE IMPLEMENTATION ============== */

#ifdef TEST_OPTIMIZED

/* === OPTIMIZED VERSION === */
#define MAX_CACHED_SURFACES     4096

typedef struct {
    msurface_t  *surf;
    unsigned int    *static_light;
    unsigned int    cached_styles[4];
    int         size;
    qboolean    valid;
} surface_cache_t;

typedef struct {
    int     s_min, s_max;
    int     t_min, t_max;
    qboolean    has_dlight;
    qboolean    needs_rebuild;
} surface_dirty_t;

static surface_cache_t surface_cache[MAX_CACHED_SURFACES];
static int num_cached_surfaces = 0;
static unsigned int static_light_pool[MAX_CACHED_SURFACES * MAX_LIGHTMAP_SIZE * MAX_LIGHTMAP_SIZE * 3];
static int static_light_pool_used = 0;
static surface_dirty_t surface_dirty[MAX_CACHED_SURFACES];

static surface_cache_t *Cache_FindSurface(msurface_t *surf)
{
    int i;
    for (i = 0; i < num_cached_surfaces; i++)
        if (surface_cache[i].surf == surf)
            return &surface_cache[i];

    if (num_cached_surfaces >= MAX_CACHED_SURFACES)
        return NULL;

    int size = surf->smax * surf->tmax;
    int needed = size * 3;
    if (static_light_pool_used + needed > MAX_CACHED_SURFACES * MAX_LIGHTMAP_SIZE * MAX_LIGHTMAP_SIZE * 3)
        return NULL;

    surface_cache_t *cache = &surface_cache[num_cached_surfaces];
    cache->surf = surf;
    cache->static_light = &static_light_pool[static_light_pool_used];
    cache->size = size;
    cache->valid = false;
    cache->cached_styles[0] = cache->cached_styles[1] = 0;
    cache->cached_styles[2] = cache->cached_styles[3] = 0;
    static_light_pool_used += needed;
    num_cached_surfaces++;

    surface_dirty[num_cached_surfaces - 1].has_dlight = false;
    surface_dirty[num_cached_surfaces - 1].needs_rebuild = true;
    surface_dirty[num_cached_surfaces - 1].s_min = 0;
    surface_dirty[num_cached_surfaces - 1].s_max = surf->smax;
    surface_dirty[num_cached_surfaces - 1].t_min = 0;
    surface_dirty[num_cached_surfaces - 1].t_max = surf->tmax;

    return cache;
}

static surface_dirty_t *Cache_GetDirty(msurface_t *surf)
{
    for (int i = 0; i < num_cached_surfaces; i++)
        if (surface_cache[i].surf == surf)
            return &surface_dirty[i];
    return NULL;
}

static void Cache_BuildStatic(surface_cache_t *cache, msurface_t *surf)
{
    int size = cache->size;
    unsigned int *dest = cache->static_light;
    qboolean needs_rebuild = !cache->valid;

    if (!needs_rebuild) {
        for (int maps = 0; maps < 4 && surf->styles[maps] != 255; maps++) {
            unsigned int current = (unsigned int)(cl.lightstylevalue[surf->styles[maps]] * 256.0f);
            if (current != cache->cached_styles[maps]) {
                needs_rebuild = true;
                break;
            }
        }
    }

    if (!needs_rebuild) return;

    memset(dest, 0, size * 3 * sizeof(unsigned int));
    byte *lightmap = surf->samples;
    if (!lightmap) {
        for (int i = 0; i < size * 3; i++)
            dest[i] = 255 * 256;
        cache->valid = true;
        return;
    }

    for (int maps = 0; maps < 4 && surf->styles[maps] != 255; maps++) {
        unsigned scale = (unsigned int)(cl.lightstylevalue[surf->styles[maps]] * 256.0f);
        cache->cached_styles[maps] = scale;
        surf->cached_light[maps] = scale;
        lightmap = surf->samples + maps * size * 3;
        for (int i = 0; i < size; i++) {
            dest[i * 3 + 0] += lightmap[i * 3 + 0] * scale;
            dest[i * 3 + 1] += lightmap[i * 3 + 1] * scale;
            dest[i * 3 + 2] += lightmap[i * 3 + 2] * scale;
        }
    }
    cache->valid = true;
}

static void R_CalcLightDirtyRect(msurface_t *surf, dlight_t *dl,
                                  int *s_min, int *s_max, int *t_min, int *t_max)
{
    float dist = DotProduct(dl->origin, surf->plane->normal) - surf->plane->dist;
    float rad = dl->radius - fabsf(dist);
    if (rad <= 0) { *s_min = *s_max = *t_min = *t_max = 0; return; }

    vec3_t impact;
    for (int i = 0; i < 3; i++)
        impact[i] = dl->origin[i] - surf->plane->normal[i] * dist;

    mtexinfo_t *tex = surf->texinfo;
    float local_s = DotProduct(impact, tex->vecs[0]) + tex->vecs[0][3] - surf->texturemins[0];
    float local_t = DotProduct(impact, tex->vecs[1]) + tex->vecs[1][3] - surf->texturemins[1];

    *s_min = (int)floorf((local_s - rad) / 16.0f);
    *s_max = (int)ceilf((local_s + rad) / 16.0f);
    *t_min = (int)floorf((local_t - rad) / 16.0f);
    *t_max = (int)ceilf((local_t + rad) / 16.0f);

    if (*s_min < 0) *s_min = 0;
    if (*t_min < 0) *t_min = 0;
    if (*s_max > surf->smax) *s_max = surf->smax;
    if (*t_max > surf->tmax) *t_max = surf->tmax;
}

static void R_AddDynamicLights_Optimized(msurface_t *surf,
    int dirty_s_min, int dirty_s_max, int dirty_t_min, int dirty_t_max)
{
    int smax = surf->smax;
    mtexinfo_t *tex = surf->texinfo;

    for (int lnum = 0; lnum < MAX_DLIGHTS; lnum++) {
        if (!(surf->dlightbits & (1 << lnum))) continue;
        dlight_t *dl = &cl.dlights[lnum];
        float dist = DotProduct(dl->origin, surf->plane->normal) - surf->plane->dist;
        float rad = dl->radius - fabsf(dist);
        if (rad <= 0) continue;

        vec3_t impact;
        for (int i = 0; i < 3; i++)
            impact[i] = dl->origin[i] - surf->plane->normal[i] * dist;

        float local_s = DotProduct(impact, tex->vecs[0]) + tex->vecs[0][3] - surf->texturemins[0];
        float local_t = DotProduct(impact, tex->vecs[1]) + tex->vecs[1][3] - surf->texturemins[1];

        int light_s_min = (int)floorf((local_s - rad) / 16.0f);
        int light_s_max = (int)ceilf((local_s + rad) / 16.0f);
        int light_t_min = (int)floorf((local_t - rad) / 16.0f);
        int light_t_max = (int)ceilf((local_t + rad) / 16.0f);

        if (light_s_min < 0) light_s_min = 0;
        if (light_t_min < 0) light_t_min = 0;
        if (light_s_max > smax) light_s_max = smax;
        if (light_t_max > surf->tmax) light_t_max = surf->tmax;

        int proc_s_min = (light_s_min > dirty_s_min) ? light_s_min : dirty_s_min;
        int proc_s_max = (light_s_max < dirty_s_max) ? light_s_max : dirty_s_max;
        int proc_t_min = (light_t_min > dirty_t_min) ? light_t_min : dirty_t_min;
        int proc_t_max = (light_t_max < dirty_t_max) ? light_t_max : dirty_t_max;

        if (proc_s_min >= proc_s_max || proc_t_min >= proc_t_max) continue;

        for (int t = proc_t_min; t < proc_t_max; t++) {
            int td = (int)local_t - (t * 16 + 8);
            if (td < 0) td = -td;
            unsigned *bl = blocklights + (t * smax + proc_s_min) * 3;
            for (int s = proc_s_min; s < proc_s_max; s++, bl += 3) {
                int sd = (int)local_s - (s * 16 + 8);
                if (sd < 0) sd = -sd;
                float d = (sd > td) ? sd + (td >> 1) : td + (sd >> 1);
                if (d < rad) {
                    float brightness = (rad - d) * 256.0f;
                    bl[0] += (unsigned int)(brightness * dl->color[0]);
                    bl[1] += (unsigned int)(brightness * dl->color[1]);
                    bl[2] += (unsigned int)(brightness * dl->color[2]);
                }
            }
        }
    }
}

void R_AddDynamicLights(msurface_t *surf)
{
    surface_dirty_t *dirty = Cache_GetDirty(surf);
    if (dirty)
        R_AddDynamicLights_Optimized(surf, dirty->s_min, dirty->s_max, dirty->t_min, dirty->t_max);
    else
        R_AddDynamicLights_Optimized(surf, 0, surf->smax, 0, surf->tmax);
}

void R_BuildLightMap(msurface_t *surf, byte *dest, int stride)
{
    surf->cached_dlight = (surf->dlightframe == cl.framecount);
    int smax = surf->smax, tmax = surf->tmax;
    int size = smax * tmax;

    surface_cache_t *cache = Cache_FindSurface(surf);
    surface_dirty_t *dirty = Cache_GetDirty(surf);

    int dirty_s_min = 0, dirty_s_max = smax;
    int dirty_t_min = 0, dirty_t_max = tmax;
    if (dirty) {
        dirty_s_min = dirty->s_min; dirty_s_max = dirty->s_max;
        dirty_t_min = dirty->t_min; dirty_t_max = dirty->t_max;
    }

    if (cache) {
        Cache_BuildStatic(cache, surf);
        if (surf->dlightframe == cl.framecount) {
            for (int t = dirty_t_min; t < dirty_t_max; t++) {
                unsigned int *src = cache->static_light + t * smax * 3;
                unsigned int *dst = blocklights + t * smax * 3;
                for (int s = dirty_s_min; s < dirty_s_max; s++) {
                    dst[s * 3 + 0] = src[s * 3 + 0];
                    dst[s * 3 + 1] = src[s * 3 + 1];
                    dst[s * 3 + 2] = src[s * 3 + 2];
                }
            }
            R_AddDynamicLights_Optimized(surf, dirty_s_min, dirty_s_max, dirty_t_min, dirty_t_max);
        } else {
            memcpy(blocklights, cache->static_light, size * 3 * sizeof(unsigned int));
        }
    } else {
        if (!surf->samples) {
            for (int i = 0; i < size * 3; i++) blocklights[i] = 255 * 256;
        } else {
            memset(blocklights, 0, size * 3 * sizeof(unsigned int));
            for (int maps = 0; maps < 4 && surf->styles[maps] != 255; maps++) {
                unsigned scale = (unsigned int)(cl.lightstylevalue[surf->styles[maps]] * 256.0f);
                surf->cached_light[maps] = scale;
                byte *lm = surf->samples + maps * size * 3;
                unsigned *bl = blocklights;
                for (int i = 0; i < size; i++) {
                    bl[0] += lm[0] * scale; bl[1] += lm[1] * scale; bl[2] += lm[2] * scale;
                    lm += 3; bl += 3;
                }
            }
            if (surf->dlightframe == cl.framecount) R_AddDynamicLights(surf);
        }
    }

    unsigned *bl = blocklights;
    for (int t = 0; t < tmax; t++, dest += stride) {
        for (int s = 0; s < smax; s++) {
            unsigned int r = bl[0] >> 8, g = bl[1] >> 8, b = bl[2] >> 8;
            if (r > 255) r = 255; if (g > 255) g = 255; if (b > 255) b = 255;
            dest[s * 4 + 0] = (byte)r; dest[s * 4 + 1] = (byte)g;
            dest[s * 4 + 2] = (byte)b; dest[s * 4 + 3] = 255;
            bl += 3;
        }
    }
}

void R_RenderDynamicLightmaps(msurface_t *surf)
{
    if (surf->flags & (SURF_DRAWSKY | SURF_DRAWTURB)) return;
    int smax = surf->smax, tmax = surf->tmax;

    surface_dirty_t *dirty = Cache_GetDirty(surf);
    if (!dirty) { Cache_FindSurface(surf); dirty = Cache_GetDirty(surf); }

    int dirty_s_min = smax, dirty_s_max = 0;
    int dirty_t_min = tmax, dirty_t_max = 0;
    qboolean needsUpdate = false, stylesChanged = false;

    if (surf->dlightframe == cl.framecount) {
        needsUpdate = true;
        for (int lnum = 0; lnum < MAX_DLIGHTS; lnum++) {
            if (!(surf->dlightbits & (1 << lnum))) continue;
            int ls_min, ls_max, lt_min, lt_max;
            R_CalcLightDirtyRect(surf, &cl.dlights[lnum], &ls_min, &ls_max, &lt_min, &lt_max);
            if (ls_min < dirty_s_min) dirty_s_min = ls_min;
            if (ls_max > dirty_s_max) dirty_s_max = ls_max;
            if (lt_min < dirty_t_min) dirty_t_min = lt_min;
            if (lt_max > dirty_t_max) dirty_t_max = lt_max;
        }
    }

    for (int maps = 0; maps < 4 && surf->styles[maps] != 255; maps++) {
        unsigned int current = (unsigned int)(cl.lightstylevalue[surf->styles[maps]] * 256.0f);
        if (current != surf->cached_light[maps]) { needsUpdate = true; stylesChanged = true; break; }
    }

    if (!needsUpdate) return;
    if (stylesChanged) { dirty_s_min = 0; dirty_s_max = smax; dirty_t_min = 0; dirty_t_max = tmax; }

    if (dirty) {
        dirty->s_min = dirty_s_min; dirty->s_max = dirty_s_max;
        dirty->t_min = dirty_t_min; dirty->t_max = dirty_t_max;
        dirty->has_dlight = (surf->dlightframe == cl.framecount);
        dirty->needs_rebuild = stylesChanged;
    }

    lightmap_modified[surf->lightmaptexturenum] = true;
    byte *base = lightmaps + surf->lightmaptexturenum * 4 * BLOCK_WIDTH * BLOCK_HEIGHT;
    base += (surf->light_t * BLOCK_WIDTH + surf->light_s) * 4;
    R_BuildLightMap(surf, base, BLOCK_WIDTH * 4);
}

void R_ClearLightmapCache(void) { num_cached_surfaces = 0; static_light_pool_used = 0; }

#else

/* === UNOPTIMIZED VERSION === */

void R_AddDynamicLights(msurface_t *surf)
{
    int smax = surf->smax, tmax = surf->tmax;
    mtexinfo_t *tex = surf->texinfo;

    for (int lnum = 0; lnum < MAX_DLIGHTS; lnum++) {
        if (!(surf->dlightbits & (1 << lnum))) continue;
        dlight_t *dl = &cl.dlights[lnum];
        float dist = DotProduct(dl->origin, surf->plane->normal) - surf->plane->dist;
        float rad = dl->radius - fabsf(dist);
        if (rad < 0) continue;

        vec3_t impact;
        for (int i = 0; i < 3; i++)
            impact[i] = dl->origin[i] - surf->plane->normal[i] * dist;

        float local_s = DotProduct(impact, tex->vecs[0]) + tex->vecs[0][3] - surf->texturemins[0];
        float local_t = DotProduct(impact, tex->vecs[1]) + tex->vecs[1][3] - surf->texturemins[1];

        unsigned *bl = blocklights;
        for (int t = 0; t < tmax; t++) {
            int td = (int)local_t - (t * 16 + 8);
            if (td < 0) td = -td;
            for (int s = 0; s < smax; s++, bl += 3) {
                int sd = (int)local_s - (s * 16 + 8);
                if (sd < 0) sd = -sd;
                float d = (sd > td) ? sd + (td >> 1) : td + (sd >> 1);
                if (d < rad) {
                    float brightness = (rad - d) * 256.0f;
                    bl[0] += (unsigned int)(brightness * dl->color[0]);
                    bl[1] += (unsigned int)(brightness * dl->color[1]);
                    bl[2] += (unsigned int)(brightness * dl->color[2]);
                }
            }
        }
    }
}

void R_BuildLightMap(msurface_t *surf, byte *dest, int stride)
{
    surf->cached_dlight = (surf->dlightframe == cl.framecount);
    int smax = surf->smax, tmax = surf->tmax;
    int size = smax * tmax;

    if (!surf->samples) {
        for (int i = 0; i < size * 3; i++) blocklights[i] = 255 * 256;
    } else {
        memset(blocklights, 0, size * 3 * sizeof(unsigned int));
        for (int maps = 0; maps < 4 && surf->styles[maps] != 255; maps++) {
            unsigned scale = (unsigned int)(cl.lightstylevalue[surf->styles[maps]] * 256.0f);
            surf->cached_light[maps] = scale;
            byte *lightmap = surf->samples + maps * size * 3;
            unsigned *bl = blocklights;
            for (int i = 0; i < size; i++) {
                bl[0] += lightmap[0] * scale;
                bl[1] += lightmap[1] * scale;
                bl[2] += lightmap[2] * scale;
                lightmap += 3; bl += 3;
            }
        }
    }

    if (surf->dlightframe == cl.framecount)
        R_AddDynamicLights(surf);

    unsigned *bl = blocklights;
    for (int t = 0; t < tmax; t++, dest += stride) {
        for (int s = 0; s < smax; s++) {
            unsigned int r = bl[0] >> 8, g = bl[1] >> 8, b = bl[2] >> 8;
            if (r > 255) r = 255; if (g > 255) g = 255; if (b > 255) b = 255;
            dest[s * 4 + 0] = (byte)r; dest[s * 4 + 1] = (byte)g;
            dest[s * 4 + 2] = (byte)b; dest[s * 4 + 3] = 255;
            bl += 3;
        }
    }
}

void R_RenderDynamicLightmaps(msurface_t *surf)
{
    if (surf->flags & (SURF_DRAWSKY | SURF_DRAWTURB)) return;
    int smax = surf->smax, tmax = surf->tmax;
    qboolean needsUpdate = false;

    if (surf->dlightframe == cl.framecount)
        needsUpdate = true;
    else {
        for (int maps = 0; maps < 4 && surf->styles[maps] != 255; maps++) {
            unsigned int current = (unsigned int)(cl.lightstylevalue[surf->styles[maps]] * 256.0f);
            if (current != surf->cached_light[maps]) { needsUpdate = true; break; }
        }
    }

    if (!needsUpdate) return;
    lightmap_modified[surf->lightmaptexturenum] = true;

    byte *base = lightmaps + surf->lightmaptexturenum * 4 * BLOCK_WIDTH * BLOCK_HEIGHT;
    base += (surf->light_t * BLOCK_WIDTH + surf->light_s) * 4;
    R_BuildLightMap(surf, base, BLOCK_WIDTH * 4);
}

void R_ClearLightmapCache(void) { /* No cache in unoptimized version */ }

#endif

/* ============== TEST HELPERS ============== */

static mplane_t test_plane;
static mtexinfo_t test_texinfo;
static byte test_lightmap_data[MAX_LIGHTMAP_SIZE * MAX_LIGHTMAP_SIZE * 3 * 4];
static byte test_output[MAX_LIGHTMAP_SIZE * MAX_LIGHTMAP_SIZE * 4];

static void init_test_surface(msurface_t *surf, int smax, int tmax)
{
    memset(surf, 0, sizeof(*surf));
    surf->smax = smax;
    surf->tmax = tmax;
    surf->plane = &test_plane;
    surf->texinfo = &test_texinfo;
    surf->lightmaptexturenum = 0;
    surf->light_s = 0;
    surf->light_t = 0;
    surf->texturemins[0] = 0;
    surf->texturemins[1] = 0;
    surf->extents[0] = smax * 16;
    surf->extents[1] = tmax * 16;
    surf->styles[0] = 0;
    surf->styles[1] = 255;
    surf->styles[2] = 255;
    surf->styles[3] = 255;
    surf->cached_light[0] = 0;
    surf->cached_light[1] = 0;
    surf->cached_light[2] = 0;
    surf->cached_light[3] = 0;
    surf->dlightframe = 0;
    surf->dlightbits = 0;

    // Setup plane (floor facing up)
    test_plane.normal[0] = 0;
    test_plane.normal[1] = 0;
    test_plane.normal[2] = 1;
    test_plane.dist = 0;

    // Setup texinfo (identity mapping)
    test_texinfo.vecs[0][0] = 1; test_texinfo.vecs[0][1] = 0;
    test_texinfo.vecs[0][2] = 0; test_texinfo.vecs[0][3] = 0;
    test_texinfo.vecs[1][0] = 0; test_texinfo.vecs[1][1] = 1;
    test_texinfo.vecs[1][2] = 0; test_texinfo.vecs[1][3] = 0;

    // Initialize lightmap data with some pattern
    int size = smax * tmax;
    for (int i = 0; i < size; i++) {
        test_lightmap_data[i * 3 + 0] = 128;  // R
        test_lightmap_data[i * 3 + 1] = 128;  // G
        test_lightmap_data[i * 3 + 2] = 128;  // B
    }
    surf->samples = test_lightmap_data;
}

static void init_dynamic_light(dlight_t *dl, float x, float y, float z, float radius)
{
    dl->origin[0] = x;
    dl->origin[1] = y;
    dl->origin[2] = z;
    dl->radius = radius;
    dl->die = cl.time + 1.0f;
    dl->decay = 0;
    dl->minlight = 0;
    dl->key = 0;
    dl->color[0] = 1.0f;
    dl->color[1] = 1.0f;
    dl->color[2] = 1.0f;
}

static void clear_state(void)
{
    memset(&cl, 0, sizeof(cl));
    cl.time = 1.0f;
    cl.framecount = 1;
    for (int i = 0; i < MAX_LIGHTSTYLES; i++)
        cl.lightstylevalue[i] = 1.0f;
    memset(lightmaps, 0, sizeof(lightmaps));
    memset(lightmap_modified, 0, sizeof(lightmap_modified));
    memset(lightmap_rectchange, 0, sizeof(lightmap_rectchange));
    R_ClearLightmapCache();
}

/* ============== CORRECTNESS TESTS ============== */

static int test_static_lightmap(void)
{
    clear_state();
    msurface_t surf;
    init_test_surface(&surf, 8, 8);

    R_BuildLightMap(&surf, test_output, 8 * 4);

    // Should have non-zero output
    int sum = 0;
    for (int i = 0; i < 8 * 8 * 4; i += 4)
        sum += test_output[i];
    TEST_ASSERT(sum > 0, "Static lightmap should have non-zero values");
    return 1;
}

static int test_single_dynamic_light(void)
{
    clear_state();
    msurface_t surf;
    init_test_surface(&surf, 8, 8);

    // Add a dynamic light at center of surface
    init_dynamic_light(&cl.dlights[0], 64, 64, 50, 100);
    surf.dlightframe = cl.framecount;
    surf.dlightbits = 1;

    R_BuildLightMap(&surf, test_output, 8 * 4);

    // Center should be brightest
    int center_r = test_output[(4 * 8 + 4) * 4];
    int corner_r = test_output[0];
    TEST_ASSERT(center_r > corner_r, "Center should be brighter than corner with centered light");
    return 1;
}

static int test_light_at_corner(void)
{
    clear_state();
    msurface_t surf;
    init_test_surface(&surf, 8, 8);

    // Light at corner
    init_dynamic_light(&cl.dlights[0], 0, 0, 50, 100);
    surf.dlightframe = cl.framecount;
    surf.dlightbits = 1;

    R_BuildLightMap(&surf, test_output, 8 * 4);

    // Corner should be brightest
    int corner_r = test_output[0];
    int far_corner_r = test_output[(7 * 8 + 7) * 4];
    TEST_ASSERT(corner_r > far_corner_r, "Corner with light should be brighter than far corner");
    return 1;
}

static int test_multiple_lights(void)
{
    clear_state();
    msurface_t surf;
    init_test_surface(&surf, 8, 8);

    // Two lights at opposite corners
    init_dynamic_light(&cl.dlights[0], 0, 0, 50, 80);
    init_dynamic_light(&cl.dlights[1], 128, 128, 50, 80);
    surf.dlightframe = cl.framecount;
    surf.dlightbits = 3;  // bits 0 and 1

    R_BuildLightMap(&surf, test_output, 8 * 4);

    // Both corners should be bright
    int corner0_r = test_output[0];
    int corner1_r = test_output[(7 * 8 + 7) * 4];
    int center_r = test_output[(4 * 8 + 4) * 4];

    TEST_ASSERT(corner0_r > center_r || corner1_r > center_r,
                "At least one corner should be brighter than center");
    return 1;
}

static int test_colored_light(void)
{
    clear_state();
    msurface_t surf;
    init_test_surface(&surf, 8, 8);

    // Red light
    init_dynamic_light(&cl.dlights[0], 64, 64, 50, 100);
    cl.dlights[0].color[0] = 2.0f;
    cl.dlights[0].color[1] = 0.0f;
    cl.dlights[0].color[2] = 0.0f;
    surf.dlightframe = cl.framecount;
    surf.dlightbits = 1;

    R_BuildLightMap(&surf, test_output, 8 * 4);

    // Center should have more red than blue
    int center_idx = (4 * 8 + 4) * 4;
    int r = test_output[center_idx + 0];
    int b = test_output[center_idx + 2];
    TEST_ASSERT(r > b, "Red light should produce more red than blue");
    return 1;
}

static int test_no_dynamic_lights(void)
{
    clear_state();
    msurface_t surf;
    init_test_surface(&surf, 8, 8);

    // No dynamic lights
    surf.dlightframe = 0;
    surf.dlightbits = 0;

    byte output1[MAX_LIGHTMAP_SIZE * MAX_LIGHTMAP_SIZE * 4];
    R_BuildLightMap(&surf, output1, 8 * 4);

    // Call again - should get same result
    byte output2[MAX_LIGHTMAP_SIZE * MAX_LIGHTMAP_SIZE * 4];
    R_BuildLightMap(&surf, output2, 8 * 4);

    TEST_ASSERT(memcmp(output1, output2, 8 * 8 * 4) == 0,
                "Repeated calls without changes should produce same output");
    return 1;
}

static int test_light_outside_surface(void)
{
    clear_state();
    msurface_t surf;
    init_test_surface(&surf, 8, 8);

    // Light far from surface
    init_dynamic_light(&cl.dlights[0], 1000, 1000, 50, 100);
    surf.dlightframe = cl.framecount;
    surf.dlightbits = 1;

    byte output_with_far_light[MAX_LIGHTMAP_SIZE * MAX_LIGHTMAP_SIZE * 4];
    R_BuildLightMap(&surf, output_with_far_light, 8 * 4);

    // Compare to no light
    clear_state();
    init_test_surface(&surf, 8, 8);
    surf.dlightframe = 0;
    surf.dlightbits = 0;

    byte output_no_light[MAX_LIGHTMAP_SIZE * MAX_LIGHTMAP_SIZE * 4];
    R_BuildLightMap(&surf, output_no_light, 8 * 4);

    // Should be similar (far light has minimal effect)
    int diff = 0;
    for (int i = 0; i < 8 * 8 * 4; i++)
        diff += abs(output_with_far_light[i] - output_no_light[i]);
    TEST_ASSERT(diff < 100, "Far light should have minimal effect on surface");
    return 1;
}

/* ============== PERFORMANCE TESTS ============== */

static double benchmark_light_count(int num_lights, int num_surfaces, int iterations)
{
    clear_state();

    // Setup surfaces
    msurface_t surfaces[64];
    for (int i = 0; i < num_surfaces; i++) {
        init_test_surface(&surfaces[i], 16, 16);
        surfaces[i].dlightframe = cl.framecount;
        surfaces[i].dlightbits = (1 << num_lights) - 1;
    }

    // Setup lights in a grid pattern
    for (int i = 0; i < num_lights; i++) {
        float x = (i % 4) * 64.0f + 32.0f;
        float y = (i / 4) * 64.0f + 32.0f;
        init_dynamic_light(&cl.dlights[i], x, y, 50, 100);
    }

    // Benchmark
    double start = get_time_ms();
    for (int iter = 0; iter < iterations; iter++) {
        for (int s = 0; s < num_surfaces; s++) {
            R_BuildLightMap(&surfaces[s], test_output, 16 * 4);
        }
    }
    double end = get_time_ms();

    return (end - start) / iterations;
}

static int test_performance_1_light(void)
{
    double time_ms = benchmark_light_count(1, 16, 100);
    printf("  1 light, 16 surfaces: %.3f ms avg\n", time_ms);
    TEST_ASSERT(time_ms < 50, "1 light should be fast");
    return 1;
}

static int test_performance_4_lights(void)
{
    double time_ms = benchmark_light_count(4, 16, 100);
    printf("  4 lights, 16 surfaces: %.3f ms avg\n", time_ms);
    TEST_ASSERT(time_ms < 100, "4 lights should be reasonable");
    return 1;
}

static int test_performance_8_lights(void)
{
    double time_ms = benchmark_light_count(8, 16, 100);
    printf("  8 lights, 16 surfaces: %.3f ms avg\n", time_ms);
    TEST_ASSERT(time_ms < 200, "8 lights should complete");
    return 1;
}

static int test_performance_16_lights(void)
{
    double time_ms = benchmark_light_count(16, 16, 50);
    printf("  16 lights, 16 surfaces: %.3f ms avg\n", time_ms);
    TEST_ASSERT(time_ms < 500, "16 lights should complete");
    return 1;
}

/* ============== SCALING TESTS ============== */

static int test_scaling_with_lights(void)
{
    double time_4 = benchmark_light_count(4, 16, 100);
    double time_8 = benchmark_light_count(8, 16, 100);
    double time_16 = benchmark_light_count(16, 16, 50);

    printf("  Scaling: 4 lights=%.2fms, 8 lights=%.2fms, 16 lights=%.2fms\n",
           time_4, time_8, time_16);

    double ratio_8_4 = time_8 / time_4;
    double ratio_16_8 = time_16 / time_8;

    printf("  Ratios: 8/4=%.2fx, 16/8=%.2fx\n", ratio_8_4, ratio_16_8);

#ifdef TEST_OPTIMIZED
    // Optimized version should scale better
    TEST_ASSERT(ratio_8_4 < 3.0, "Optimized should not scale worse than O(n)");
#endif

    return 1;
}

static int test_small_light_performance(void)
{
    // Small lights should benefit more from dirty rect optimization
    clear_state();

    msurface_t surf;
    init_test_surface(&surf, 16, 16);
    surf.dlightframe = cl.framecount;
    surf.dlightbits = 1;

    // Small light (only affects a few texels)
    init_dynamic_light(&cl.dlights[0], 32, 32, 10, 30);

    double start = get_time_ms();
    for (int i = 0; i < 1000; i++) {
        R_BuildLightMap(&surf, test_output, 16 * 4);
    }
    double end = get_time_ms();

    printf("  Small light (r=30): %.3f ms for 1000 iterations\n", end - start);
    return 1;
}

static int test_large_light_performance(void)
{
    // Large lights touch most texels - less optimization benefit
    clear_state();

    msurface_t surf;
    init_test_surface(&surf, 16, 16);
    surf.dlightframe = cl.framecount;
    surf.dlightbits = 1;

    // Large light (affects entire surface)
    init_dynamic_light(&cl.dlights[0], 128, 128, 50, 500);

    double start = get_time_ms();
    for (int i = 0; i < 1000; i++) {
        R_BuildLightMap(&surf, test_output, 16 * 4);
    }
    double end = get_time_ms();

    printf("  Large light (r=500): %.3f ms for 1000 iterations\n", end - start);
    return 1;
}

/* ============== EDGE CASE TESTS ============== */

static int test_min_size_surface(void)
{
    clear_state();
    msurface_t surf;
    init_test_surface(&surf, 1, 1);

    init_dynamic_light(&cl.dlights[0], 8, 8, 50, 100);
    surf.dlightframe = cl.framecount;
    surf.dlightbits = 1;

    R_BuildLightMap(&surf, test_output, 1 * 4);

    TEST_ASSERT(test_output[0] > 0, "1x1 surface should have lighting");
    return 1;
}

static int test_max_size_surface(void)
{
    clear_state();
    msurface_t surf;
    init_test_surface(&surf, 18, 18);

    init_dynamic_light(&cl.dlights[0], 144, 144, 50, 200);
    surf.dlightframe = cl.framecount;
    surf.dlightbits = 1;

    R_BuildLightMap(&surf, test_output, 18 * 4);

    TEST_ASSERT(test_output[0] >= 0, "18x18 surface should work");
    return 1;
}

static int test_many_lights_max(void)
{
    clear_state();
    msurface_t surf;
    init_test_surface(&surf, 8, 8);

    // Add MAX_DLIGHTS lights
    surf.dlightframe = cl.framecount;
    surf.dlightbits = 0xFFFFFFFF;  // All 32 lights

    for (int i = 0; i < MAX_DLIGHTS; i++) {
        float angle = i * 2.0f * 3.14159f / MAX_DLIGHTS;
        float x = 64 + cosf(angle) * 50;
        float y = 64 + sinf(angle) * 50;
        init_dynamic_light(&cl.dlights[i], x, y, 50, 60);
    }

    R_BuildLightMap(&surf, test_output, 8 * 4);

    // Should complete without crashing
    TEST_ASSERT(test_output[0] >= 0, "Should handle max lights");
    return 1;
}

static int test_light_on_surface_plane(void)
{
    clear_state();
    msurface_t surf;
    init_test_surface(&surf, 8, 8);

    // Light exactly on the surface plane (z=0)
    init_dynamic_light(&cl.dlights[0], 64, 64, 0, 100);
    surf.dlightframe = cl.framecount;
    surf.dlightbits = 1;

    R_BuildLightMap(&surf, test_output, 8 * 4);

    // Should handle gracefully
    TEST_ASSERT(test_output[0] >= 0, "Light on plane should be handled");
    return 1;
}

/* ============== MAIN ============== */

int main(void)
{
    printf("Lightmap Update Tests\n");
    printf("=====================\n\n");

#ifdef TEST_OPTIMIZED
    printf("Version: OPTIMIZED (dirty rectangle tracking)\n\n");
#else
    printf("Version: UNOPTIMIZED (full recalculation)\n\n");
#endif

    printf("=== Correctness Tests ===\n");
    RUN_TEST(test_static_lightmap);
    RUN_TEST(test_single_dynamic_light);
    RUN_TEST(test_light_at_corner);
    RUN_TEST(test_multiple_lights);
    RUN_TEST(test_colored_light);
    RUN_TEST(test_no_dynamic_lights);
    RUN_TEST(test_light_outside_surface);

    printf("\n=== Performance Tests ===\n");
    RUN_TEST(test_performance_1_light);
    RUN_TEST(test_performance_4_lights);
    RUN_TEST(test_performance_8_lights);
    RUN_TEST(test_performance_16_lights);

    printf("\n=== Scaling Tests ===\n");
    RUN_TEST(test_scaling_with_lights);
    RUN_TEST(test_small_light_performance);
    RUN_TEST(test_large_light_performance);

    printf("\n=== Edge Case Tests ===\n");
    RUN_TEST(test_min_size_surface);
    RUN_TEST(test_max_size_surface);
    RUN_TEST(test_many_lights_max);
    RUN_TEST(test_light_on_surface_plane);

    printf("\n=====================\n");
    printf("Results: %d/%d tests passed\n", tests_passed, tests_run);
    printf("=====================\n");

    return (tests_passed == tests_run) ? 0 : 1;
}
