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
// world.c -- world collision detection
// UNOPTIMIZED VERSION - Uses linked list traversal O(n²)

#include "quakedef.h"

/*
 * Quake's Entity Collision System
 * ================================
 *
 * This module handles collision detection between entities (monsters,
 * projectiles, items, players). The original implementation uses a
 * hierarchical BSP-like area node structure with linked lists of entities.
 *
 * PERFORMANCE PROBLEM:
 * When finding entities in a region, we must traverse the entire linked
 * list of solid/trigger edicts at each area node. With many entities,
 * this becomes O(n²) as each entity's movement checks against all others.
 *
 * Entity counts and typical performance:
 *   64 entities:  ~16ms physics update
 *   128 entities: ~65ms physics update
 *   256 entities: ~260ms physics update (unplayable)
 */

// ============================================================================
// DATA STRUCTURES
// ============================================================================

#define MAX_EDICTS          1024
#define AREA_DEPTH          4
#define AREA_NODES          32

/*
 * Doubly-linked list node for entity chains
 */
typedef struct link_s {
    struct link_s   *prev, *next;
} link_t;

/*
 * Area node - BSP-like subdivision of the world
 * Entities are stored in linked lists at leaf nodes
 */
typedef struct areanode_s {
    int             axis;           // -1 = leaf node, 0-2 = split axis
    float           dist;           // Split plane distance
    struct areanode_s   *children[2];
    link_t          trigger_edicts; // Linked list of trigger entities
    link_t          solid_edicts;   // Linked list of solid entities
} areanode_t;

// Area node tree
static areanode_t   sv_areanodes[AREA_NODES];
static int          sv_numareanodes;

// Global entity list
edict_t             sv_edicts[MAX_EDICTS];
int                 sv_num_edicts;

// World bounds
static vec3_t       world_mins, world_maxs;


// ============================================================================
// LINKED LIST HELPERS
// ============================================================================

static void ClearLink(link_t *l)
{
    l->prev = l->next = l;
}

static void RemoveLink(link_t *l)
{
    l->next->prev = l->prev;
    l->prev->next = l->next;
}

static void InsertLinkBefore(link_t *l, link_t *before)
{
    l->next = before;
    l->prev = before->prev;
    l->prev->next = l;
    l->next->prev = l;
}

#define EDICT_FROM_AREA(l) ((edict_t *)((byte *)l - offsetof(edict_t, area)))


// ============================================================================
// AREA NODE TREE
// ============================================================================

/*
===============
SV_CreateAreaNode

Recursively build area node tree
===============
*/
static areanode_t *SV_CreateAreaNode(int depth, vec3_t mins, vec3_t maxs)
{
    areanode_t  *anode;
    vec3_t      size;
    vec3_t      mins1, maxs1, mins2, maxs2;

    anode = &sv_areanodes[sv_numareanodes];
    sv_numareanodes++;

    ClearLink(&anode->trigger_edicts);
    ClearLink(&anode->solid_edicts);

    if (depth == AREA_DEPTH) {
        anode->axis = -1;
        anode->children[0] = anode->children[1] = NULL;
        return anode;
    }

    VectorSubtract(maxs, mins, size);
    if (size[0] > size[1])
        anode->axis = 0;
    else
        anode->axis = 1;

    anode->dist = 0.5f * (maxs[anode->axis] + mins[anode->axis]);
    VectorCopy(mins, mins1);
    VectorCopy(mins, mins2);
    VectorCopy(maxs, maxs1);
    VectorCopy(maxs, maxs2);

    maxs1[anode->axis] = mins2[anode->axis] = anode->dist;

    anode->children[0] = SV_CreateAreaNode(depth + 1, mins2, maxs2);
    anode->children[1] = SV_CreateAreaNode(depth + 1, mins1, maxs1);

    return anode;
}


/*
===============
SV_ClearWorld

Initialize the world collision system
===============
*/
void SV_ClearWorld(vec3_t mins, vec3_t maxs)
{
    VectorCopy(mins, world_mins);
    VectorCopy(maxs, world_maxs);

    sv_numareanodes = 0;
    SV_CreateAreaNode(0, mins, maxs);

    sv_num_edicts = 0;
    memset(sv_edicts, 0, sizeof(sv_edicts));
}


// ============================================================================
// ENTITY LINKING
// ============================================================================

