/*
 * BSP Traversal Test Harness
 *
 * Tests the R_RecursiveWorldNode function to verify correct BSP tree traversal.
 * Mocks Quake's data structures to create isolated unit tests.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

/* Mock Quake types and definitions */
typedef float vec_t;
typedef vec_t vec3_t[3];

#define M_PI 3.14159265358979323846

/* Plane types for fast axis-aligned checks */
#define PLANE_X     0
#define PLANE_Y     1
#define PLANE_Z     2
#define PLANE_ANYX  3
#define PLANE_ANYY  4
#define PLANE_ANYZ  5

/* Content types */
#define CONTENTS_EMPTY    -1
#define CONTENTS_SOLID    -2
#define CONTENTS_WATER    -3
#define CONTENTS_SLIME    -4
#define CONTENTS_LAVA     -5
#define CONTENTS_SKY      -6

/* Surface flags */
#define SURF_PLANEBACK    2

#define BACKFACE_EPSILON  0.01

#define DotProduct(a,b) ((a)[0]*(b)[0] + (a)[1]*(b)[1] + (a)[2]*(b)[2])
#define VectorCopy(s,d) {(d)[0]=(s)[0];(d)[1]=(s)[1];(d)[2]=(s)[2];}

typedef struct mplane_s {
    vec3_t  normal;
    float   dist;
    int     type;
} mplane_t;

/* Forward declaration for mutual reference */
struct mnode_s;
struct mleaf_s;

typedef struct msurface_s {
    int         visframe;
    int         flags;
    mplane_t    *plane;
    int         test_id;  /* For tracking which surfaces were visited */
} msurface_t;

typedef struct mnode_s {
    int             contents;       /* 0 for nodes, negative for leaves */
    int             visframe;
    float           minmaxs[6];
    struct mnode_s  *parent;
    mplane_t        *plane;
    struct mnode_s  *children[2];
    int             firstsurface;
    int             numsurfaces;
    int             test_id;        /* For tracking traversal order */
} mnode_t;

typedef struct mleaf_s {
    int             contents;
    int             visframe;
    float           minmaxs[6];
    struct mnode_s  *parent;
    msurface_t      **firstmarksurface;
    int             nummarksurfaces;
    int             key;
    void            *efrags;
    int             test_id;
} mleaf_t;

typedef struct {
    msurface_t  *surfaces;
    int         nummodelsurfaces;
    int         firstmodelsurface;
} model_t;

typedef struct {
    model_t     *worldmodel;
} client_t;

/* Global state (mocked) */
int r_visframecount = 1;
int r_framecount = 1;
int r_currentkey = 0;
vec3_t modelorg = {0, 0, 0};

client_t cl;

/* View frustum (disabled for tests) */
int pfrustum_indexes[4][6] = {0};
struct {
    vec3_t normal;
    float dist;
} view_clipplanes[4] = {0};

/* Test tracking */
#define MAX_VISITED 100
int visited_nodes[MAX_VISITED];
int visited_count = 0;
int surfaces_marked = 0;

/* Mock functions */
void R_StoreEfrags(void **efrags) { /* No-op for testing */ }
void R_RenderFace(msurface_t *surf, int clipflags) { surfaces_marked++; }
void R_RenderPoly(msurface_t *surf, int clipflags) { surfaces_marked++; }

int r_drawpolys = 0;
int r_worldpolysbacktofront = 0;

#define MAX_BTOFPOLYS 256
int numbtofpolys = 0;
struct {
    int clipflags;
    msurface_t *psurf;
} *pbtofpolys;


/* Track node visits for testing */
void track_node_visit(int node_id) {
    if (visited_count < MAX_VISITED) {
        visited_nodes[visited_count++] = node_id;
    }
}

/*
 * Include the actual r_bsp.c code here for testing.
 * In the real test, this would be compiled separately.
 */

/* ============== FUNCTION UNDER TEST ============== */

