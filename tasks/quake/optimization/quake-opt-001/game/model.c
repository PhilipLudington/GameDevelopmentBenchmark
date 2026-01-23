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
// model.c -- PVS (Potentially Visible Set) decompression
// UNOPTIMIZED VERSION - This code needs performance optimization

#include "quakedef.h"

/*
 * Quake's Potentially Visible Set (PVS) System
 * ============================================
 *
 * The PVS is a precomputed visibility structure that dramatically reduces
 * rendering workload. For each leaf in the BSP tree, it stores a bitmap
 * indicating which other leaves are potentially visible from that leaf.
 *
 * Storage Format (Run-Length Encoded):
 * ------------------------------------
 * - Non-zero byte: 8 visibility bits (raw data)
 * - Zero byte followed by count: skip 'count' bytes of zeros
 *
 * Example: 0xFF 0x00 0x03 0x01
 *   - 0xFF: First 8 leaves visible
 *   - 0x00 0x03: Next 24 leaves (3 bytes * 8) invisible
 *   - 0x01: One more leaf visible
 *
 * This achieves good compression because most leaves can only see
 * a small fraction of the total level.
 */

#define MAX_MAP_LEAFS   65536
#define MAX_PVS_CACHE   4

// Static decompression buffer
static byte mod_novis[MAX_MAP_LEAFS/8];
static byte decompressed[MAX_MAP_LEAFS/8];

/*
===============
Mod_DecompressVis

Decompress run-length encoded PVS data for a given leaf.
This is the UNOPTIMIZED reference implementation.

Performance Issues:
1. Byte-by-byte processing - no word-level operations
2. Branch-heavy main loop with unpredictable branches
3. No caching - decompresses from scratch every time
4. Inner loop for zero runs is slow for large runs

Parameters:
  in    - Pointer to compressed PVS data
  model - BSP model containing leaf count

Returns:
  Pointer to decompressed visibility bitmap
===============
*/
byte *Mod_DecompressVis(byte *in, model_t *model)
{
    int     c;
    byte    *out;
    int     row;

    row = (model->numleafs + 7) >> 3;   // Bytes needed for all leaves
    out = decompressed;

    // Return all-visible if no PVS data
    if (!in) {
        memset(decompressed, 0xFF, row);
        return decompressed;
    }

    // Main decompression loop
    // PERFORMANCE ISSUE: This loop has unpredictable branches
    do {
        if (*in) {
            // Non-zero byte: copy raw visibility data
            // PERFORMANCE ISSUE: Byte-by-byte copy
            *out++ = *in++;
            continue;
        }

        // Zero byte: run-length encoded sequence of invisible leaves
        // Format: 0x00 followed by count of zero bytes
        c = in[1];
        in += 2;

        // PERFORMANCE ISSUE: Loop for each zero byte
        // Large runs (common in outdoor areas) are slow
        while (c) {
            *out++ = 0;
            c--;
        }
    } while (out - decompressed < row);

    return decompressed;
}


/*
===============
Mod_LeafPVS

Get the PVS for a given leaf.
Called by the renderer when entering a new leaf.

PERFORMANCE ISSUE: No caching - calls Mod_DecompressVis every time
even if we recently decompressed the same leaf's PVS.

Parameters:
  leaf  - The leaf to get PVS for
  model - BSP model

Returns:
  Pointer to decompressed PVS bitmap
===============
*/
byte *Mod_LeafPVS(mleaf_t *leaf, model_t *model)
{
    // Leaf 0 is the solid leaf - everything is visible from solid
    if (leaf == model->leafs)
        return mod_novis;

    // Decompress the PVS for this leaf
    // PERFORMANCE ISSUE: No caching!
    return Mod_DecompressVis(leaf->compressed_vis, model);
}


/*
===============
Mod_NoVisPVS

Returns a PVS where everything is visible.
Used when PVS data is missing or disabled.
===============
*/
byte *Mod_NoVisPVS(model_t *model)
{
    int row = (model->numleafs + 7) >> 3;
    memset(mod_novis, 0xFF, row);
    return mod_novis;
}


/*
===============
Mod_InitPVS

Initialize PVS system. Call at startup.
===============
*/
void Mod_InitPVS(void)
{
    // Initialize no-vis buffer to all visible
    memset(mod_novis, 0xFF, sizeof(mod_novis));
}


/*
===============
Mod_PointInLeaf

Find which leaf contains the given point.
Used to determine current PVS when player moves.

Parameters:
  p     - World coordinates
  model - BSP model

Returns:
  Pointer to leaf containing point
===============
*/
mleaf_t *Mod_PointInLeaf(vec3_t p, model_t *model)
{
    mnode_t     *node;
    float       d;
    mplane_t    *plane;

    if (!model || !model->nodes)
        return NULL;

    node = model->nodes;

    // Walk down BSP tree to find containing leaf
    while (1) {
        if (node->contents < 0)
            return (mleaf_t *)node;

        plane = node->plane;

        // Determine which side of the plane the point is on
        if (plane->type < 3)
            d = p[plane->type] - plane->dist;
        else
            d = DotProduct(p, plane->normal) - plane->dist;

        if (d > 0)
            node = node->children[0];
        else
            node = node->children[1];
    }

    return NULL;    // Never reached
}


/*
===============
Mod_CheckVis

Check if two leaves can potentially see each other.

Parameters:
  leaf1 - First leaf
  leaf2 - Second leaf (leaf number, not pointer)
  model - BSP model

Returns:
  true if leaf2 is potentially visible from leaf1
===============
*/
qboolean Mod_CheckVis(mleaf_t *leaf1, int leaf2, model_t *model)
{
    byte *pvs;
    int byte_index, bit_index;

    if (!leaf1 || leaf2 < 0 || leaf2 >= model->numleafs)
        return true;    // Assume visible on error

    pvs = Mod_LeafPVS(leaf1, model);

    byte_index = leaf2 >> 3;
    bit_index = leaf2 & 7;

    return (pvs[byte_index] & (1 << bit_index)) != 0;
}
