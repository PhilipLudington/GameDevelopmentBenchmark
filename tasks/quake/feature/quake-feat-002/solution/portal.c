/*
 * portal.c - Portal Culling System (COMPLETE SOLUTION)
 *
 * This file contains the complete, working implementation of
 * the portal-based visibility culling system.
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
 */
void Frustum_FromCamera(camera_t *camera, frustum_t *frustum)
{
    Frustum_Init(frustum);

    float halfFovH = camera->fov * 0.5f;
    float halfFovV = atanf(tanf(halfFovH) / camera->aspect);

    /* Near plane - points forward */
    {
        mplane_t plane;
        vec3_t nearPoint;
        VectorCopy(camera->origin, nearPoint);
        VectorScale(camera->forward, camera->nearDist, nearPoint);
        VectorAdd(camera->origin, nearPoint, nearPoint);
        Plane_FromPointNormal(nearPoint, camera->forward, &plane);
        Frustum_AddPlane(frustum, &plane);
    }

    /* Far plane - points backward */
    {
        mplane_t plane;
        vec3_t farPoint, backNormal;
        VectorScale(camera->forward, camera->farDist, farPoint);
        VectorAdd(camera->origin, farPoint, farPoint);
        VectorNegate(camera->forward, backNormal);
        Plane_FromPointNormal(farPoint, backNormal, &plane);
        Frustum_AddPlane(frustum, &plane);
    }

    /* Left plane */
    {
        mplane_t plane;
        vec3_t normal;
        float c = cosf(halfFovH);
        float s = sinf(halfFovH);
        /* Rotate forward by +halfFov around up axis */
        normal[0] = camera->forward[0] * c + camera->right[0] * s;
        normal[1] = camera->forward[1] * c + camera->right[1] * s;
        normal[2] = camera->forward[2] * c + camera->right[2] * s;
        Vec3_Normalize(normal);
        Plane_FromPointNormal(camera->origin, normal, &plane);
        Frustum_AddPlane(frustum, &plane);
    }

    /* Right plane */
    {
        mplane_t plane;
        vec3_t normal;
        float c = cosf(halfFovH);
        float s = sinf(halfFovH);
        /* Rotate forward by -halfFov around up axis */
        normal[0] = camera->forward[0] * c - camera->right[0] * s;
        normal[1] = camera->forward[1] * c - camera->right[1] * s;
        normal[2] = camera->forward[2] * c - camera->right[2] * s;
        Vec3_Normalize(normal);
        Plane_FromPointNormal(camera->origin, normal, &plane);
        Frustum_AddPlane(frustum, &plane);
    }

    /* Top plane */
    {
        mplane_t plane;
        vec3_t normal;
        float c = cosf(halfFovV);
        float s = sinf(halfFovV);
        /* Rotate forward by -halfFovV around right axis */
        normal[0] = camera->forward[0] * c - camera->up[0] * s;
        normal[1] = camera->forward[1] * c - camera->up[1] * s;
        normal[2] = camera->forward[2] * c - camera->up[2] * s;
        Vec3_Normalize(normal);
        Plane_FromPointNormal(camera->origin, normal, &plane);
        Frustum_AddPlane(frustum, &plane);
    }

    /* Bottom plane */
    {
        mplane_t plane;
        vec3_t normal;
        float c = cosf(halfFovV);
        float s = sinf(halfFovV);
        /* Rotate forward by +halfFovV around right axis */
        normal[0] = camera->forward[0] * c + camera->up[0] * s;
        normal[1] = camera->forward[1] * c + camera->up[1] * s;
        normal[2] = camera->forward[2] * c + camera->up[2] * s;
        Vec3_Normalize(normal);
        Plane_FromPointNormal(camera->origin, normal, &plane);
        Frustum_AddPlane(frustum, &plane);
    }
}

/*
 * Frustum_CullPoint - Test if a point is inside the frustum
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
 * Uses the "p-vertex" and "n-vertex" optimization:
 * For each plane, find the vertex most likely to be outside (n-vertex)
 * and the vertex most likely to be inside (p-vertex).
 */
int Frustum_CullBox(frustum_t *frustum, vec3_t mins, vec3_t maxs)
{
    int result = FRUSTUM_INSIDE;

    for (int i = 0; i < frustum->numPlanes; i++) {
        mplane_t *plane = &frustum->planes[i];

        /* Find p-vertex (positive vertex) - furthest along plane normal */
        vec3_t pVertex;
        pVertex[0] = (plane->normal[0] >= 0) ? maxs[0] : mins[0];
        pVertex[1] = (plane->normal[1] >= 0) ? maxs[1] : mins[1];
        pVertex[2] = (plane->normal[2] >= 0) ? maxs[2] : mins[2];

        /* Find n-vertex (negative vertex) - opposite corner */
        vec3_t nVertex;
        nVertex[0] = (plane->normal[0] >= 0) ? mins[0] : maxs[0];
        nVertex[1] = (plane->normal[1] >= 0) ? mins[1] : maxs[1];
        nVertex[2] = (plane->normal[2] >= 0) ? mins[2] : maxs[2];

        /* If n-vertex is outside, the whole box is outside */
        if (Plane_Distance(plane, pVertex) < 0)
            return FRUSTUM_OUTSIDE;

        /* If p-vertex is outside, box intersects this plane */
        if (Plane_Distance(plane, nVertex) < 0)
            result = FRUSTUM_INTERSECT;
    }

    return result;
}

/*
 * Frustum_CullSphere - Test if a sphere is inside the frustum
 */
