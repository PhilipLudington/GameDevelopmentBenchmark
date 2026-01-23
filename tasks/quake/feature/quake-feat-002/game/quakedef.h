/*
 * quakedef.h - Quake type definitions for portal culling system
 *
 * Simplified types extracted from Quake source for benchmark purposes.
 */

#ifndef QUAKEDEF_H
#define QUAKEDEF_H

#include <stdint.h>
#include <string.h>
#include <math.h>

/* Basic types */
typedef unsigned char byte;
typedef float vec_t;
typedef vec_t vec3_t[3];
typedef int qboolean;

#define true 1
#define false 0

/* Math macros */
#define DotProduct(a, b) ((a)[0] * (b)[0] + (a)[1] * (b)[1] + (a)[2] * (b)[2])
#define VectorCopy(a, b) ((b)[0] = (a)[0], (b)[1] = (a)[1], (b)[2] = (a)[2])
#define VectorAdd(a, b, c) ((c)[0] = (a)[0] + (b)[0], (c)[1] = (a)[1] + (b)[1], (c)[2] = (a)[2] + (b)[2])
#define VectorSubtract(a, b, c) ((c)[0] = (a)[0] - (b)[0], (c)[1] = (a)[1] - (b)[1], (c)[2] = (a)[2] - (b)[2])
#define VectorScale(a, s, b) ((b)[0] = (a)[0] * (s), (b)[1] = (a)[1] * (s), (b)[2] = (a)[2] * (s))
#define VectorClear(a) ((a)[0] = (a)[1] = (a)[2] = 0)
#define VectorLength(a) (sqrtf(DotProduct((a), (a))))
#define VectorNegate(a, b) ((b)[0] = -(a)[0], (b)[1] = -(a)[1], (b)[2] = -(a)[2])

/* Cross product */
#define CrossProduct(a, b, c) \
    ((c)[0] = (a)[1] * (b)[2] - (a)[2] * (b)[1], \
     (c)[1] = (a)[2] * (b)[0] - (a)[0] * (b)[2], \
     (c)[2] = (a)[0] * (b)[1] - (a)[1] * (b)[0])

/* Constants */
#define MAX_PORTAL_VERTICES  8      /* Max vertices per portal polygon */
#define MAX_FRUSTUM_PLANES   32     /* Max planes in a frustum */
#define MAX_PORTAL_DEPTH     8      /* Max recursive portal depth */
#define MAX_PORTALS          256    /* Max portals in world */
#define MAX_AREAS            64     /* Max areas in world */
#define MAX_LEAVES_PER_AREA  256    /* Max BSP leaves per area */

#define PLANE_EPSILON        0.001f /* Epsilon for plane tests */

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* Frustum cull results */
#define FRUSTUM_OUTSIDE     0
#define FRUSTUM_INSIDE      1
#define FRUSTUM_INTERSECT   2

/* Plane side classification */
#define SIDE_FRONT          0
#define SIDE_BACK           1
#define SIDE_ON             2

/* ============== PLANE STRUCTURE ============== */

/*
 * mplane_t - A plane in 3D space
 *
 * Plane equation: normal.x * x + normal.y * y + normal.z * z = dist
 */
typedef struct mplane_s {
    vec3_t      normal;
    float       dist;
    int         type;       /* 0-2 = axial, 3+ = non-axial */
    int         signbits;   /* For fast box culling */
} mplane_t;

/* ============== FRUSTUM STRUCTURE ============== */

/*
 * frustum_t - A view frustum defined by clipping planes
 *
 * Planes point INWARD (positive side is inside the frustum)
 */
typedef struct {
    mplane_t    planes[MAX_FRUSTUM_PLANES];
    int         numPlanes;
} frustum_t;

/* ============== PORTAL STRUCTURE ============== */

/*
 * portal_t - A portal between two areas
 *
 * Portal is a convex polygon that acts as a window between areas.
 * When the portal is "open", visibility can pass through.
 */
typedef struct portal_s {
    int         areaA;          /* Area on front side of portal */
    int         areaB;          /* Area on back side of portal */
    vec3_t      vertices[MAX_PORTAL_VERTICES];
    int         numVertices;
    mplane_t    plane;          /* Portal plane (normal points to areaA) */
    qboolean    open;           /* Can see through this portal? */
    vec3_t      center;         /* Center point of portal */
    float       radius;         /* Bounding radius for quick culling */
} portal_t;

