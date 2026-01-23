/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// quakedef.h -- Primary Quake header for lighting system

#ifndef QUAKEDEF_H
#define QUAKEDEF_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

// Basic types
typedef unsigned char byte;
typedef float vec_t;
typedef vec_t vec3_t[3];
typedef int qboolean;

#define true 1
#define false 0

// Vector math
#define DotProduct(a, b) ((a)[0] * (b)[0] + (a)[1] * (b)[1] + (a)[2] * (b)[2])
#define VectorCopy(s, d) ((d)[0] = (s)[0], (d)[1] = (s)[1], (d)[2] = (s)[2])
#define VectorClear(v) ((v)[0] = (v)[1] = (v)[2] = 0)
#define VectorSubtract(a, b, c) ((c)[0] = (a)[0] - (b)[0], (c)[1] = (a)[1] - (b)[1], (c)[2] = (a)[2] - (b)[2])
#define VectorAdd(a, b, c) ((c)[0] = (a)[0] + (b)[0], (c)[1] = (a)[1] + (b)[1], (c)[2] = (a)[2] + (b)[2])
#define VectorScale(v, s, d) ((d)[0] = (v)[0] * (s), (d)[1] = (v)[1] * (s), (d)[2] = (v)[2] * (s))
#define VectorLength(v) (sqrtf((v)[0] * (v)[0] + (v)[1] * (v)[1] + (v)[2] * (v)[2]))

// Constants
#define MAX_DLIGHTS         32      // Maximum dynamic lights
#define MAX_LIGHTSTYLES     64      // Maximum light styles
#define MAX_LIGHTMAPS       64      // Maximum lightmap textures
#define BLOCK_WIDTH         128     // Lightmap texture width
#define BLOCK_HEIGHT        128     // Lightmap texture height
#define MAX_LIGHTMAP_SIZE   18      // Maximum surface lightmap size (18x18)

// Light styles (animated lights)
typedef struct {
    int     length;
    char    map[64];            // Light pattern string
} lightstyle_t;

// BSP plane
typedef struct mplane_s {
    vec3_t  normal;
    float   dist;
    byte    type;
    byte    signbits;
    byte    pad[2];
} mplane_t;

// Texture info for lightmap coordinates
typedef struct mtexinfo_s {
    float   vecs[2][4];         // [s/t][xyz offset]
    int     flags;
    int     texnum;
} mtexinfo_t;

// Surface structure
typedef struct msurface_s {
    int         visframe;       // Should be drawn when visframe == r_framecount

    mplane_t    *plane;
    int         flags;

    int         firstedge;      // Look up in model->surfedges[]
    int         numedges;

    short       texturemins[2]; // Smallest s/t position on surface
    short       extents[2];     // (texturemins + extents) bounds surface

    int         light_s, light_t;       // Lightmap position in atlas
    int         lightmaptexturenum;     // Which lightmap texture

    byte        styles[4];      // Index into lightstyles for animation
    int         cached_light[4];        // Cached light values for flicker detection
    qboolean    cached_dlight;          // True if dynamic light in cache

    byte        *samples;       // Static lightmap data [numstyles][width*height]

    mtexinfo_t  *texinfo;

    // Lightmap dimensions
    int         smax, tmax;     // Lightmap dimensions for this surface

    // Dynamic light tracking
    int         dlightframe;    // Frame number when dlights were last checked
    int         dlightbits;     // Bitmask of affecting dynamic lights

    // For linking in lists
    struct msurface_s   *texturechain;
    struct msurface_s   *lightmapchain;
} msurface_t;

// Dynamic light structure
typedef struct dlight_s {
    vec3_t  origin;
    float   radius;
    float   die;            // Stop lighting after this time
    float   decay;          // Drop this much each second
    float   minlight;       // Minimum light value
    int     key;            // Unique ID for caching
    vec3_t  color;          // RGB color (1.0 = normal)
} dlight_t;

// Global state
typedef struct {
    float       time;           // Current game time
    dlight_t    dlights[MAX_DLIGHTS];
    lightstyle_t    lightstyles[MAX_LIGHTSTYLES];
    int         framecount;     // Incremented every frame
    float       lightstylevalue[MAX_LIGHTSTYLES];   // 0-1 scale light value
} client_state_t;

extern client_state_t cl;

// Lightmap block for accumulation (8.8 fixed point)
extern unsigned int blocklights[MAX_LIGHTMAP_SIZE * MAX_LIGHTMAP_SIZE * 3];

// Lightmap storage
extern byte lightmaps[4 * MAX_LIGHTMAPS * BLOCK_WIDTH * BLOCK_HEIGHT];
extern qboolean lightmap_modified[MAX_LIGHTMAPS];

// Rectangle tracking for partial updates
typedef struct {
    int     l, t, w, h;     // Left, top, width, height
} glRect_t;

extern glRect_t lightmap_rectchange[MAX_LIGHTMAPS];

// Function prototypes
void R_BuildLightMap(msurface_t *surf, byte *dest, int stride);
void R_AddDynamicLights(msurface_t *surf);
void R_RenderDynamicLightmaps(msurface_t *surf);
void R_MarkLights(dlight_t *light, int bit, struct mnode_s *node);
void R_PushDlights(void);
void R_AnimateLight(void);
int R_LightPoint(vec3_t p);

// Surface flags
#define SURF_PLANEBACK      0x02
#define SURF_DRAWSKY        0x04
#define SURF_DRAWTURB       0x10
#define SURF_DRAWTILED      0x20
#define SURF_UNDERWATER     0x80

#endif // QUAKEDEF_H
