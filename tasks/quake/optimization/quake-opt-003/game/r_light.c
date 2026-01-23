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
// r_light.c -- Dynamic lighting (UNOPTIMIZED VERSION)
//
// PERFORMANCE ISSUE: This implementation recalculates the ENTIRE lightmap
// for any surface affected by a dynamic light, even if only a small portion
// is actually illuminated. This causes severe performance degradation with
// multiple dynamic lights.
//
// Original: https://github.com/id-Software/Quake/blob/master/WinQuake/gl_rsurf.c

#include "quakedef.h"

// Global state
client_state_t cl;
unsigned int blocklights[MAX_LIGHTMAP_SIZE * MAX_LIGHTMAP_SIZE * 3];
byte lightmaps[4 * MAX_LIGHTMAPS * BLOCK_WIDTH * BLOCK_HEIGHT];
qboolean lightmap_modified[MAX_LIGHTMAPS];
glRect_t lightmap_rectchange[MAX_LIGHTMAPS];

/*
===============
R_AddDynamicLights

Add the contribution of all dynamic lights to the blocklights array.
This processes EVERY texel in the lightmap, even if the light only
affects a small portion.

PERFORMANCE ISSUES:
1. Iterates through all MAX_DLIGHTS even if most are inactive
2. Calculates contribution for EVERY texel in the lightmap
3. No early-out for texels clearly outside the light's radius
4. No spatial acceleration - pure brute force O(lights * texels)
===============
*/
void R_AddDynamicLights(msurface_t *surf)
{
    int         lnum;
    int         sd, td;
    float       dist, rad, minlight;
    vec3_t      impact, local;
    int         s, t;
    int         i;
    int         smax, tmax;
    mtexinfo_t  *tex;
    dlight_t    *dl;
    unsigned    *bl;

    smax = surf->smax;
    tmax = surf->tmax;
    tex = surf->texinfo;

    // PERFORMANCE ISSUE: Check ALL dynamic lights, even inactive ones
    for (lnum = 0; lnum < MAX_DLIGHTS; lnum++)
    {
        // Skip inactive lights
        if (!(surf->dlightbits & (1 << lnum)))
            continue;

        dl = &cl.dlights[lnum];
        rad = dl->radius;

        // Calculate distance from light to surface plane
        dist = DotProduct(dl->origin, surf->plane->normal) - surf->plane->dist;
        rad -= fabsf(dist);

        // Light doesn't reach surface
        if (rad < 0)
            continue;

        minlight = dl->minlight;

        // Calculate impact point on surface
        for (i = 0; i < 3; i++)
        {
            impact[i] = dl->origin[i] - surf->plane->normal[i] * dist;
        }

        // Convert to texture coordinates
        local[0] = DotProduct(impact, tex->vecs[0]) + tex->vecs[0][3];
        local[1] = DotProduct(impact, tex->vecs[1]) + tex->vecs[1][3];

        local[0] -= surf->texturemins[0];
        local[1] -= surf->texturemins[1];

        // PERFORMANCE ISSUE: Process EVERY texel in the lightmap
        // even though many are outside the light's radius
        bl = blocklights;
        for (t = 0; t < tmax; t++)
        {
            // Distance from texel center to light impact in T direction
            td = (int)local[1] - (t * 16 + 8);
            if (td < 0)
                td = -td;

            for (s = 0; s < smax; s++, bl += 3)
            {
                // Distance from texel center to light impact in S direction
                sd = (int)local[0] - (s * 16 + 8);
                if (sd < 0)
                    sd = -sd;

                // PERFORMANCE ISSUE: Slow distance approximation
                // Using max(sd,td) + min(sd,td)/2 instead of sqrt
                if (sd > td)
                    dist = sd + (td >> 1);
                else
                    dist = td + (sd >> 1);

                // Add light contribution if within radius
                // PERFORMANCE ISSUE: Still calculates even when dist > rad
                if (dist < rad)
                {
                    float brightness = (rad - dist) * 256.0f;
                    bl[0] += (unsigned int)(brightness * dl->color[0]);
                    bl[1] += (unsigned int)(brightness * dl->color[1]);
                    bl[2] += (unsigned int)(brightness * dl->color[2]);
                }
            }
        }
    }
}