/*
===============
SV_UnlinkEdict

Remove an entity from the world collision structure
===============
*/
void SV_UnlinkEdict(edict_t *ent)
{
    if (!ent->area.prev)
        return;     // Not linked in

    RemoveLink(&ent->area);
    ent->area.prev = ent->area.next = NULL;
}


/*
===============
SV_FindTouchingAreaNodes

Find area nodes that touch the given bounds
UNOPTIMIZED: Recursively searches entire tree
===============
*/
static void SV_FindTouchingAreaNodes(areanode_t *node, edict_t *ent)
{
    link_t *list;

    // Add to this node's list
    if (ent->solid == SOLID_TRIGGER)
        list = &node->trigger_edicts;
    else
        list = &node->solid_edicts;

    InsertLinkBefore(&ent->area, list);
}


/*
===============
SV_LinkEdict

Add an entity to the world collision structure.
Finds the appropriate area node and inserts into its linked list.

PERFORMANCE ISSUE: Simple insertion, but queries are slow
===============
*/
void SV_LinkEdict(edict_t *ent)
{
    areanode_t  *node;

    if (ent->area.prev)
        SV_UnlinkEdict(ent);    // Unlink from old position

    if (ent->free)
        return;

    // Calculate absolute bounds
    VectorAdd(ent->origin, ent->mins, ent->absmin);
    VectorAdd(ent->origin, ent->maxs, ent->absmax);

    // Expand for movement
    if (ent->solid == SOLID_BSP &&
        (ent->angles[0] || ent->angles[1] || ent->angles[2])) {
        // Rotated BSP needs expanded bounds
        float max = 0;
        for (int i = 0; i < 3; i++) {
            float v = fabsf(ent->mins[i]);
            if (v > max) max = v;
            v = fabsf(ent->maxs[i]);
            if (v > max) max = v;
        }
        for (int i = 0; i < 3; i++) {
            ent->absmin[i] = ent->origin[i] - max;
            ent->absmax[i] = ent->origin[i] + max;
        }
    }

    // Find which area node to insert into
    node = sv_areanodes;
    while (1) {
        if (node->axis == -1)
            break;
        if (ent->absmin[node->axis] > node->dist)
            node = node->children[0];
        else if (ent->absmax[node->axis] < node->dist)
            node = node->children[1];
        else
            break;  // Crosses the split plane
    }

    SV_FindTouchingAreaNodes(node, ent);
    ent->linked = true;
}


// ============================================================================
// COLLISION QUERIES
// ============================================================================

// For SV_AreaEdicts
static edict_t  *area_list[MAX_EDICTS];
static int      area_count;
static int      area_maxcount;
static vec3_t   area_mins, area_maxs;
static int      area_type;

/*
===============
SV_AreaEdicts_r

UNOPTIMIZED: Recursively traverse and check EVERY entity in the tree
This is the performance bottleneck - O(n) per query, O(n²) total
===============
*/
static void SV_AreaEdicts_r(areanode_t *node)
{
    link_t      *l, *next, *start;
    edict_t     *check;

    // Check entities at this node
    if (area_type == AREA_SOLID)
        start = &node->solid_edicts;
    else
        start = &node->trigger_edicts;

    // PERFORMANCE ISSUE: Must iterate through entire linked list
    for (l = start->next; l != start; l = next) {
        next = l->next;
        check = EDICT_FROM_AREA(l);

        if (check->solid == SOLID_NOT)
            continue;

        // Check if this entity overlaps the query region
        // PERFORMANCE ISSUE: Checking every entity, even distant ones
        if (check->absmin[0] > area_maxs[0] ||
            check->absmin[1] > area_maxs[1] ||
            check->absmin[2] > area_maxs[2] ||
            check->absmax[0] < area_mins[0] ||
            check->absmax[1] < area_mins[1] ||
            check->absmax[2] < area_mins[2])
            continue;   // Not touching

        if (area_count >= area_maxcount) {
            return;     // List full
        }

        area_list[area_count] = check;
        area_count++;
    }

    // Recurse into child nodes if needed
    if (node->axis == -1)
        return;     // Leaf node

    if (area_maxs[node->axis] > node->dist)
        SV_AreaEdicts_r(node->children[0]);
    if (area_mins[node->axis] < node->dist)
        SV_AreaEdicts_r(node->children[1]);
}