/* ============== AREA STRUCTURE ============== */

/*
 * area_t - A region of the world bounded by portals
 */
typedef struct area_s {
    int         portalIndices[MAX_PORTALS];  /* Indices of portals connected to this area */
    int         numPortals;
    vec3_t      mins, maxs;     /* Bounding box of the area */
    int         visframe;       /* For avoiding duplicate traversal */
} area_t;

/* ============== BSP LEAF (simplified) ============== */

/*
 * mleaf_t - A BSP leaf containing geometry
 */
typedef struct mleaf_s {
    vec3_t      mins, maxs;     /* Bounding box */
    int         area;           /* Which area this leaf belongs to */
    int         visframe;       /* Frame number when last marked visible */
    int         contents;       /* Leaf contents (solid, water, etc.) */
} mleaf_t;

/* ============== WORLD STRUCTURE ============== */

/*
 * world_t - The complete portal/area system
 */
typedef struct {
    portal_t    portals[MAX_PORTALS];
    int         numPortals;
    area_t      areas[MAX_AREAS];
    int         numAreas;
    mleaf_t     *leaves;
    int         numLeaves;
    int         visframecount;  /* Current visibility frame */
} world_t;

/* ============== CAMERA/VIEW STRUCTURE ============== */

typedef struct {
    vec3_t      origin;         /* Camera position */
    vec3_t      forward;        /* View direction */
    vec3_t      right;          /* Right vector */
    vec3_t      up;             /* Up vector */
    float       fov;            /* Field of view in radians */
    float       aspect;         /* Aspect ratio (width/height) */
    float       nearDist;       /* Near clip plane distance */
    float       farDist;        /* Far clip plane distance */
} camera_t;

/* ============== FUNCTION DECLARATIONS ============== */

/* Plane operations */
void Plane_FromPointNormal(vec3_t point, vec3_t normal, mplane_t *out);
void Plane_FromPoints(vec3_t a, vec3_t b, vec3_t c, mplane_t *out);
float Plane_Distance(mplane_t *plane, vec3_t point);
int Plane_PointSide(mplane_t *plane, vec3_t point);
void Plane_Normalize(mplane_t *plane);

/* Vector operations */
void Vec3_Normalize(vec3_t v);
float Vec3_Length(vec3_t v);

/* Frustum operations */
void Frustum_Init(frustum_t *frustum);
void Frustum_FromCamera(camera_t *camera, frustum_t *frustum);
void Frustum_AddPlane(frustum_t *frustum, mplane_t *plane);
int Frustum_CullPoint(frustum_t *frustum, vec3_t point);
int Frustum_CullBox(frustum_t *frustum, vec3_t mins, vec3_t maxs);
int Frustum_CullSphere(frustum_t *frustum, vec3_t center, float radius);
int Frustum_CullPolygon(frustum_t *frustum, vec3_t *verts, int numVerts);

/* Portal operations */
void Portal_Init(portal_t *portal);
void Portal_SetVertices(portal_t *portal, vec3_t *verts, int numVerts);
void Portal_CalculatePlane(portal_t *portal);
void Portal_CalculateBounds(portal_t *portal);
qboolean Portal_IsVisible(portal_t *portal, frustum_t *frustum, vec3_t viewOrigin);
void Portal_ClipFrustum(portal_t *portal, vec3_t viewOrigin, frustum_t *in, frustum_t *out);

/* Area operations */
void Area_Init(area_t *area);
void Area_AddPortal(area_t *area, int portalIndex);

/* World operations */
void World_Init(world_t *world);
int World_AddPortal(world_t *world, int areaA, int areaB, vec3_t *verts, int numVerts);
int World_AddArea(world_t *world);
int World_FindAreaForPoint(world_t *world, vec3_t point);
void World_MarkVisibleLeaves(world_t *world, camera_t *camera);

/* Recursive portal rendering */
void Portal_RenderArea(world_t *world, int areaIndex, frustum_t *frustum,
                       vec3_t viewOrigin, int depth);

#endif /* QUAKEDEF_H */
