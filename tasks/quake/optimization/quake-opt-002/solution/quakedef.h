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
// quakedef.h -- Primary Quake header for world.c

#ifndef QUAKEDEF_H
#define QUAKEDEF_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
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
#define VectorAdd(a, b, c) ((c)[0] = (a)[0] + (b)[0], (c)[1] = (a)[1] + (b)[1], (c)[2] = (a)[2] + (b)[2])
#define VectorSubtract(a, b, c) ((c)[0] = (a)[0] - (b)[0], (c)[1] = (a)[1] - (b)[1], (c)[2] = (a)[2] - (b)[2])
#define VectorMA(v, s, b, d) ((d)[0] = (v)[0] + (s) * (b)[0], (d)[1] = (v)[1] + (s) * (b)[1], (d)[2] = (v)[2] + (s) * (b)[2])

// Solid types
#define SOLID_NOT       0   // No interaction with other objects
#define SOLID_TRIGGER   1   // Touch on edge, but not blocking
#define SOLID_BBOX      2   // Touch on edge, block
#define SOLID_SLIDEBOX  3   // Touch on edge, but not an onground
#define SOLID_BSP       4   // BSP clip, touch on edge, block

// Area types for queries
#define AREA_SOLID      1
#define AREA_TRIGGERS   2

// Forward declaration for link
struct link_s;

/*
 * Entity structure (simplified for collision testing)
 */
typedef struct edict_s {
    // Link for area node lists
    struct {
        struct link_s   *prev, *next;
    } area;

    // State
    qboolean    free;           // If true, this edict is available for reuse
    qboolean    linked;         // If true, currently in world

    // Physics properties
    int         solid;          // Solid type (SOLID_*)
    vec3_t      origin;         // World position
    vec3_t      angles;         // Orientation (for BSP)
    vec3_t      mins, maxs;     // Bounding box relative to origin
    vec3_t      absmin, absmax; // Absolute bounding box in world
    vec3_t      velocity;       // Movement velocity

    // For testing
    int         id;             // Entity ID for verification
} edict_t;

// Function prototypes
void SV_ClearWorld(vec3_t mins, vec3_t maxs);
void SV_UnlinkEdict(edict_t *ent);
void SV_LinkEdict(edict_t *ent);
int SV_AreaEdicts(vec3_t mins, vec3_t maxs, edict_t **list, int maxcount, int type);
qboolean SV_TestEntityPosition(edict_t *ent);
void SV_TouchLinks(edict_t *ent);
edict_t *SV_CreateEdict(void);
void SV_FreeEdict(edict_t *ent);
void SV_RunPhysics(float frametime);
int SV_GetEdictCount(void);

// Optimized version extras
void SV_GetHashStats(int *total_cells, int *used_cells, int *max_per_cell, int *overflow_count);

#endif // QUAKEDEF_H
