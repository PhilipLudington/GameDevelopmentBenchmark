/*
 * Portal Culling Test Harness
 *
 * Tests all components of the portal-based visibility culling system:
 * 1. Plane operations (distance, point classification)
 * 2. Frustum construction and culling
 * 3. Portal visibility tests
 * 4. Frustum clipping through portals
 * 5. Recursive area traversal
 * 6. Open/closed portal handling
 *
 * Build:
 *   make test          - Test incomplete (game) version
 *   make test_solution - Test complete (solution) version
 *   make compare       - Compare both versions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Include the implementation under test */
#ifdef TEST_SOLUTION
#include "../solution/portal.c"
#else
#include "../game/portal.c"
#endif

/* ============== TEST UTILITIES ============== */

static int tests_passed = 0;
static int tests_failed = 0;

#define EPSILON 0.01f

static int float_eq(float a, float b)
{
    return fabsf(a - b) < EPSILON;
}

static int vec3_eq(vec3_t a, vec3_t b)
{
    return float_eq(a[0], b[0]) && float_eq(a[1], b[1]) && float_eq(a[2], b[2]);
}

#define TEST(name) \
    static void test_##name(void); \
    static void run_test_##name(void) { \
        printf("Testing: %s...\n", #name); \
        test_##name(); \
    } \
    static void test_##name(void)

#define ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            printf("  FAIL: %s\n", msg); \
            tests_failed++; \
            return; \
        } \
    } while(0)

#define PASS() \
    do { \
        printf("  PASS\n"); \
        tests_passed++; \
    } while(0)

/* ============== HELPER FUNCTIONS ============== */

static void setup_camera(camera_t *camera, vec3_t origin, vec3_t forward)
{
    VectorCopy(origin, camera->origin);
    VectorCopy(forward, camera->forward);
    Vec3_Normalize(camera->forward);

    /* Create orthonormal basis */
    vec3_t worldUp = {0, 0, 1};
    if (fabsf(DotProduct(forward, worldUp)) > 0.99f) {
        worldUp[0] = 1; worldUp[1] = 0; worldUp[2] = 0;
    }

    CrossProduct(camera->forward, worldUp, camera->right);
    Vec3_Normalize(camera->right);
    CrossProduct(camera->right, camera->forward, camera->up);
    Vec3_Normalize(camera->up);

    camera->fov = (float)M_PI / 2.0f;  /* 90 degrees */
    camera->aspect = 1.0f;
    camera->nearDist = 1.0f;
    camera->farDist = 1000.0f;
}

static void create_quad_portal(vec3_t center, vec3_t normal, float width, float height, vec3_t *verts)
{
    /* Create orthonormal basis on the plane */
    vec3_t up = {0, 0, 1};
    if (fabsf(DotProduct(normal, up)) > 0.99f) {
        up[0] = 0; up[1] = 1; up[2] = 0;
    }

    vec3_t right, realUp;
    CrossProduct(up, normal, right);  /* Changed order for correct winding */
    Vec3_Normalize(right);
    CrossProduct(normal, right, realUp);
    Vec3_Normalize(realUp);

    /* Create quad vertices (counter-clockwise when viewed from normal direction) */
    float hw = width * 0.5f;
    float hh = height * 0.5f;

    /* v0: bottom-left */
    verts[0][0] = center[0] - right[0] * hw - realUp[0] * hh;
    verts[0][1] = center[1] - right[1] * hw - realUp[1] * hh;
    verts[0][2] = center[2] - right[2] * hw - realUp[2] * hh;

    /* v1: bottom-right */
    verts[1][0] = center[0] + right[0] * hw - realUp[0] * hh;
    verts[1][1] = center[1] + right[1] * hw - realUp[1] * hh;
    verts[1][2] = center[2] + right[2] * hw - realUp[2] * hh;

    /* v2: top-right */
    verts[2][0] = center[0] + right[0] * hw + realUp[0] * hh;
    verts[2][1] = center[1] + right[1] * hw + realUp[1] * hh;
    verts[2][2] = center[2] + right[2] * hw + realUp[2] * hh;

    /* v3: top-left */
    verts[3][0] = center[0] - right[0] * hw + realUp[0] * hh;
    verts[3][1] = center[1] - right[1] * hw + realUp[1] * hh;
    verts[3][2] = center[2] - right[2] * hw + realUp[2] * hh;
}

