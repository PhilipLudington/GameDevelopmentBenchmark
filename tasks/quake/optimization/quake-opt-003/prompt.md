# Optimize Lightmap Interpolation for Dynamic Lights

## Problem Description

Quake's dynamic lighting (muzzle flashes, rocket explosions, torches) causes significant frame rate drops in complex areas. The current implementation recalculates the entire lightmap for affected surfaces every frame, even when only small portions are affected by dynamic lights.

Performance impact:
- 1 dynamic light: 2ms overhead (acceptable)
- 4 dynamic lights: 15ms overhead (noticeable)
- 8+ dynamic lights: 40ms+ overhead (severe stuttering)

## Background: Quake's Lighting System

Quake uses two lighting systems:

1. **Static lightmaps**: Pre-computed during map compilation, stored as textures
2. **Dynamic lights**: Runtime lights that modify lightmaps temporarily

```c
// Current dynamic light update (simplified)
void R_AddDynamicLights(msurface_t *surf)
{
    int         lnum;
    dlight_t    *dl;
    float       dist;

    for (lnum = 0; lnum < MAX_DLIGHTS; lnum++) {
        dl = &cl_dlights[lnum];
        if (dl->die < cl.time || !dl->radius)
            continue;

        // Calculate distance to surface
        dist = DotProduct(dl->origin, surf->plane->normal) - surf->plane->dist;

        if (dist > dl->radius)
            continue;

        // Recalculate ENTIRE lightmap for this surface
        R_BuildLightMap(surf);  // <-- Expensive!
    }
}
```

The problem: `R_BuildLightMap()` recalculates every texel in the lightmap, even if only a small corner is affected by the dynamic light.

## The Optimization

Implement incremental lightmap updates:

1. **Dirty rectangle tracking**: Track which regions of each lightmap are affected by dynamic lights
2. **Partial updates**: Only recalculate texels within the dirty region
3. **Light contribution caching**: Cache the base (static) lightmap separately from dynamic contributions
4. **Additive blending**: Add dynamic light contribution to cached static lightmap

```c
// Proposed structure
typedef struct {
    int x, y, width, height;  // Dirty region in lightmap coordinates
    qboolean dirty;
} lightmap_dirty_rect_t;

typedef struct {
    byte *static_lightmap;    // Cached static lighting
    byte *combined_lightmap;  // Static + dynamic
    lightmap_dirty_rect_t dirty;
} lightmap_cache_t;
```

## Files

The lighting code is in:
- `game/r_light.c` - Software renderer lighting
- `game/gl_rlight.c` - OpenGL renderer lighting

Modify:
- `R_BuildLightMap()` - Add dirty region support
- `R_AddDynamicLights()` - Track affected regions
- Add caching infrastructure

## Performance Target

After optimization:
- 8 dynamic lights: < 8ms overhead
- 16 dynamic lights: < 15ms overhead
- Graceful degradation with more lights

## Implementation Requirements

1. **Dirty region calculation**: Determine which lightmap texels are affected by each light based on:
   - Light position and radius
   - Surface geometry
   - Current lightmap resolution

2. **Region merging**: Multiple lights may affect overlapping regions. Merge dirty rects efficiently.

3. **Static lightmap caching**: Store the original (static) lightmap so dynamic contributions can be re-added each frame without accumulation.

4. **Texture upload optimization**: Only upload changed regions to GPU (OpenGL path).

## Challenges

1. **Lightmap packing**: Quake packs multiple surface lightmaps into texture atlases. Dirty regions must account for this.

2. **Surface visibility**: Don't update lightmaps for surfaces not in the current PVS.

3. **Light movement**: When a light moves, both the old and new affected regions need updating.

4. **Precision**: Dynamic light contribution uses different precision than static. Match the visual output.

## Testing

```bash
# Compile with optimization
make test_light_perf CFLAGS="-O2"

# Run lighting performance tests
./test_light_perf
```

Tests include:
- Visual regression: Compare screenshots with reference
- Performance: Measure frame time with varying light counts
- Correctness: Verify dirty region calculations
- Edge cases: Lights at surface boundaries, very large lights

## Hints

1. Start with dirty region tracking without actual optimization - verify it identifies the right texels
2. The lightmap coordinate transform is in `R_BuildLightMap()` - study it carefully
3. For the OpenGL path, look at `glTexSubImage2D()` for partial updates
4. Consider a threshold: if dirty region > 50% of surface, just recalculate everything
5. Profile texture upload vs calculation time - sometimes calculation is fast but upload is slow