/*
===============
R_BuildLightMap

Combine base (static) lightmaps with dynamic lighting into final output.
Called EVERY FRAME for any surface with dynamic lights affecting it.

PERFORMANCE ISSUES:
1. Recalculates the ENTIRE lightmap even if only 1 texel changed
2. No caching of static lightmap contributions
3. Copies all styles even if they haven't changed
4. Must re-upload entire lightmap region to GPU
===============
*/
void R_BuildLightMap(msurface_t *surf, byte *dest, int stride)
{
    int         smax, tmax;
    int         t;
    int         i, j, size;
    byte        *lightmap;
    unsigned    scale;
    int         maps;
    unsigned    *bl;

    // PERFORMANCE ISSUE: Always recalculate everything
    surf->cached_dlight = (surf->dlightframe == cl.framecount);

    smax = surf->smax;
    tmax = surf->tmax;
    size = smax * tmax;
    lightmap = surf->samples;

    // Start with no light (or fullbright if no samples)
    if (!lightmap)
    {
        // No lightmap data - set to fullbright
        for (i = 0; i < size * 3; i++)
            blocklights[i] = 255 * 256;
    }
    else
    {
        // Clear to zero
        memset(blocklights, 0, size * 3 * sizeof(unsigned int));

        // PERFORMANCE ISSUE: Process ALL light styles every time
        // Even styles that haven't animated since last frame
        for (maps = 0; maps < 4 && surf->styles[maps] != 255; maps++)
        {
            // Get the scale for this lightstyle
            scale = (unsigned int)(cl.lightstylevalue[surf->styles[maps]] * 256.0f);

            // Cache the light value for later comparison
            surf->cached_light[maps] = scale;

            lightmap = surf->samples + maps * size * 3;
            bl = blocklights;

            // PERFORMANCE ISSUE: No SIMD, pure scalar loop
            for (i = 0; i < size; i++)
            {
                bl[0] += lightmap[0] * scale;
                bl[1] += lightmap[1] * scale;
                bl[2] += lightmap[2] * scale;
                lightmap += 3;
                bl += 3;
            }
        }
    }

    // Add dynamic lights
    // PERFORMANCE ISSUE: This recalculates ALL texels
    if (surf->dlightframe == cl.framecount)
    {
        R_AddDynamicLights(surf);
    }

    // Convert from 8.8 fixed point to bytes
    // PERFORMANCE ISSUE: Convert ALL texels even if only some changed
    bl = blocklights;
    for (i = 0; i < tmax; i++, dest += stride)
    {
        for (j = 0; j < smax; j++)
        {
            // Shift down and clamp to 255
            unsigned int r = bl[0] >> 8;
            unsigned int g = bl[1] >> 8;
            unsigned int b = bl[2] >> 8;

            if (r > 255) r = 255;
            if (g > 255) g = 255;
            if (b > 255) b = 255;

            // RGBA output
            dest[j * 4 + 0] = (byte)r;
            dest[j * 4 + 1] = (byte)g;
            dest[j * 4 + 2] = (byte)b;
            dest[j * 4 + 3] = 255;

            bl += 3;
        }
    }
}

/*
===============
R_RenderDynamicLightmaps

Check if a surface needs its lightmap rebuilt due to:
1. Dynamic lights affecting it
2. Light style animations changing

Marks the entire lightmap region as needing GPU upload.

PERFORMANCE ISSUES:
1. Marks entire lightmap region dirty, not just changed texels
2. Always rebuilds entire lightmap for any change
3. No dirty rectangle optimization
===============
*/
void R_RenderDynamicLightmaps(msurface_t *surf)
{
    byte        *base;
    int         maps;
    qboolean    needsUpdate = false;
    glRect_t    *theRect;
    int         smax, tmax;

    // Skip special surfaces
    if (surf->flags & (SURF_DRAWSKY | SURF_DRAWTURB))
        return;

    smax = surf->smax;
    tmax = surf->tmax;

    // Check if dynamic lights changed
    if (surf->dlightframe == cl.framecount)
    {
        needsUpdate = true;
    }
    // Check if light styles changed
    else
    {
        for (maps = 0; maps < 4 && surf->styles[maps] != 255; maps++)
        {
            unsigned int current = (unsigned int)(cl.lightstylevalue[surf->styles[maps]] * 256.0f);
            if (current != surf->cached_light[maps])
            {
                needsUpdate = true;
                break;
            }
        }
    }

    if (!needsUpdate)
        return;

    // Mark the lightmap as modified
    lightmap_modified[surf->lightmaptexturenum] = true;

    // PERFORMANCE ISSUE: Always mark the ENTIRE surface as dirty
    // A smarter approach would track only the affected region
    theRect = &lightmap_rectchange[surf->lightmaptexturenum];

    // Expand the dirty rect to include this surface
    // PERFORMANCE ISSUE: No rect minimization, just union
    if (surf->light_t < theRect->t)
    {
        if (theRect->h)
            theRect->h += theRect->t - surf->light_t;
        theRect->t = surf->light_t;
    }
    if (surf->light_s < theRect->l)
    {
        if (theRect->w)
            theRect->w += theRect->l - surf->light_s;
        theRect->l = surf->light_s;
    }

    int newWidth = (surf->light_s - theRect->l) + smax;
    int newHeight = (surf->light_t - theRect->t) + tmax;

    if (newWidth > theRect->w)
        theRect->w = newWidth;
    if (newHeight > theRect->h)
        theRect->h = newHeight;

    // Rebuild the ENTIRE lightmap for this surface
    // PERFORMANCE ISSUE: This is the key problem - full recalculation
    base = lightmaps + surf->lightmaptexturenum * 4 * BLOCK_WIDTH * BLOCK_HEIGHT;
    base += (surf->light_t * BLOCK_WIDTH + surf->light_s) * 4;

    R_BuildLightMap(surf, base, BLOCK_WIDTH * 4);
}

/*
===============
R_AnimateLight

Update light style values based on time.
Called once per frame.
===============
*/
void R_AnimateLight(void)
{
    int         i, j;
    float       time = cl.time * 10.0f;

    for (i = 0; i < MAX_LIGHTSTYLES; i++)
    {
        if (!cl.lightstyles[i].length)
        {
            cl.lightstylevalue[i] = 1.0f;
            continue;
        }

        j = (int)time % cl.lightstyles[i].length;
        // 'a' = 0, 'm' = 12 (normal), 'z' = 25 (double bright)
        cl.lightstylevalue[i] = (float)(cl.lightstyles[i].map[j] - 'a') / 12.0f;
    }
}

/*
=============
R_PushDlights

Mark surfaces affected by dynamic lights.
===============
*/
void R_PushDlights(void)
{
    // This would traverse the BSP and mark surfaces
    // For the test harness, we handle this in the test setup
}

/*
=============
R_LightPoint

Get the light level at a point.
===============
*/
int R_LightPoint(vec3_t p)
{
    // Simplified for test harness
    return 128;
}