/* ============== PLANE TESTS ============== */

TEST(plane_distance)
{
    mplane_t plane;
    vec3_t normal = {1, 0, 0};
    vec3_t point = {5, 0, 0};
    Plane_FromPointNormal(point, normal, &plane);

    vec3_t testPoint1 = {10, 0, 0};
    vec3_t testPoint2 = {0, 0, 0};
    vec3_t testPoint3 = {5, 10, 10};

    ASSERT(float_eq(Plane_Distance(&plane, testPoint1), 5.0f), "Point at 10 should be 5 units in front");
    ASSERT(float_eq(Plane_Distance(&plane, testPoint2), -5.0f), "Point at 0 should be 5 units behind");
    ASSERT(float_eq(Plane_Distance(&plane, testPoint3), 0.0f), "Point on plane should have distance 0");
    PASS();
}

TEST(plane_from_points)
{
    vec3_t a = {0, 0, 0};
    vec3_t b = {1, 0, 0};
    vec3_t c = {0, 1, 0};
    mplane_t plane;

    Plane_FromPoints(a, b, c, &plane);

    /* Normal should point in +Z direction (right-hand rule) */
    ASSERT(float_eq(plane.normal[2], 1.0f), "Normal Z should be 1");
    ASSERT(float_eq(plane.dist, 0.0f), "Plane should pass through origin");
    PASS();
}

TEST(plane_point_side)
{
    mplane_t plane;
    vec3_t normal = {0, 1, 0};
    vec3_t point = {0, 0, 0};
    Plane_FromPointNormal(point, normal, &plane);

    vec3_t front = {0, 10, 0};
    vec3_t back = {0, -10, 0};
    vec3_t on = {5, 0, 5};

    ASSERT(Plane_PointSide(&plane, front) == SIDE_FRONT, "Point should be on front side");
    ASSERT(Plane_PointSide(&plane, back) == SIDE_BACK, "Point should be on back side");
    ASSERT(Plane_PointSide(&plane, on) == SIDE_ON, "Point should be on plane");
    PASS();
}

/* ============== FRUSTUM TESTS ============== */

TEST(frustum_from_camera)
{
    camera_t camera;
    vec3_t origin = {0, 0, 0};
    vec3_t forward = {1, 0, 0};
    setup_camera(&camera, origin, forward);

    frustum_t frustum;
    Frustum_FromCamera(&camera, &frustum);

    /* Should have 6 planes (near, far, left, right, top, bottom) */
    ASSERT(frustum.numPlanes == 6, "Frustum should have 6 planes");

    /* Test a point that should be inside */
    vec3_t inside = {10, 0, 0};
    ASSERT(Frustum_CullPoint(&frustum, inside) == FRUSTUM_INSIDE,
           "Point in front of camera should be inside frustum");

    /* Test a point that should be outside (behind camera) */
    vec3_t behind = {-10, 0, 0};
    ASSERT(Frustum_CullPoint(&frustum, behind) == FRUSTUM_OUTSIDE,
           "Point behind camera should be outside frustum");

    PASS();
}