void R_RecursiveWorldNode(mnode_t *node, int clipflags) {
    int         i, c, side;
    mplane_t    *plane;
    msurface_t  *surf, **mark;
    mleaf_t     *pleaf;
    double      dot;

    if (node->contents == CONTENTS_SOLID)
        return;

    if (node->visframe != r_visframecount)
        return;

    /* Track this visit for testing */
    track_node_visit(node->test_id);

    /* Skip frustum culling for unit tests */

    /* if a leaf node, draw stuff */
    if (node->contents < 0)
    {
        pleaf = (mleaf_t *)node;

        mark = pleaf->firstmarksurface;
        c = pleaf->nummarksurfaces;

        if (c)
        {
            do
            {
                (*mark)->visframe = r_framecount;
                mark++;
            } while (--c);
        }

        if (pleaf->efrags)
        {
            R_StoreEfrags(&pleaf->efrags);
        }

        pleaf->key = r_currentkey;
        r_currentkey++;
    }
    else
    {
        plane = node->plane;

        switch (plane->type)
        {
        case PLANE_X:
            dot = modelorg[0] - plane->dist;
            break;
        case PLANE_Y:
            dot = modelorg[1] - plane->dist;
            break;
        case PLANE_Z:
            dot = modelorg[2] - plane->dist;
            break;
        default:
            dot = DotProduct(modelorg, plane->normal) - plane->dist;
            break;
        }

        if (dot >= 0)
            side = 0;
        else
            side = 1;

        /* BUG: This visframe check with >= is incorrect */
        if (node->children[side]->visframe >= r_visframecount)
        {
            R_RecursiveWorldNode(node->children[side], clipflags);
        }

        c = node->numsurfaces;

        if (c)
        {
            surf = cl.worldmodel->surfaces + node->firstsurface;

            if (dot < -BACKFACE_EPSILON)
            {
                do
                {
                    if ((surf->flags & SURF_PLANEBACK) &&
                        (surf->visframe == r_framecount))
                    {
                        R_RenderFace(surf, clipflags);
                    }
                    surf++;
                } while (--c);
            }
            else if (dot > BACKFACE_EPSILON)
            {
                do
                {
                    if (!(surf->flags & SURF_PLANEBACK) &&
                        (surf->visframe == r_framecount))
                    {
                        R_RenderFace(surf, clipflags);
                    }
                    surf++;
                } while (--c);
            }

            r_currentkey++;
        }

        /* BUG: Same incorrect visframe check on back side */
        if (node->children[!side]->visframe >= r_visframecount)
        {
            R_RecursiveWorldNode(node->children[!side], clipflags);
        }
    }
}

/* ============== TEST CASES ============== */

/*
 * Create a simple BSP tree for testing:
 *
 *         [0]          (root node, splits on X at 0)
 *        /   \
 *      [1]   [2]       (nodes, split on Y at 0)
 *     /   \ /   \
 *   [3] [4][5] [6]     (leaves)
 */

mplane_t planes[3];
mnode_t nodes[3];
mleaf_t leaves[4];
msurface_t surfaces[10];
msurface_t *marksurfaces[4];
model_t worldmodel;

void setup_test_bsp(void) {
    memset(planes, 0, sizeof(planes));
    memset(nodes, 0, sizeof(nodes));
    memset(leaves, 0, sizeof(leaves));
    memset(surfaces, 0, sizeof(surfaces));

    /* Setup planes */
    planes[0].normal[0] = 1; planes[0].dist = 0; planes[0].type = PLANE_X;
    planes[1].normal[1] = 1; planes[1].dist = 0; planes[1].type = PLANE_Y;
    planes[2].normal[1] = 1; planes[2].dist = 0; planes[2].type = PLANE_Y;

    /* Setup root node */
    nodes[0].contents = 0;
    nodes[0].visframe = r_visframecount;
    nodes[0].plane = &planes[0];
    nodes[0].children[0] = &nodes[1];
    nodes[0].children[1] = &nodes[2];
    nodes[0].test_id = 0;
    nodes[0].numsurfaces = 1;
    nodes[0].firstsurface = 0;

    /* Setup child nodes */
    nodes[1].contents = 0;
    nodes[1].visframe = r_visframecount;
    nodes[1].plane = &planes[1];
    nodes[1].children[0] = (mnode_t*)&leaves[0];
    nodes[1].children[1] = (mnode_t*)&leaves[1];
    nodes[1].test_id = 1;
    nodes[1].numsurfaces = 1;
    nodes[1].firstsurface = 1;

    nodes[2].contents = 0;
    nodes[2].visframe = r_visframecount;
    nodes[2].plane = &planes[2];
    nodes[2].children[0] = (mnode_t*)&leaves[2];
    nodes[2].children[1] = (mnode_t*)&leaves[3];
    nodes[2].test_id = 2;
    nodes[2].numsurfaces = 1;
    nodes[2].firstsurface = 2;

    /* Setup leaves */
    for (int i = 0; i < 4; i++) {
        leaves[i].contents = CONTENTS_EMPTY;
        leaves[i].visframe = r_visframecount;
        leaves[i].nummarksurfaces = 1;
        leaves[i].test_id = 10 + i;
        marksurfaces[i] = &surfaces[3 + i];
        leaves[i].firstmarksurface = &marksurfaces[i];
        leaves[i].efrags = NULL;
    }

    /* Setup surfaces */
    for (int i = 0; i < 7; i++) {
        surfaces[i].visframe = 0;
        surfaces[i].flags = 0;
        surfaces[i].test_id = i;
    }

    /* Setup world model */
    worldmodel.surfaces = surfaces;
    worldmodel.nummodelsurfaces = 7;
    cl.worldmodel = &worldmodel;
}