int Frustum_CullSphere(frustum_t *frustum, vec3_t center, float radius)
{
    int result = FRUSTUM_INSIDE;

    for (int i = 0; i < frustum->numPlanes; i++) {
        float dist = Plane_Distance(&frustum->planes[i], center);

        if (dist < -radius)
            return FRUSTUM_OUTSIDE;

        if (dist < radius)
            result = FRUSTUM_INTERSECT;
    }

    return result;
}

/*
 * Frustum_CullPolygon - Test if a convex polygon is inside the frustum
 */
int Frustum_CullPolygon(frustum_t *frustum, vec3_t *verts, int numVerts)
{
    int allInside = 1;

    for (int p = 0; p < frustum->numPlanes; p++) {
        int inside = 0;
        int outside = 0;

        for (int v = 0; v < numVerts; v++) {
            float dist = Plane_Distance(&frustum->planes[p], verts[v]);
            if (dist >= 0)
                inside++;
            else
                outside++;
        }

        /* All vertices outside this plane = polygon is outside */
        if (outside == numVerts)
            return FRUSTUM_OUTSIDE;

        /* Some vertices outside = not completely inside */
        if (inside != numVerts)
            allInside = 0;
    }

    return allInside ? FRUSTUM_INSIDE : FRUSTUM_INTERSECT;
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
 */
qboolean Portal_IsVisible(portal_t *portal, frustum_t *frustum, vec3_t viewOrigin)
{
    /* Portal must be open */
    if (!portal->open)
        return false;

    /* Quick sphere cull using bounding radius */
    if (Frustum_CullSphere(frustum, portal->center, portal->radius) == FRUSTUM_OUTSIDE)
        return false;

    /* Check if viewer is on front side of portal (can see through it)
     * or very close to it (within epsilon) */
    float viewDist = Plane_Distance(&portal->plane, viewOrigin);
    if (viewDist < -PLANE_EPSILON)
        return false;  /* Viewer is behind the portal */

    /* Polygon-frustum test */
    int cullResult = Frustum_CullPolygon(frustum, portal->vertices, portal->numVertices);
    if (cullResult == FRUSTUM_OUTSIDE)
        return false;

    return true;
}

/*
 * Portal_ClipFrustum - Clip frustum through a portal
 *
 * Creates a new frustum by adding planes from the viewpoint through
 * each edge of the portal polygon.
 */
void Portal_ClipFrustum(portal_t *portal, vec3_t viewOrigin, frustum_t *in, frustum_t *out)
{
    Frustum_Init(out);

    /* Copy existing frustum planes */
    for (int i = 0; i < in->numPlanes; i++) {
        Frustum_AddPlane(out, &in->planes[i]);
    }

    /* For each edge of the portal, create a clipping plane from the
     * view origin through that edge. The plane normal should point
     * INWARD (into the visible region through the portal). */
    for (int i = 0; i < portal->numVertices; i++) {
        int j = (i + 1) % portal->numVertices;

        vec3_t *v0 = &portal->vertices[i];
        vec3_t *v1 = &portal->vertices[j];

        /* Create plane from viewOrigin through edge v0->v1 */
        vec3_t toV0, toV1, normal;
        VectorSubtract(*v0, viewOrigin, toV0);
        VectorSubtract(*v1, viewOrigin, toV1);

        /* Cross product: toV1 x toV0 gives normal pointing into the portal
         * (into the visible region through the portal) */
        CrossProduct(toV1, toV0, normal);

        float len = Vec3_Length(normal);
        if (len < 0.0001f)
            continue;  /* Degenerate edge */

        Vec3_Normalize(normal);

        mplane_t clipPlane;
        Plane_FromPointNormal(viewOrigin, normal, &clipPlane);
        Frustum_AddPlane(out, &clipPlane);
    }
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
 */
void Portal_RenderArea(world_t *world, int areaIndex, frustum_t *frustum,
                       vec3_t viewOrigin, int depth)
{
    /* Check depth limit to prevent infinite recursion */
    if (depth > MAX_PORTAL_DEPTH)
        return;

    if (areaIndex < 0 || areaIndex >= world->numAreas)
        return;

    area_t *area = &world->areas[areaIndex];

    /* Check if already visited this frame */
    if (area->visframe == world->visframecount)
        return;

    /* Mark area as visited */
    area->visframe = world->visframecount;

    /* Mark leaves in this area as visible if they're in the frustum */
    for (int i = 0; i < world->numLeaves; i++) {
        mleaf_t *leaf = &world->leaves[i];
        if (leaf->area == areaIndex) {
            int cullResult = Frustum_CullBox(frustum, leaf->mins, leaf->maxs);
            if (cullResult != FRUSTUM_OUTSIDE) {
                leaf->visframe = world->visframecount;
            }
        }
    }

    /* Check each portal leading out of this area */
    for (int p = 0; p < area->numPortals; p++) {
        int portalIndex = area->portalIndices[p];
        portal_t *portal = &world->portals[portalIndex];

        /* Skip closed portals */
        if (!portal->open)
            continue;

        /* Check if portal is visible in current frustum */
        if (!Portal_IsVisible(portal, frustum, viewOrigin))
            continue;

        /* Determine which area is on the other side */
        int otherArea = (portal->areaA == areaIndex) ? portal->areaB : portal->areaA;

        /* Skip if other area was already visited */
        if (otherArea >= 0 && otherArea < world->numAreas &&
            world->areas[otherArea].visframe == world->visframecount)
            continue;

        /* Clip frustum through portal */
        frustum_t clippedFrustum;
        Portal_ClipFrustum(portal, viewOrigin, frustum, &clippedFrustum);

        /* Recursively render the area on the other side */
        Portal_RenderArea(world, otherArea, &clippedFrustum, viewOrigin, depth + 1);
    }
}

/*
 * World_MarkVisibleLeaves - Mark all leaves visible from camera position
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