TEST(frustum_cull_box)
{
    camera_t camera;
    vec3_t origin = {0, 0, 0};
    vec3_t forward = {1, 0, 0};
    setup_camera(&camera, origin, forward);

    frustum_t frustum;
    Frustum_FromCamera(&camera, &frustum);

    /* Box fully inside frustum */
    vec3_t mins1 = {10, -1, -1};
    vec3_t maxs1 = {20, 1, 1};
    int result1 = Frustum_CullBox(&frustum, mins1, maxs1);
    ASSERT(result1 != FRUSTUM_OUTSIDE, "Box in front should not be outside");

    /* Box fully outside (behind camera) */
    vec3_t mins2 = {-20, -1, -1};
    vec3_t maxs2 = {-10, 1, 1};
    int result2 = Frustum_CullBox(&frustum, mins2, maxs2);
    ASSERT(result2 == FRUSTUM_OUTSIDE, "Box behind camera should be outside");

    /* Box fully outside (to the side) */
    vec3_t mins3 = {10, 100, -1};
    vec3_t maxs3 = {20, 110, 1};
    int result3 = Frustum_CullBox(&frustum, mins3, maxs3);
    ASSERT(result3 == FRUSTUM_OUTSIDE, "Box far to side should be outside");

    PASS();
}

TEST(frustum_cull_sphere)
{
    camera_t camera;
    vec3_t origin = {0, 0, 0};
    vec3_t forward = {1, 0, 0};
    setup_camera(&camera, origin, forward);

    frustum_t frustum;
    Frustum_FromCamera(&camera, &frustum);

    /* Sphere fully inside */
    vec3_t center1 = {20, 0, 0};
    int result1 = Frustum_CullSphere(&frustum, center1, 1.0f);
    ASSERT(result1 != FRUSTUM_OUTSIDE, "Sphere in front should not be outside");

    /* Sphere fully outside */
    vec3_t center2 = {-20, 0, 0};
    int result2 = Frustum_CullSphere(&frustum, center2, 1.0f);
    ASSERT(result2 == FRUSTUM_OUTSIDE, "Sphere behind should be outside");

    PASS();
}

TEST(frustum_cull_polygon)
{
    camera_t camera;
    vec3_t origin = {0, 0, 0};
    vec3_t forward = {1, 0, 0};
    setup_camera(&camera, origin, forward);

    frustum_t frustum;
    Frustum_FromCamera(&camera, &frustum);

    /* Polygon in front of camera */
    vec3_t poly1[4] = {
        {10, -1, -1},
        {10, 1, -1},
        {10, 1, 1},
        {10, -1, 1}
    };
    int result1 = Frustum_CullPolygon(&frustum, poly1, 4);
    ASSERT(result1 != FRUSTUM_OUTSIDE, "Polygon in front should not be outside");

    /* Polygon behind camera */
    vec3_t poly2[4] = {
        {-10, -1, -1},
        {-10, 1, -1},
        {-10, 1, 1},
        {-10, -1, 1}
    };
    int result2 = Frustum_CullPolygon(&frustum, poly2, 4);
    ASSERT(result2 == FRUSTUM_OUTSIDE, "Polygon behind should be outside");

    PASS();
}

/* ============== PORTAL TESTS ============== */

TEST(portal_init)
{
    portal_t portal;
    Portal_Init(&portal);

    ASSERT(portal.open == true, "Portal should be open by default");
    ASSERT(portal.areaA == -1, "AreaA should be -1");
    ASSERT(portal.areaB == -1, "AreaB should be -1");
    PASS();
}

TEST(portal_set_vertices)
{
    portal_t portal;
    Portal_Init(&portal);

    vec3_t center = {10, 0, 0};
    vec3_t normal = {-1, 0, 0};  /* Faces -X */
    vec3_t verts[4];
    create_quad_portal(center, normal, 4, 4, verts);

    Portal_SetVertices(&portal, verts, 4);

    ASSERT(portal.numVertices == 4, "Portal should have 4 vertices");
    ASSERT(portal.radius > 0, "Portal radius should be positive");
    PASS();
}

TEST(portal_visibility_facing)
{
    portal_t portal;
    Portal_Init(&portal);

    /* Create portal facing -X at x=10 */
    vec3_t center = {10, 0, 0};
    vec3_t normal = {-1, 0, 0};
    vec3_t verts[4];
    create_quad_portal(center, normal, 4, 4, verts);
    Portal_SetVertices(&portal, verts, 4);

    /* Camera at origin looking +X (toward portal) */
    camera_t camera;
    vec3_t camOrigin = {0, 0, 0};
    vec3_t camForward = {1, 0, 0};
    setup_camera(&camera, camOrigin, camForward);

    frustum_t frustum;
    Frustum_FromCamera(&camera, &frustum);

    /* Portal should be visible (we're on front side, looking at it) */
    ASSERT(Portal_IsVisible(&portal, &frustum, camera.origin) == true,
           "Portal facing camera should be visible");

    PASS();
}

