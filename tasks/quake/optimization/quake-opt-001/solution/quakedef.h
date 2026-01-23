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
// quakedef.h -- Primary Quake header

#ifndef QUAKEDEF_H
#define QUAKEDEF_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

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

// BSP content types
#define CONTENTS_EMPTY      -1
#define CONTENTS_SOLID      -2
#define CONTENTS_WATER      -3
#define CONTENTS_SLIME      -4
#define CONTENTS_LAVA       -5
#define CONTENTS_SKY        -6

// Plane types
#define PLANE_X             0
#define PLANE_Y             1
#define PLANE_Z             2
#define PLANE_ANYX          3
#define PLANE_ANYY          4
#define PLANE_ANYZ          5

// BSP plane
typedef struct mplane_s {
    vec3_t  normal;
    float   dist;
    byte    type;       // for fast side tests
    byte    signbits;   // signx + (signy<<1) + (signz<<2)
    byte    pad[2];
} mplane_t;

// BSP node
typedef struct mnode_s {
    // common with leaf
    int         contents;       // 0, to differentiate from leafs
    int         visframe;       // node needs to be traversed if current

    float       minmaxs[6];     // for bounding box culling

    struct mnode_s  *parent;

    // node specific
    mplane_t    *plane;
    struct mnode_s  *children[2];

    unsigned short  firstsurface;
    unsigned short  numsurfaces;
} mnode_t;

// BSP leaf
typedef struct mleaf_s {
    // common with node
    int         contents;       // will be a negative contents number
    int         visframe;       // node needs to be traversed if current

    float       minmaxs[6];     // for bounding box culling

    struct mnode_s  *parent;

    // leaf specific
    byte        *compressed_vis;
    struct efrag_s  *efrags;

    struct msurface_s   **firstmarksurface;
    int         nummarksurfaces;
    int         key;            // BSP sequence number for leaf's contents
    byte        ambient_sound_level[4];  // NUM_AMBIENTS
} mleaf_t;

// Model structure (partial - just what we need for PVS)
typedef struct model_s {
    char        name[64];

    qboolean    needload;

    int         numleafs;       // number of visible leafs, not counting 0
    mleaf_t     *leafs;

    int         numnodes;
    mnode_t     *nodes;

    int         numplanes;
    mplane_t    *planes;

    // Additional fields omitted for brevity...
} model_t;

// Function prototypes for model.c
byte *Mod_DecompressVis(byte *in, model_t *model);
byte *Mod_LeafPVS(mleaf_t *leaf, model_t *model);
byte *Mod_NoVisPVS(model_t *model);
void Mod_InitPVS(void);
void Mod_ClearPVSCache(void);  // Optimized version only
mleaf_t *Mod_PointInLeaf(vec3_t p, model_t *model);
qboolean Mod_CheckVis(mleaf_t *leaf1, int leaf2, model_t *model);

#endif // QUAKEDEF_H
