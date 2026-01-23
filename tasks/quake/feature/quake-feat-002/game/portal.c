/*
 * portal.c - Portal Culling System (INCOMPLETE)
 *
 * This file contains stub implementations that need to be completed.
 * The portal culling system should:
 * 1. Define and manage portal geometry
 * 2. Test portal visibility against a view frustum
 * 3. Clip the frustum through a portal for recursive rendering
 * 4. Recursively traverse areas through visible portals
 *
 * TASK: Implement the TODO sections in each function.
 */

#include "quakedef.h"
#include <stdio.h>
#include <stdlib.h>

/* ============== VECTOR OPERATIONS ============== */

/*
 * Vec3_Normalize - Normalize a vector to unit length
 */
void Vec3_Normalize(vec3_t v)
{
    float len = sqrtf(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    if (len > 0.0001f) {
        float invLen = 1.0f / len;
        v[0] *= invLen;
        v[1] *= invLen;
        v[2] *= invLen;
    }
}

/*
 * Vec3_Length - Get length of vector
 */
float Vec3_Length(vec3_t v)
{
    return sqrtf(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

/* ============== PLANE OPERATIONS ============== */

/*
 * Plane_FromPointNormal - Create plane from point and normal
 */
void Plane_FromPointNormal(vec3_t point, vec3_t normal, mplane_t *out)
{
    VectorCopy(normal, out->normal);
    Vec3_Normalize(out->normal);
    out->dist = DotProduct(out->normal, point);
    out->type = 3;  /* Non-axial */
    out->signbits = 0;
}

/*
 * Plane_FromPoints - Create plane from three points
 *
 * Points should be in counter-clockwise order when viewed
 * from the front (positive side) of the plane.
 */
void Plane_FromPoints(vec3_t a, vec3_t b, vec3_t c, mplane_t *out)
{
    vec3_t ab, ac;
    VectorSubtract(b, a, ab);
    VectorSubtract(c, a, ac);
    CrossProduct(ab, ac, out->normal);
    Vec3_Normalize(out->normal);
    out->dist = DotProduct(out->normal, a);
    out->type = 3;
    out->signbits = 0;
}

/*
 * Plane_Distance - Calculate signed distance from point to plane
 *
 * Positive = front side (same side as normal)
 * Negative = back side
 * Zero = on the plane
 */
float Plane_Distance(mplane_t *plane, vec3_t point)
{
    return DotProduct(point, plane->normal) - plane->dist;
}

/*
 * Plane_PointSide - Classify which side of plane a point is on
 */
int Plane_PointSide(mplane_t *plane, vec3_t point)
{
    float dist = Plane_Distance(plane, point);
    if (dist > PLANE_EPSILON)
        return SIDE_FRONT;
    if (dist < -PLANE_EPSILON)
        return SIDE_BACK;
    return SIDE_ON;
}

/*
 * Plane_Normalize - Ensure plane normal is unit length
 */
void Plane_Normalize(mplane_t *plane)
{
    float len = Vec3_Length(plane->normal);
    if (len > 0.0001f) {
        float invLen = 1.0f / len;
        plane->normal[0] *= invLen;
        plane->normal[1] *= invLen;
        plane->normal[2] *= invLen;
        plane->dist *= invLen;
    }
}

/* ============== FRUSTUM OPERATIONS ============== */

/*
 * Frustum_Init - Initialize an empty frustum
 */
void Frustum_Init(frustum_t *frustum)
{
    frustum->numPlanes = 0;
    memset(frustum->planes, 0, sizeof(frustum->planes));
}

/*
 * Frustum_AddPlane - Add a clipping plane to the frustum
 */
void Frustum_AddPlane(frustum_t *frustum, mplane_t *plane)
{
    if (frustum->numPlanes >= MAX_FRUSTUM_PLANES)
        return;

    frustum->planes[frustum->numPlanes] = *plane;
    frustum->numPlanes++;
}

/*
 * Frustum_FromCamera - Build view frustum from camera parameters
 *
 * Creates 6 planes: near, far, left, right, top, bottom
 * All planes point inward (positive side is inside frustum)
 *
 * TODO: Implement frustum construction from camera parameters
 */
void Frustum_FromCamera(camera_t *camera, frustum_t *frustum)
{
    /* TODO: Implement frustum construction */
    /* STUB: Creates empty frustum - incorrect! */
    Frustum_Init(frustum);

    (void)camera;  /* Unused in stub */
}

/*
 * Frustum_CullPoint - Test if a point is inside the frustum
 *
 * Returns: FRUSTUM_INSIDE if point is inside all planes
 *          FRUSTUM_OUTSIDE if point is outside any plane
 */
int Frustum_CullPoint(frustum_t *frustum, vec3_t point)
{
    for (int i = 0; i < frustum->numPlanes; i++) {
        if (Plane_Distance(&frustum->planes[i], point) < 0)
            return FRUSTUM_OUTSIDE;
    }
    return FRUSTUM_INSIDE;
}

/*
 * Frustum_CullBox - Test if an axis-aligned box is inside the frustum
 *
 * TODO: Implement box-frustum culling
 *
 * Returns: FRUSTUM_OUTSIDE if box is completely outside
 *          FRUSTUM_INSIDE if box is completely inside
 *          FRUSTUM_INTERSECT if box crosses frustum boundary
 */
int Frustum_CullBox(frustum_t *frustum, vec3_t mins, vec3_t maxs)
{
    /* TODO: Implement box-frustum culling */
    /* STUB: Returns INSIDE for everything - incorrect! */
    (void)frustum;
    (void)mins;
    (void)maxs;
    return FRUSTUM_INSIDE;
}

/*
 * Frustum_CullSphere - Test if a sphere is inside the frustum
 *
 * TODO: Implement sphere-frustum culling
 */
int Frustum_CullSphere(frustum_t *frustum, vec3_t center, float radius)
{
    /* TODO: Implement sphere-frustum culling */
    /* STUB: Returns INSIDE for everything - incorrect! */
    (void)frustum;
    (void)center;
    (void)radius;
    return FRUSTUM_INSIDE;
}

/*
 * Frustum_CullPolygon - Test if a convex polygon is inside the frustum
 *
 * TODO: Implement polygon-frustum culling
 *
 * Returns: FRUSTUM_OUTSIDE if all vertices outside any plane
 *          FRUSTUM_INSIDE if all vertices inside all planes
 *          FRUSTUM_INTERSECT if polygon crosses frustum boundary
 */
int Frustum_CullPolygon(frustum_t *frustum, vec3_t *verts, int numVerts)
{
    /* TODO: Implement polygon-frustum culling */
    /* STUB: Returns INSIDE for everything - incorrect! */
    (void)frustum;
    (void)verts;
    (void)numVerts;
    return FRUSTUM_INSIDE;
}

/* ============== PORTAL OPERATIONS ============== */

/*
 * Portal_Init - Initialize a portal
 */
void Portal_Init(portal_t *portal)
{
    memset(portal, 0, sizeof(*portal));
    portal->open = true;
    portal->areaA = -1;
    portal->areaB = -1;
}

/*
 * Portal_SetVertices - Set portal polygon vertices
 */
void Portal_SetVertices(portal_t *portal, vec3_t *verts, int numVerts)
{
    if (numVerts > MAX_PORTAL_VERTICES)
        numVerts = MAX_PORTAL_VERTICES;

    portal->numVertices = numVerts;
    for (int i = 0; i < numVerts; i++) {
        VectorCopy(verts[i], portal->vertices[i]);
    }

    Portal_CalculatePlane(portal);
    Portal_CalculateBounds(portal);
}

/*
 * Portal_CalculatePlane - Calculate the portal's plane from its vertices
 */
void Portal_CalculatePlane(portal_t *portal)
{
    if (portal->numVertices < 3)
        return;

    Plane_FromPoints(portal->vertices[0],
                     portal->vertices[1],
                     portal->vertices[2],
                     &portal->plane);
}

/*
 * Portal_CalculateBounds - Calculate center and bounding radius
 */
void Portal_CalculateBounds(portal_t *portal)
{
    if (portal->numVertices == 0)
        return;

    /* Calculate center */
    VectorClear(portal->center);
    for (int i = 0; i < portal->numVertices; i++) {
        VectorAdd(portal->center, portal->vertices[i], portal->center);
    }
    VectorScale(portal->center, 1.0f / portal->numVertices, portal->center);

    /* Calculate bounding radius */
    portal->radius = 0;
    for (int i = 0; i < portal->numVertices; i++) {
        vec3_t diff;
        VectorSubtract(portal->vertices[i], portal->center, diff);
        float dist = Vec3_Length(diff);
        if (dist > portal->radius)
            portal->radius = dist;
    }
}

/*
 * Portal_IsVisible - Check if portal is potentially visible from viewpoint
 *
 * A portal is visible if:
 * 1. The viewer is on the front side of the portal plane (or close to it)
 * 2. The portal polygon intersects or is inside the view frustum
 * 3. The portal is marked as open
 *
 * TODO: Implement portal visibility test
 */
qboolean Portal_IsVisible(portal_t *portal, frustum_t *frustum, vec3_t viewOrigin)
{
    /* TODO: Implement portal visibility test */
    /* STUB: Returns true for all open portals - no culling! */

    if (!portal->open)
        return false;

    (void)frustum;
    (void)viewOrigin;
    return true;
}

/*
 * Portal_ClipFrustum - Clip frustum through a portal
 *
 * Creates a new frustum that represents the view through the portal.
 * The new frustum is the intersection of the original frustum with
 * a pyramid from the viewpoint through the portal edges.
 *
 * TODO: Implement frustum clipping through portal
 *
 * For each edge of the portal:
 *   Create a plane from viewOrigin through the edge
 *   Add to output frustum
 * Also keep original frustum planes that don't face the portal
 */
void Portal_ClipFrustum(portal_t *portal, vec3_t viewOrigin, frustum_t *in, frustum_t *out)
{
    /* TODO: Implement frustum clipping through portal */
    /* STUB: Just copies input frustum - no clipping! */

    *out = *in;

    (void)portal;
    (void)viewOrigin;
}

/* ============== AREA OPERATIONS ============== */

/*
 * Area_Init - Initialize an area
 */
void Area_Init(area_t *area)
{
    memset(area, 0, sizeof(*area));
    area->mins[0] = area->mins[1] = area->mins[2] = 999999;
    area->maxs[0] = area->maxs[1] = area->maxs[2] = -999999;
}

/*
 * Area_AddPortal - Add a portal index to the area
 */
void Area_AddPortal(area_t *area, int portalIndex)
{
    if (area->numPortals >= MAX_PORTALS)
        return;

    area->portalIndices[area->numPortals] = portalIndex;
    area->numPortals++;
}

/* ============== WORLD OPERATIONS ============== */

/*
 * World_Init - Initialize the world portal system
 */
void World_Init(world_t *world)
{
    memset(world, 0, sizeof(*world));
    world->visframecount = 0;
}

/*
 * World_AddArea - Add a new area to the world
 */
int World_AddArea(world_t *world)
{
    if (world->numAreas >= MAX_AREAS)
        return -1;

    int index = world->numAreas;
    Area_Init(&world->areas[index]);
    world->numAreas++;
    return index;
}

/*
 * World_AddPortal - Add a portal connecting two areas
 */
int World_AddPortal(world_t *world, int areaA, int areaB, vec3_t *verts, int numVerts)
{
    if (world->numPortals >= MAX_PORTALS)
        return -1;

    int index = world->numPortals;
    portal_t *portal = &world->portals[index];

    Portal_Init(portal);
    portal->areaA = areaA;
    portal->areaB = areaB;
    Portal_SetVertices(portal, verts, numVerts);

    /* Add portal to both areas */
    if (areaA >= 0 && areaA < world->numAreas)
        Area_AddPortal(&world->areas[areaA], index);
    if (areaB >= 0 && areaB < world->numAreas)
        Area_AddPortal(&world->areas[areaB], index);

    world->numPortals++;
    return index;
}

/*
 * World_FindAreaForPoint - Find which area contains a point
 *
 * Simple implementation using area bounding boxes.
 */
int World_FindAreaForPoint(world_t *world, vec3_t point)
{
    for (int i = 0; i < world->numAreas; i++) {
        area_t *area = &world->areas[i];
        if (point[0] >= area->mins[0] && point[0] <= area->maxs[0] &&
            point[1] >= area->mins[1] && point[1] <= area->maxs[1] &&
            point[2] >= area->mins[2] && point[2] <= area->maxs[2]) {
            return i;
        }
    }
    return -1;
}

/* ============== RECURSIVE PORTAL RENDERING ============== */

/*
 * Portal_RenderArea - Recursively render an area and visible areas through portals
 *
 * TODO: Implement recursive portal rendering:
 * 1. Check if this area was already visited this frame
 * 2. Mark area as visited
 * 3. Mark all leaves in this area as visible (if in frustum)
 * 4. For each portal in this area:
 *    a. Skip if portal is closed
 *    b. Check if portal is visible in current frustum
 *    c. Determine which area is on the other side
 *    d. Clip frustum through portal
 *    e. Recursively render other area with clipped frustum
 */
void Portal_RenderArea(world_t *world, int areaIndex, frustum_t *frustum,
                       vec3_t viewOrigin, int depth)
{
    /* Check depth limit */
    if (depth > MAX_PORTAL_DEPTH)
        return;

    if (areaIndex < 0 || areaIndex >= world->numAreas)
        return;

    /* TODO: Implement recursive portal rendering */
    /* STUB: Just marks area as visited, no portal traversal */

    area_t *area = &world->areas[areaIndex];

    /* Mark area as visited */
    area->visframe = world->visframecount;

    /* Mark leaves in this area as visible */
    for (int i = 0; i < world->numLeaves; i++) {
        mleaf_t *leaf = &world->leaves[i];
        if (leaf->area == areaIndex) {
            leaf->visframe = world->visframecount;
        }
    }

    /* TODO: Traverse portals and recurse into visible areas */
    (void)frustum;
    (void)viewOrigin;
    (void)depth;
}

/*
 * World_MarkVisibleLeaves - Mark all leaves visible from camera position
 *
 * Entry point for portal culling. Builds view frustum and starts
 * recursive portal traversal from the camera's area.
 */
void World_MarkVisibleLeaves(world_t *world, camera_t *camera)
{
    /* Increment frame counter */
    world->visframecount++;

    /* Build view frustum */
    frustum_t frustum;
    Frustum_FromCamera(camera, &frustum);

    /* Find starting area */
    int startArea = World_FindAreaForPoint(world, camera->origin);
    if (startArea < 0)
        return;

    /* Start recursive portal rendering */
    Portal_RenderArea(world, startArea, &frustum, camera->origin, 0);
}