TEST(portal_visibility_behind)
{
    portal_t portal;
    Portal_Init(&portal);

    /* Create portal facing -X at x=10 */
    vec3_t center = {10, 0, 0};
    vec3_t normal = {-1, 0, 0};
    vec3_t verts[4];
    create_quad_portal(center, normal, 4, 4, verts);
    Portal_SetVertices(&portal, verts, 4);

    /* Camera at x=20 looking -X (behind the portal) */
    camera_t camera;
    vec3_t camOrigin = {20, 0, 0};
    vec3_t camForward = {-1, 0, 0};
    setup_camera(&camera, camOrigin, camForward);

    frustum_t frustum;
    Frustum_FromCamera(&camera, &frustum);

    /* Portal should NOT be visible (we're on back side) */
    ASSERT(Portal_IsVisible(&portal, &frustum, camera.origin) == false,
           "Portal facing away should not be visible");

    PASS();
}

TEST(portal_visibility_closed)
{
    portal_t portal;
    Portal_Init(&portal);

    vec3_t center = {10, 0, 0};
    vec3_t normal = {-1, 0, 0};
    vec3_t verts[4];
    create_quad_portal(center, normal, 4, 4, verts);
    Portal_SetVertices(&portal, verts, 4);

    /* Close the portal */
    portal.open = false;

    /* Camera looking at portal */
    camera_t camera;
    vec3_t camOrigin = {0, 0, 0};
    vec3_t camForward = {1, 0, 0};
    setup_camera(&camera, camOrigin, camForward);

    frustum_t frustum;
    Frustum_FromCamera(&camera, &frustum);

    ASSERT(Portal_IsVisible(&portal, &frustum, camera.origin) == false,
           "Closed portal should not be visible");

    PASS();
}

TEST(portal_visibility_outside_frustum)
{
    portal_t portal;
    Portal_Init(&portal);

    /* Create portal at x=10, y=100 (far to the side) */
    vec3_t center = {10, 100, 0};
    vec3_t normal = {-1, 0, 0};
    vec3_t verts[4];
    create_quad_portal(center, normal, 4, 4, verts);
    Portal_SetVertices(&portal, verts, 4);

    /* Camera at origin looking +X */
    camera_t camera;
    vec3_t camOrigin = {0, 0, 0};
    vec3_t camForward = {1, 0, 0};
    setup_camera(&camera, camOrigin, camForward);

    frustum_t frustum;
    Frustum_FromCamera(&camera, &frustum);

    ASSERT(Portal_IsVisible(&portal, &frustum, camera.origin) == false,
           "Portal outside frustum should not be visible");

    PASS();
}

TEST(portal_clip_frustum)
{
    portal_t portal;
    Portal_Init(&portal);

    /* Create portal at x=10 */
    vec3_t center = {10, 0, 0};
    vec3_t normal = {-1, 0, 0};
    vec3_t verts[4];
    create_quad_portal(center, normal, 4, 4, verts);
    Portal_SetVertices(&portal, verts, 4);

    /* Camera at origin */
    camera_t camera;
    vec3_t camOrigin = {0, 0, 0};
    vec3_t camForward = {1, 0, 0};
    setup_camera(&camera, camOrigin, camForward);

    frustum_t inputFrustum, outputFrustum;
    Frustum_FromCamera(&camera, &inputFrustum);

    Portal_ClipFrustum(&portal, camera.origin, &inputFrustum, &outputFrustum);

    /* Output frustum should have more planes (original + portal edges + portal plane) */
    ASSERT(outputFrustum.numPlanes > inputFrustum.numPlanes,
           "Clipped frustum should have more planes");

    /* Point through portal should still be inside */
    vec3_t insidePortal = {15, 0, 0};
    ASSERT(Frustum_CullPoint(&outputFrustum, insidePortal) == FRUSTUM_INSIDE,
           "Point through portal center should be inside clipped frustum");

    /* Point far to the side (beyond portal) should be outside */
    vec3_t outsidePortal = {15, 50, 0};
    ASSERT(Frustum_CullPoint(&outputFrustum, outsidePortal) == FRUSTUM_OUTSIDE,
           "Point beyond portal edge should be outside clipped frustum");

    PASS();
}