/*
===============
SV_AreaEdicts

Find all entities that touch the given region.
Returns count and fills list array.

PERFORMANCE ISSUE: This is O(n) where n is total entities,
because we must traverse all linked lists in touched area nodes.
When called for each moving entity, total becomes O(n²).
===============
*/
int SV_AreaEdicts(vec3_t mins, vec3_t maxs, edict_t **list, int maxcount, int type)
{
    VectorCopy(mins, area_mins);
    VectorCopy(maxs, area_maxs);
    area_list[0] = NULL;  // Use the static array
    area_count = 0;
    area_maxcount = maxcount;
    area_type = type;

    SV_AreaEdicts_r(sv_areanodes);

    // Copy to output
    for (int i = 0; i < area_count && i < maxcount; i++) {
        list[i] = area_list[i];
    }

    return area_count;
}


// ============================================================================
// COLLISION TESTING
// ============================================================================

/*
===============
SV_TestEntityPosition

Check if an entity's position is valid (not stuck in world or other entities)
===============
*/
qboolean SV_TestEntityPosition(edict_t *ent)
{
    edict_t *touched[MAX_EDICTS];
    int num_touched;
    int i;

    // Get entities that might touch us
    num_touched = SV_AreaEdicts(ent->absmin, ent->absmax, touched, MAX_EDICTS, AREA_SOLID);

    // Check each one
    for (i = 0; i < num_touched; i++) {
        if (touched[i] == ent)
            continue;   // Don't collide with self

        if (touched[i]->solid == SOLID_NOT)
            continue;

        // Simple AABB overlap test (already passed in SV_AreaEdicts)
        // In real Quake, this would do a more precise hull trace
        return true;    // Collision found
    }

    return false;   // Position is valid
}


/*
===============
SV_TouchLinks

Call touch functions for all triggers an entity is touching
===============
*/
void SV_TouchLinks(edict_t *ent)
{
    edict_t *touched[MAX_EDICTS];
    int num_touched;
    int i;

    num_touched = SV_AreaEdicts(ent->absmin, ent->absmax, touched, MAX_EDICTS, AREA_TRIGGERS);

    for (i = 0; i < num_touched; i++) {
        if (touched[i] == ent)
            continue;

        // Would call touch function here
        // touched[i]->touch(touched[i], ent);
    }
}


// ============================================================================
// ENTITY MANAGEMENT
// ============================================================================

/*
===============
SV_CreateEdict

Allocate a new entity
===============
*/
edict_t *SV_CreateEdict(void)
{
    edict_t *ent;

    if (sv_num_edicts >= MAX_EDICTS)
        return NULL;

    ent = &sv_edicts[sv_num_edicts];
    sv_num_edicts++;

    memset(ent, 0, sizeof(*ent));
    ent->free = false;
    ent->solid = SOLID_BBOX;

    return ent;
}


/*
===============
SV_FreeEdict

Mark an entity as free
===============
*/
void SV_FreeEdict(edict_t *ent)
{
    SV_UnlinkEdict(ent);
    ent->free = true;
}


/*
===============
SV_RunPhysics

Update all entity physics - this is where O(n²) happens
===============
*/
void SV_RunPhysics(float frametime)
{
    int i;
    edict_t *ent;

    // PERFORMANCE ISSUE: For each entity...
    for (i = 0; i < sv_num_edicts; i++) {
        ent = &sv_edicts[i];
        if (ent->free)
            continue;

        // Move the entity
        VectorMA(ent->origin, frametime, ent->velocity, ent->origin);

        // Re-link in world (updates area node membership)
        SV_LinkEdict(ent);

        // Check for collisions
        // PERFORMANCE ISSUE: This calls SV_AreaEdicts which is O(n)
        // So total is O(n) entities * O(n) query = O(n²)
        if (SV_TestEntityPosition(ent)) {
            // Handle collision - back out movement
            VectorMA(ent->origin, -frametime, ent->velocity, ent->origin);
            VectorClear(ent->velocity);
            SV_LinkEdict(ent);
        }

        // Check triggers
        SV_TouchLinks(ent);
    }
}


/*
===============
SV_GetEdictCount

Return current entity count (for benchmarking)
===============
*/
int SV_GetEdictCount(void)
{
    return sv_num_edicts;
}
