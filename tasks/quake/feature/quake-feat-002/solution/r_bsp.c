/*
 * r_bsp.c - BSP rendering with portal integration (INCOMPLETE)
 *
 * This file shows how the portal culling system integrates with
 * Quake's BSP renderer. The portal functions in portal.c need
 * to be completed for proper visibility culling.
 *
 * Original Quake uses static PVS (Potentially Visible Set).
 * This version adds dynamic portal culling on top of PVS.
 */

#include "quakedef.h"
#include <stdio.h>
#include <stdlib.h>

/* Global world state */
static world_t g_world;
static int g_visframecount = 0;

/* ============== BSP LEAF RENDERING ============== */

/*
 * R_MarkLeafVisible - Mark a BSP leaf as visible for this frame
 */
static void R_MarkLeafVisible(mleaf_t *leaf)
{
    leaf->visframe = g_visframecount;
}

/*
 * R_IsLeafVisible - Check if a leaf was marked visible this frame
 */
static qboolean R_IsLeafVisible(mleaf_t *leaf)
{
    return leaf->visframe == g_visframecount;
}

/* ============== TRADITIONAL PVS RENDERING ============== */

/*
 * R_MarkLeavesPVS - Mark leaves visible using static PVS
 *
 * This is the original Quake method - fast but static.
 * Cannot handle dynamic visibility changes (doors, etc.)
 */
void R_MarkLeavesPVS(byte *vis, int numLeaves, mleaf_t *leaves)
{
    g_visframecount++;

    for (int i = 0; i < numLeaves; i++) {
        /* Check if leaf is in PVS (bit array) */
        int byteIndex = i >> 3;
        int bitIndex = i & 7;
        if (vis[byteIndex] & (1 << bitIndex)) {
            R_MarkLeafVisible(&leaves[i]);
        }
    }
}

/* ============== PORTAL-BASED RENDERING ============== */

/*
 * R_SetupPortalWorld - Initialize portal system from BSP data
 */
void R_SetupPortalWorld(void)
{
    World_Init(&g_world);
}

/*
 * R_AddPortalArea - Add an area to the portal system
 */
int R_AddPortalArea(vec3_t mins, vec3_t maxs)
{
    int index = World_AddArea(&g_world);
    if (index >= 0) {
        VectorCopy(mins, g_world.areas[index].mins);
        VectorCopy(maxs, g_world.areas[index].maxs);
    }
    return index;
}

/*
 * R_AddPortal - Add a portal between two areas
 */
int R_AddPortal(int areaA, int areaB, vec3_t *verts, int numVerts)
{
    return World_AddPortal(&g_world, areaA, areaB, verts, numVerts);
}

/*
 * R_SetPortalOpen - Open or close a portal (e.g., when a door opens/closes)
 */
void R_SetPortalOpen(int portalIndex, qboolean open)
{
    if (portalIndex >= 0 && portalIndex < g_world.numPortals) {
        g_world.portals[portalIndex].open = open;
    }
}

/*
 * R_SetLeaves - Set the leaf array for visibility marking
 */
void R_SetLeaves(mleaf_t *leaves, int numLeaves)
{
    g_world.leaves = leaves;
    g_world.numLeaves = numLeaves;
}

/*
 * R_MarkLeavesPortals - Mark leaves visible using portal culling
 *
 * This is the new method - slower but handles dynamic visibility.
 * Uses recursive portal traversal with frustum clipping.
 */
void R_MarkLeavesPortals(camera_t *camera)
{
    g_visframecount++;
    g_world.visframecount = g_visframecount;

    World_MarkVisibleLeaves(&g_world, camera);
}

/* ============== HYBRID RENDERING ============== */

/*
 * R_MarkLeavesHybrid - Use both PVS and portals for visibility
 *
 * 1. First, use PVS to quickly reject distant leaves
 * 2. Then, use portals to refine visibility in connected areas
 * 3. A leaf is visible only if it passes both tests
 */
void R_MarkLeavesHybrid(byte *pvs, camera_t *camera)
{
    g_visframecount++;
    g_world.visframecount = g_visframecount;

    /* First pass: PVS */
    R_MarkLeavesPVS(pvs, g_world.numLeaves, g_world.leaves);

    /* Second pass: Portal culling (refines PVS results) */
    World_MarkVisibleLeaves(&g_world, camera);
}

/* ============== QUERY FUNCTIONS ============== */

/*
 * R_GetVisibleLeafCount - Count how many leaves are visible
 */
int R_GetVisibleLeafCount(void)
{
    int count = 0;
    for (int i = 0; i < g_world.numLeaves; i++) {
        if (R_IsLeafVisible(&g_world.leaves[i]))
            count++;
    }
    return count;
}

/*
 * R_GetWorld - Get access to the world structure (for testing)
 */
world_t* R_GetWorld(void)
{
    return &g_world;
}

/*
 * R_GetVisframecount - Get current visibility frame count
 */
int R_GetVisframecount(void)
{
    return g_visframecount;
}