/* ============== WORLD TESTS ============== */

TEST(world_init)
{
    world_t world;
    World_Init(&world);

    ASSERT(world.numAreas == 0, "World should have 0 areas");
    ASSERT(world.numPortals == 0, "World should have 0 portals");
    PASS();
}

TEST(world_add_area)
{
    world_t world;
    World_Init(&world);

    int area0 = World_AddArea(&world);
    int area1 = World_AddArea(&world);

    ASSERT(area0 == 0, "First area should be index 0");
    ASSERT(area1 == 1, "Second area should be index 1");
    ASSERT(world.numAreas == 2, "World should have 2 areas");
    PASS();
}

TEST(world_add_portal)
{
    world_t world;
    World_Init(&world);

    int area0 = World_AddArea(&world);
    int area1 = World_AddArea(&world);

    vec3_t center = {50, 0, 0};
    vec3_t normal = {-1, 0, 0};
    vec3_t verts[4];
    create_quad_portal(center, normal, 10, 10, verts);

    int portalIdx = World_AddPortal(&world, area0, area1, verts, 4);

    ASSERT(portalIdx == 0, "First portal should be index 0");
    ASSERT(world.numPortals == 1, "World should have 1 portal");
    ASSERT(world.portals[0].areaA == area0, "Portal areaA should match");
    ASSERT(world.portals[0].areaB == area1, "Portal areaB should match");
    PASS();
}

/* ============== RECURSIVE RENDERING TESTS ============== */

TEST(render_single_area)
{
    world_t world;
    World_Init(&world);

    /* Create one area */
    int area0 = World_AddArea(&world);
    world.areas[area0].mins[0] = 0; world.areas[area0].mins[1] = -10; world.areas[area0].mins[2] = -10;
    world.areas[area0].maxs[0] = 50; world.areas[area0].maxs[1] = 10; world.areas[area0].maxs[2] = 10;

    /* Create leaves in the area */
    mleaf_t leaves[2];
    memset(leaves, 0, sizeof(leaves));
    leaves[0].area = area0;
    leaves[0].mins[0] = 10; leaves[0].mins[1] = -5; leaves[0].mins[2] = -5;
    leaves[0].maxs[0] = 20; leaves[0].maxs[1] = 5; leaves[0].maxs[2] = 5;
    leaves[1].area = area0;
    leaves[1].mins[0] = 30; leaves[1].mins[1] = -5; leaves[1].mins[2] = -5;
    leaves[1].maxs[0] = 40; leaves[1].maxs[1] = 5; leaves[1].maxs[2] = 5;

    world.leaves = leaves;
    world.numLeaves = 2;

    /* Camera inside area looking forward */
    camera_t camera;
    vec3_t origin = {5, 0, 0};
    vec3_t forward = {1, 0, 0};
    setup_camera(&camera, origin, forward);

    World_MarkVisibleLeaves(&world, &camera);

    /* Both leaves should be visible */
    ASSERT(leaves[0].visframe == world.visframecount, "Leaf 0 should be visible");
    ASSERT(leaves[1].visframe == world.visframecount, "Leaf 1 should be visible");
    PASS();
}