void reset_test_state(void) {
    visited_count = 0;
    surfaces_marked = 0;
    r_currentkey = 0;

    for (int i = 0; i < 7; i++) {
        surfaces[i].visframe = 0;
    }
}

int test_all_visible_nodes_visited(void) {
    printf("Test: All visible nodes are visited\n");

    setup_test_bsp();
    reset_test_state();

    /* Set camera at origin */
    modelorg[0] = 0;
    modelorg[1] = 0;
    modelorg[2] = 0;

    R_RecursiveWorldNode(&nodes[0], 0);

    /* Should visit all 3 nodes + 4 leaves = 7 total */
    printf("  Visited %d nodes/leaves\n", visited_count);

    /* With the bug, some nodes may be skipped */
    if (visited_count < 7) {
        printf("  FAIL: Only visited %d of 7 expected nodes\n", visited_count);
        return 0;
    }

    printf("  PASS\n");
    return 1;
}

int test_visframe_boundary(void) {
    printf("Test: Visframe boundary condition\n");

    setup_test_bsp();
    reset_test_state();

    /* Test edge case: visframe just past the current count */
    r_visframecount = 100;
    for (int i = 0; i < 3; i++) {
        nodes[i].visframe = 100;
    }
    for (int i = 0; i < 4; i++) {
        leaves[i].visframe = 100;
    }

    /* Add one node with future visframe (should still be visited with bug) */
    leaves[2].visframe = 101;

    modelorg[0] = 0;
    modelorg[1] = 0;
    modelorg[2] = 0;

    R_RecursiveWorldNode(&nodes[0], 0);

    /* With correct code: leaf[2] should NOT be visited (visframe != r_visframecount)
       With buggy code: leaf[2] WILL be visited (101 >= 100 is true) */

    int found_leaf2 = 0;
    for (int i = 0; i < visited_count; i++) {
        if (visited_nodes[i] == 12) { /* leaf[2] has test_id 12 */
            found_leaf2 = 1;
            break;
        }
    }

    if (found_leaf2) {
        printf("  FAIL: Visited node with incorrect visframe (bug present)\n");
        return 0;
    }

    printf("  PASS\n");
    return 1;
}

int test_front_to_back_order(void) {
    printf("Test: Front-to-back traversal order\n");

    setup_test_bsp();
    reset_test_state();

    /* Camera on positive X side - should visit node[1] subtree first */
    modelorg[0] = 10;
    modelorg[1] = 10;
    modelorg[2] = 0;

    R_RecursiveWorldNode(&nodes[0], 0);

    /* Expected order: 0, 1, 10 (leaf0), 11 (leaf1), 2, 12 (leaf2), 13 (leaf3) */
    /* First node after root should be 1 (front side of root plane) */

    if (visited_count < 2) {
        printf("  FAIL: Not enough nodes visited\n");
        return 0;
    }

    if (visited_nodes[0] != 0) {
        printf("  FAIL: Root not visited first\n");
        return 0;
    }

    if (visited_nodes[1] != 1) {
        printf("  FAIL: Front child not visited second (got %d)\n", visited_nodes[1]);
        return 0;
    }

    printf("  PASS\n");
    return 1;
}

int test_node_with_zero_visframe(void) {
    printf("Test: Node with visframe=0 should be skipped\n");

    setup_test_bsp();
    reset_test_state();

    /* Set one node to have visframe 0 (not visible) */
    nodes[1].visframe = 0;

    modelorg[0] = 10;
    modelorg[1] = 10;
    modelorg[2] = 0;

    R_RecursiveWorldNode(&nodes[0], 0);

    /* Node 1 and its children should NOT be visited */
    int found_node1 = 0;
    for (int i = 0; i < visited_count; i++) {
        if (visited_nodes[i] == 1) {
            found_node1 = 1;
            break;
        }
    }

    if (found_node1) {
        printf("  FAIL: Invisible node was visited\n");
        return 0;
    }

    printf("  PASS\n");
    return 1;
}

int main(void) {
    printf("BSP Traversal Tests\n");
    printf("===================\n\n");

    int passed = 0;
    int total = 4;

    passed += test_all_visible_nodes_visited();
    passed += test_visframe_boundary();
    passed += test_front_to_back_order();
    passed += test_node_with_zero_visframe();

    printf("\n===================\n");
    printf("Results: %d/%d tests passed\n", passed, total);

    return (passed == total) ? 0 : 1;
}
