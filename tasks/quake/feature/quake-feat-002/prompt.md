# Implement Portal-Based Occlusion Culling

## Overview

Quake's PVS (Potentially Visible Set) is pre-computed and static. This means:
- Doors can't block visibility (everything behind a closed door is still processed)
- Moving geometry can't affect visibility
- Large outdoor areas have poor culling

Implement a portal system that enables runtime visibility determination. Portals are planar windows between areas that can be opened/closed and used to restrict what's visible.

## Background: Portals in Game Engines

Portal culling works by:
1. Defining portal polygons at doorways/windows between areas
2. When rendering, testing if the portal is visible
3. If visible, clipping the view frustum to the portal
4. Recursively rendering the area beyond with the clipped frustum

```
     [Camera]
         \
          \  View Frustum
           \_______________
           |               |
    Area A | Portal ====== | Area B
           |               |
           |_______________|
```

If the portal is closed (door shut), Area B is not rendered at all.

## Implementation Requirements

### 1. Portal Definition

```c
typedef struct portal_s {
    int         areaA;          // Area on one side
    int         areaB;          // Area on other side
    vec3_t      vertices[4];    // Portal polygon (convex, max 4 verts)
    int         numVertices;
    mplane_t    plane;          // Portal plane
    qboolean    open;           // Can see through?
    struct portal_s *next;      // Next portal in area
} portal_t;

typedef struct area_s {
    portal_t    *portals;       // Portals leading out of this area
    int         numPortals;
    mleaf_t     **leaves;       // BSP leaves in this area
    int         numLeaves;
} area_t;
```

### 2. Portal-Frustum Intersection

```c
// Check if portal is visible in current frustum
qboolean Portal_IsVisible(portal_t *portal, frustum_t *frustum)
{
    // Portal is visible if any of its vertices are inside the frustum
    // OR if any frustum edge intersects the portal polygon
}

// Clip frustum to portal, creating new frustum for recursive culling
void Portal_ClipFrustum(portal_t *portal, frustum_t *in, frustum_t *out)
{
    // The new frustum is the intersection of the view frustum
    // with the pyramid from the camera through the portal
}
```

### 3. Recursive Rendering

```c
void R_RenderAreaThroughPortal(area_t *area, portal_t *portal, frustum_t *frustum, int depth)
{
    if (depth > MAX_PORTAL_DEPTH)  // Prevent infinite recursion
        return;

    // Mark leaves in this area as visible (if in frustum)
    for (each leaf in area->leaves) {
        if (Frustum_CullBox(frustum, leaf->mins, leaf->maxs) == INSIDE)
            leaf->visframe = r_visframecount;
    }

    // Check portals leading out of this area
    for (each portal in area->portals) {
        if (!portal->open)
            continue;

        if (!Portal_IsVisible(portal, frustum))
            continue;

        // Clip frustum to portal
        frustum_t clippedFrustum;
        Portal_ClipFrustum(portal, frustum, &clippedFrustum);

        // Recursively render area on other side
        area_t *nextArea = (portal->areaA == area) ? portal->areaB : portal->areaA;
        R_RenderAreaThroughPortal(nextArea, portal, &clippedFrustum, depth + 1);
    }
}
```

### 4. Integration with Existing System

- Portals supplement (don't replace) the PVS
- Use PVS to quickly reject distant areas
- Use portals for fine-grained culling of connected areas
- Closed doors should mark their portals as closed

## Files to Modify

- `game/r_main.c` - Main rendering setup, call portal culling
- `game/r_bsp.c` - BSP traversal, integrate with portal visibility
- `game/world.c` - Area/portal management
- Create new: `game/portal.c` - Portal culling system

## Mathematics Reference

### Point-Plane Distance
```c
float Plane_Distance(mplane_t *plane, vec3_t point)
{
    return DotProduct(point, plane->normal) - plane->dist;
}
```

### Frustum Plane from 3 Points
```c
void Frustum_PlaneFromPoints(vec3_t a, vec3_t b, vec3_t c, mplane_t *out)
{
    vec3_t ab, ac;
    VectorSubtract(b, a, ab);
    VectorSubtract(c, a, ac);
    CrossProduct(ab, ac, out->normal);
    VectorNormalize(out->normal);
    out->dist = DotProduct(out->normal, a);
}
```

### Polygon-Frustum Intersection
```c
int Frustum_CullPolygon(frustum_t *frustum, vec3_t *verts, int numVerts)
{
    int allInside = 1;
    int allOutside = 1;

    for (int p = 0; p < frustum->numPlanes; p++) {
        int inside = 0, outside = 0;

        for (int v = 0; v < numVerts; v++) {
            float dist = Plane_Distance(&frustum->planes[p], verts[v]);
            if (dist > 0) inside++;
            else outside++;
        }

        if (outside == numVerts)
            return OUTSIDE;  // Completely outside this plane

        if (inside != numVerts)
            allInside = 0;
    }

    return allInside ? INSIDE : INTERSECT;
}
```

## Testing

```bash
# Compile and run tests
make test_portals

./test_portals
```

Tests include:
- Portal visibility tests (various camera positions)
- Frustum clipping accuracy
- Recursive depth limiting
- Closed portal handling
- Performance comparison with/without portals

## Hints

1. Start with a simple two-room test case with one portal
2. Debug by drawing portal outlines in wireframe
3. The portal polygon must face the camera to be visible
4. When clipping the frustum, handle the case where camera is very close to portal
5. Use a small epsilon for plane tests to avoid precision issues
6. Consider what happens when looking through multiple aligned portals