TEST(render_two_areas_open_portal)
{
    world_t world;
    World_Init(&world);

    /* Area A (x: 0-50), Area B (x: 50-100) */
    int areaA = World_AddArea(&world);
    int areaB = World_AddArea(&world);

    world.areas[areaA].mins[0] = 0;  world.areas[areaA].mins[1] = -20; world.areas[areaA].mins[2] = -20;
    world.areas[areaA].maxs[0] = 50; world.areas[areaA].maxs[1] = 20;  world.areas[areaA].maxs[2] = 20;

    world.areas[areaB].mins[0] = 50;  world.areas[areaB].mins[1] = -20; world.areas[areaB].mins[2] = -20;
    world.areas[areaB].maxs[0] = 100; world.areas[areaB].maxs[1] = 20;  world.areas[areaB].maxs[2] = 20;

    /* Portal between areas at x=50 */
    vec3_t center = {50, 0, 0};
    vec3_t normal = {-1, 0, 0};  /* Faces area A */
    vec3_t verts[4];
    create_quad_portal(center, normal, 10, 10, verts);
    World_AddPortal(&world, areaA, areaB, verts, 4);

    /* Leaves: one in each area */
    mleaf_t leaves[2];
    memset(leaves, 0, sizeof(leaves));
    leaves[0].area = areaA;
    leaves[0].mins[0] = 10; leaves[0].mins[1] = -5; leaves[0].mins[2] = -5;
    leaves[0].maxs[0] = 30; leaves[0].maxs[1] = 5;  leaves[0].maxs[2] = 5;
    leaves[1].area = areaB;
    leaves[1].mins[0] = 60; leaves[1].mins[1] = -5; leaves[1].mins[2] = -5;
    leaves[1].maxs[0] = 80; leaves[1].maxs[1] = 5;  leaves[1].maxs[2] = 5;

    world.leaves = leaves;
    world.numLeaves = 2;

    /* Camera in area A looking toward area B */
    camera_t camera;
    vec3_t origin = {25, 0, 0};
    vec3_t forward = {1, 0, 0};
    setup_camera(&camera, origin, forward);

    World_MarkVisibleLeaves(&world, &camera);

    /* Both leaves should be visible (portal is open) */
    ASSERT(leaves[0].visframe == world.visframecount, "Leaf in area A should be visible");
    ASSERT(leaves[1].visframe == world.visframecount, "Leaf in area B should be visible through open portal");
    PASS();
}

TEST(render_two_areas_closed_portal)
{
    world_t world;
    World_Init(&world);

    /* Same setup as above */
    int areaA = World_AddArea(&world);
    int areaB = World_AddArea(&world);

    world.areas[areaA].mins[0] = 0;  world.areas[areaA].mins[1] = -20; world.areas[areaA].mins[2] = -20;
    world.areas[areaA].maxs[0] = 50; world.areas[areaA].maxs[1] = 20;  world.areas[areaA].maxs[2] = 20;

    world.areas[areaB].mins[0] = 50;  world.areas[areaB].mins[1] = -20; world.areas[areaB].mins[2] = -20;
    world.areas[areaB].maxs[0] = 100; world.areas[areaB].maxs[1] = 20;  world.areas[areaB].maxs[2] = 20;

    vec3_t center = {50, 0, 0};
    vec3_t normal = {-1, 0, 0};
    vec3_t verts[4];
    create_quad_portal(center, normal, 10, 10, verts);
    int portalIdx = World_AddPortal(&world, areaA, areaB, verts, 4);

    /* CLOSE the portal (simulating a closed door) */
    world.portals[portalIdx].open = false;

    mleaf_t leaves[2];
    memset(leaves, 0, sizeof(leaves));
    leaves[0].area = areaA;
    leaves[0].mins[0] = 10; leaves[0].mins[1] = -5; leaves[0].mins[2] = -5;
    leaves[0].maxs[0] = 30; leaves[0].maxs[1] = 5;  leaves[0].maxs[2] = 5;
    leaves[1].area = areaB;
    leaves[1].mins[0] = 60; leaves[1].mins[1] = -5; leaves[1].mins[2] = -5;
    leaves[1].maxs[0] = 80; leaves[1].maxs[1] = 5;  leaves[1].maxs[2] = 5;

    world.leaves = leaves;
    world.numLeaves = 2;

    camera_t camera;
    vec3_t origin = {25, 0, 0};
    vec3_t forward = {1, 0, 0};
    setup_camera(&camera, origin, forward);

    World_MarkVisibleLeaves(&world, &camera);

    /* Leaf in area A visible, but area B should NOT be visible (portal closed) */
    ASSERT(leaves[0].visframe == world.visframecount, "Leaf in area A should be visible");
    ASSERT(leaves[1].visframe != world.visframecount,
           "Leaf in area B should NOT be visible through closed portal");
    PASS();
}

TEST(render_looking_away)
{
    world_t world;
    World_Init(&world);

    /* Area A (x: 0-50), Area B (x: 50-100) */
    int areaA = World_AddArea(&world);
    int areaB = World_AddArea(&world);

    world.areas[areaA].mins[0] = 0;  world.areas[areaA].mins[1] = -20; world.areas[areaA].mins[2] = -20;
    world.areas[areaA].maxs[0] = 50; world.areas[areaA].maxs[1] = 20;  world.areas[areaA].maxs[2] = 20;

    world.areas[areaB].mins[0] = 50;  world.areas[areaB].mins[1] = -20; world.areas[areaB].mins[2] = -20;
    world.areas[areaB].maxs[0] = 100; world.areas[areaB].maxs[1] = 20;  world.areas[areaB].maxs[2] = 20;

    vec3_t center = {50, 0, 0};
    vec3_t normal = {-1, 0, 0};
    vec3_t verts[4];
    create_quad_portal(center, normal, 10, 10, verts);
    World_AddPortal(&world, areaA, areaB, verts, 4);

    mleaf_t leaves[2];
    memset(leaves, 0, sizeof(leaves));
    leaves[0].area = areaA;
    leaves[0].mins[0] = 10; leaves[0].mins[1] = -5; leaves[0].mins[2] = -5;
    leaves[0].maxs[0] = 30; leaves[0].maxs[1] = 5;  leaves[0].maxs[2] = 5;
    leaves[1].area = areaB;
    leaves[1].mins[0] = 60; leaves[1].mins[1] = -5; leaves[1].mins[2] = -5;
    leaves[1].maxs[0] = 80; leaves[1].maxs[1] = 5;  leaves[1].maxs[2] = 5;

    world.leaves = leaves;
    world.numLeaves = 2;

    /* Camera in area A looking AWAY from portal */
    camera_t camera;
    vec3_t origin = {25, 0, 0};
    vec3_t forward = {-1, 0, 0};  /* Looking away from portal */
    setup_camera(&camera, origin, forward);

    World_MarkVisibleLeaves(&world, &camera);

    /* Leaf in area A might be visible, but area B should NOT (not in frustum) */
    ASSERT(leaves[1].visframe != world.visframecount,
           "Leaf in area B should NOT be visible when looking away");
    PASS();
}

TEST(render_three_areas_chain)
{
    world_t world;
    World_Init(&world);

    /* Three areas in a row: A -> B -> C */
    int areaA = World_AddArea(&world);
    int areaB = World_AddArea(&world);
    int areaC = World_AddArea(&world);

    world.areas[areaA].mins[0] = 0;   world.areas[areaA].mins[1] = -20; world.areas[areaA].mins[2] = -20;
    world.areas[areaA].maxs[0] = 50;  world.areas[areaA].maxs[1] = 20;  world.areas[areaA].maxs[2] = 20;

    world.areas[areaB].mins[0] = 50;  world.areas[areaB].mins[1] = -20; world.areas[areaB].mins[2] = -20;
    world.areas[areaB].maxs[0] = 100; world.areas[areaB].maxs[1] = 20;  world.areas[areaB].maxs[2] = 20;

    world.areas[areaC].mins[0] = 100; world.areas[areaC].mins[1] = -20; world.areas[areaC].mins[2] = -20;
    world.areas[areaC].maxs[0] = 150; world.areas[areaC].maxs[1] = 20;  world.areas[areaC].maxs[2] = 20;

    /* Portal A->B at x=50 */
    vec3_t center1 = {50, 0, 0};
    vec3_t normal1 = {-1, 0, 0};
    vec3_t verts1[4];
    create_quad_portal(center1, normal1, 10, 10, verts1);
    World_AddPortal(&world, areaA, areaB, verts1, 4);

    /* Portal B->C at x=100 */
    vec3_t center2 = {100, 0, 0};
    vec3_t normal2 = {-1, 0, 0};
    vec3_t verts2[4];
    create_quad_portal(center2, normal2, 10, 10, verts2);
    World_AddPortal(&world, areaB, areaC, verts2, 4);

    mleaf_t leaves[3];
    memset(leaves, 0, sizeof(leaves));
    leaves[0].area = areaA;
    leaves[0].mins[0] = 10; leaves[0].mins[1] = -5; leaves[0].mins[2] = -5;
    leaves[0].maxs[0] = 30; leaves[0].maxs[1] = 5;  leaves[0].maxs[2] = 5;
    leaves[1].area = areaB;
    leaves[1].mins[0] = 60; leaves[1].mins[1] = -5; leaves[1].mins[2] = -5;
    leaves[1].maxs[0] = 80; leaves[1].maxs[1] = 5;  leaves[1].maxs[2] = 5;
    leaves[2].area = areaC;
    leaves[2].mins[0] = 110; leaves[2].mins[1] = -5; leaves[2].mins[2] = -5;
    leaves[2].maxs[0] = 130; leaves[2].maxs[1] = 5;  leaves[2].maxs[2] = 5;

    world.leaves = leaves;
    world.numLeaves = 3;

    camera_t camera;
    vec3_t origin = {25, 0, 0};
    vec3_t forward = {1, 0, 0};
    setup_camera(&camera, origin, forward);

    World_MarkVisibleLeaves(&world, &camera);

    /* All three leaves should be visible through portal chain */
    ASSERT(leaves[0].visframe == world.visframecount, "Leaf in area A should be visible");
    ASSERT(leaves[1].visframe == world.visframecount, "Leaf in area B should be visible");
    ASSERT(leaves[2].visframe == world.visframecount, "Leaf in area C should be visible through portal chain");
    PASS();
}

/* ============== MAIN ============== */

int main(void)
{
    printf("===========================================\n");
#ifdef TEST_SOLUTION
    printf("Portal Culling Tests (SOLUTION VERSION)\n");
#else
    printf("Portal Culling Tests (GAME VERSION)\n");
#endif
    printf("===========================================\n\n");

    /* Plane tests */
    printf("--- Plane Tests ---\n");
    run_test_plane_distance();
    run_test_plane_from_points();
    run_test_plane_point_side();

    /* Frustum tests */
    printf("\n--- Frustum Tests ---\n");
    run_test_frustum_from_camera();
    run_test_frustum_cull_box();
    run_test_frustum_cull_sphere();
    run_test_frustum_cull_polygon();

    /* Portal tests */
    printf("\n--- Portal Tests ---\n");
    run_test_portal_init();
    run_test_portal_set_vertices();
    run_test_portal_visibility_facing();
    run_test_portal_visibility_behind();
    run_test_portal_visibility_closed();
    run_test_portal_visibility_outside_frustum();
    run_test_portal_clip_frustum();

    /* World tests */
    printf("\n--- World Tests ---\n");
    run_test_world_init();
    run_test_world_add_area();
    run_test_world_add_portal();

    /* Recursive rendering tests */
    printf("\n--- Recursive Rendering Tests ---\n");
    run_test_render_single_area();
    run_test_render_two_areas_open_portal();
    run_test_render_two_areas_closed_portal();
    run_test_render_looking_away();
    run_test_render_three_areas_chain();

    /* Summary */
    printf("\n===========================================\n");
    printf("RESULTS: %d passed, %d failed\n", tests_passed, tests_failed);
    printf("===========================================\n");

    return (tests_failed > 0) ? 1 : 0;
}
