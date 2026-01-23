/*
 * Skeletal Animation Test Harness
 *
 * Tests all components of the skeletal animation system:
 * 1. Quaternion math (identity, normalize, multiply, SLERP, toMatrix)
 * 2. Matrix operations (identity, multiply, invert, transform)
 * 3. Skeleton operations (hierarchy, inverse bind pose, world transforms)
 * 4. Animation sampling and blending
 * 5. Vertex skinning
 *
 * Build:
 *   make test        - Test unoptimized (game) version
 *   make test_opt    - Test optimized (solution) version
 *   make compare     - Compare both versions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

/* Include the implementation under test */
#ifdef TEST_SOLUTION
#include "../solution/skeleton.c"
#else
#include "../game/skeleton.c"
#endif

/* ============== TEST UTILITIES ============== */

static int tests_passed = 0;
static int tests_failed = 0;

#define EPSILON 0.001f

static int float_eq(float a, float b)
{
    return fabsf(a - b) < EPSILON;
}

static int vec3_eq(vec3_t a, vec3_t b)
{
    return float_eq(a[0], b[0]) && float_eq(a[1], b[1]) && float_eq(a[2], b[2]);
}

static int vec4_eq(vec4_t a, vec4_t b)
{
    return float_eq(a[0], b[0]) && float_eq(a[1], b[1]) &&
           float_eq(a[2], b[2]) && float_eq(a[3], b[3]);
}

static void print_vec3(const char *name, vec3_t v)
{
    printf("  %s: (%.4f, %.4f, %.4f)\n", name, v[0], v[1], v[2]);
}

static void print_vec4(const char *name, vec4_t v)
{
    printf("  %s: (%.4f, %.4f, %.4f, %.4f)\n", name, v[0], v[1], v[2], v[3]);
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

/* ============== QUATERNION TESTS ============== */

TEST(quat_identity)
{
    vec4_t q;
    Quat_Identity(q);

    ASSERT(float_eq(q[0], 0.0f), "x should be 0");
    ASSERT(float_eq(q[1], 0.0f), "y should be 0");
    ASSERT(float_eq(q[2], 0.0f), "z should be 0");
    ASSERT(float_eq(q[3], 1.0f), "w should be 1");
    PASS();
}

TEST(quat_normalize)
{
    vec4_t q = {1.0f, 2.0f, 3.0f, 4.0f};
    Quat_Normalize(q);

    float len = sqrtf(q[0]*q[0] + q[1]*q[1] + q[2]*q[2] + q[3]*q[3]);
    ASSERT(float_eq(len, 1.0f), "Normalized quaternion should have length 1");
    PASS();
}

TEST(quat_multiply_identity)
{
    vec4_t a = {0.5f, 0.5f, 0.5f, 0.5f};  /* Unit quaternion */
    vec4_t identity = {0.0f, 0.0f, 0.0f, 1.0f};
    vec4_t result;

    Quat_Multiply(a, identity, result);

    ASSERT(vec4_eq(result, a), "a * identity should equal a");
    PASS();
}

TEST(quat_multiply_inverse)
{
    /* Create a 90-degree rotation around Y axis */
    vec3_t axis = {0.0f, 1.0f, 0.0f};
    vec4_t q, qInv, result;

    Quat_FromAxisAngle(axis, (float)M_PI / 2.0f, q);

    /* Create inverse (conjugate for unit quaternion) */
    qInv[0] = -q[0];
    qInv[1] = -q[1];
    qInv[2] = -q[2];
    qInv[3] = q[3];

    Quat_Multiply(q, qInv, result);

    /* Result should be identity */
    ASSERT(float_eq(result[3], 1.0f) || float_eq(result[3], -1.0f),
           "q * q^-1 should be identity (w=1 or w=-1)");
    ASSERT(float_eq(fabsf(result[0]), 0.0f) &&
           float_eq(fabsf(result[1]), 0.0f) &&
           float_eq(fabsf(result[2]), 0.0f),
           "q * q^-1 should have zero xyz");
    PASS();
}

TEST(quat_slerp_endpoints)
{
    vec4_t a = {0.0f, 0.0f, 0.0f, 1.0f};  /* Identity */
    vec3_t axis = {0.0f, 1.0f, 0.0f};
    vec4_t b, result;

    Quat_FromAxisAngle(axis, (float)M_PI / 2.0f, b);

    /* t=0 should return a */
    Quat_Slerp(a, b, 0.0f, result);
    ASSERT(vec4_eq(result, a), "SLERP at t=0 should return first quaternion");

    /* t=1 should return b */
    Quat_Slerp(a, b, 1.0f, result);
    ASSERT(vec4_eq(result, b), "SLERP at t=1 should return second quaternion");

    PASS();
}

TEST(quat_slerp_halfway)
{
    vec4_t a = {0.0f, 0.0f, 0.0f, 1.0f};  /* Identity */
    vec3_t axis = {0.0f, 1.0f, 0.0f};
    vec4_t b, result, expected;

    /* 90 degrees around Y */
    Quat_FromAxisAngle(axis, (float)M_PI / 2.0f, b);

    /* Expected: 45 degrees around Y */
    Quat_FromAxisAngle(axis, (float)M_PI / 4.0f, expected);

    Quat_Slerp(a, b, 0.5f, result);

    ASSERT(vec4_eq(result, expected), "SLERP at t=0.5 should be halfway rotation");
    PASS();
}

TEST(quat_slerp_shortest_path)
{
    /* Test that SLERP takes the shortest path when quaternions are opposite */
    vec3_t axis = {0.0f, 1.0f, 0.0f};
    vec4_t a, b, result;

    Quat_FromAxisAngle(axis, 0.1f, a);
    /* Create b as negated version of a different rotation */
    Quat_FromAxisAngle(axis, 0.2f, b);
    b[0] = -b[0]; b[1] = -b[1]; b[2] = -b[2]; b[3] = -b[3];

    /* SLERP should still produce valid result */
    Quat_Slerp(a, b, 0.5f, result);

    float len = sqrtf(result[0]*result[0] + result[1]*result[1] +
                      result[2]*result[2] + result[3]*result[3]);
    ASSERT(float_eq(len, 1.0f), "SLERP result should be unit quaternion");
    PASS();
}

TEST(quat_to_matrix_identity)
{
    vec4_t q = {0.0f, 0.0f, 0.0f, 1.0f};
    mat4_t m;

    Quat_ToMatrix(q, m);

    /* Should be identity matrix (rotation part) */
    ASSERT(float_eq(m[0][0], 1.0f) && float_eq(m[1][1], 1.0f) && float_eq(m[2][2], 1.0f),
           "Identity quaternion should produce identity rotation matrix");
    ASSERT(float_eq(m[0][1], 0.0f) && float_eq(m[0][2], 0.0f) &&
           float_eq(m[1][0], 0.0f) && float_eq(m[1][2], 0.0f) &&
           float_eq(m[2][0], 0.0f) && float_eq(m[2][1], 0.0f),
           "Off-diagonal elements should be zero");
    PASS();
}

TEST(quat_to_matrix_90_y)
{
    vec3_t axis = {0.0f, 1.0f, 0.0f};
    vec4_t q;
    mat4_t m;

    Quat_FromAxisAngle(axis, (float)M_PI / 2.0f, q);
    Quat_ToMatrix(q, m);

    /* 90 degrees around Y: X->-Z, Z->X */
    vec3_t xAxis = {1.0f, 0.0f, 0.0f};
    vec3_t result;
    Mat4_TransformVector(m, xAxis, result);

    /* X axis should become approximately -Z axis */
    ASSERT(float_eq(result[0], 0.0f) && float_eq(result[1], 0.0f) && float_eq(result[2], -1.0f),
           "90-degree Y rotation should map X to -Z");
    PASS();
}

/* ============== MATRIX TESTS ============== */

TEST(mat4_identity)
{
    mat4_t m;
    Mat4_Identity(m);

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            float expected = (i == j) ? 1.0f : 0.0f;
            ASSERT(float_eq(m[i][j], expected), "Identity matrix element incorrect");
        }
    }
    PASS();
}

TEST(mat4_multiply_identity)
{
    mat4_t a, identity, result;

    /* Create a transform matrix */
    vec3_t pos = {1.0f, 2.0f, 3.0f};
    vec4_t rot = {0.0f, 0.0f, 0.0f, 1.0f};
    vec3_t scale = {1.0f, 1.0f, 1.0f};
    Mat4_FromPosRotScale(pos, rot, scale, a);
    Mat4_Identity(identity);

    Mat4_Multiply(a, identity, result);

    /* Result should equal a */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            ASSERT(float_eq(result[i][j], a[i][j]), "A * I should equal A");
        }
    }
    PASS();
}

TEST(mat4_invert)
{
    mat4_t m, mInv, result;

    /* Create a transform matrix */
    vec3_t pos = {5.0f, 10.0f, 15.0f};
    vec3_t axis = {0.0f, 1.0f, 0.0f};
    vec4_t rot;
    Quat_FromAxisAngle(axis, (float)M_PI / 4.0f, rot);
    vec3_t scale = {1.0f, 1.0f, 1.0f};

    Mat4_FromPosRotScale(pos, rot, scale, m);
    Mat4_Invert(m, mInv);
    Mat4_Multiply(m, mInv, result);

    /* Result should be identity */
    ASSERT(float_eq(result[0][0], 1.0f) && float_eq(result[1][1], 1.0f) &&
           float_eq(result[2][2], 1.0f) && float_eq(result[3][3], 1.0f),
           "M * M^-1 diagonal should be 1");
    ASSERT(float_eq(result[0][1], 0.0f) && float_eq(result[0][2], 0.0f) &&
           float_eq(result[1][0], 0.0f) && float_eq(result[1][2], 0.0f),
           "M * M^-1 off-diagonal should be 0");
    PASS();
}

TEST(mat4_transform_point)
{
    mat4_t m;
    vec3_t pos = {10.0f, 0.0f, 0.0f};
    vec4_t rot = {0.0f, 0.0f, 0.0f, 1.0f};  /* Identity */
    vec3_t scale = {1.0f, 1.0f, 1.0f};

    Mat4_FromPosRotScale(pos, rot, scale, m);

    vec3_t point = {1.0f, 2.0f, 3.0f};
    vec3_t result;
    Mat4_TransformPoint(m, point, result);

    vec3_t expected = {11.0f, 2.0f, 3.0f};  /* Translated by (10,0,0) */
    ASSERT(vec3_eq(result, expected), "Translation should add to point");
    PASS();
}

/* ============== SKELETON TESTS ============== */

TEST(skeleton_init)
{
    skeleton_t skel;
    Skel_Init(&skel);

    ASSERT(skel.numBones == 0, "Initialized skeleton should have 0 bones");
    PASS();
}

TEST(skeleton_add_bone)
{
    skeleton_t skel;
    Skel_Init(&skel);

    vec3_t pos = {0.0f, 1.0f, 0.0f};
    vec4_t rot = {0.0f, 0.0f, 0.0f, 1.0f};
    vec3_t scale = {1.0f, 1.0f, 1.0f};

    Skel_AddBone(&skel, "root", -1, pos, rot, scale);

    ASSERT(skel.numBones == 1, "Should have 1 bone after adding");
    ASSERT(strcmp(skel.bones[0].name, "root") == 0, "Bone name should match");
    ASSERT(skel.bones[0].parent == -1, "Root bone parent should be -1");
    PASS();
}

TEST(skeleton_hierarchy)
{
    skeleton_t skel;
    Skel_Init(&skel);

    vec4_t rot = {0.0f, 0.0f, 0.0f, 1.0f};
    vec3_t scale = {1.0f, 1.0f, 1.0f};

    /* Create a simple arm: shoulder -> elbow -> hand */
    vec3_t shoulderPos = {0.0f, 0.0f, 0.0f};
    vec3_t elbowPos = {0.0f, 2.0f, 0.0f};    /* 2 units up from shoulder */
    vec3_t handPos = {0.0f, 2.0f, 0.0f};     /* 2 units up from elbow */

    Skel_AddBone(&skel, "shoulder", -1, shoulderPos, rot, scale);
    Skel_AddBone(&skel, "elbow", 0, elbowPos, rot, scale);
    Skel_AddBone(&skel, "hand", 1, handPos, rot, scale);

    ASSERT(skel.numBones == 3, "Should have 3 bones");
    ASSERT(skel.bones[1].parent == 0, "Elbow parent should be shoulder (0)");
    ASSERT(skel.bones[2].parent == 1, "Hand parent should be elbow (1)");
    PASS();
}

TEST(skeleton_world_transforms_identity)
{
    skeleton_t skel;
    Skel_Init(&skel);

    vec4_t rot = {0.0f, 0.0f, 0.0f, 1.0f};
    vec3_t scale = {1.0f, 1.0f, 1.0f};
    vec3_t pos = {0.0f, 0.0f, 0.0f};

    Skel_AddBone(&skel, "root", -1, pos, rot, scale);
    Skel_CalculateInverseBindPose(&skel);

    /* Create a pose matching bind pose */
    bone_keyframe_t pose[1];
    VectorCopy(pos, pose[0].position);
    QuatCopy(rot, pose[0].rotation);
    VectorCopy(scale, pose[0].scale);

    mat4_t boneMatrices[1];
    Skel_CalculateWorldTransforms(&skel, pose, boneMatrices);

    /* When pose matches bind pose, skinning matrix should be identity */
    ASSERT(float_eq(boneMatrices[0][0][0], 1.0f) &&
           float_eq(boneMatrices[0][1][1], 1.0f) &&
           float_eq(boneMatrices[0][2][2], 1.0f),
           "Skinning matrix should be identity when pose matches bind pose");
    PASS();
}

TEST(skeleton_world_transforms_translation)
{
    skeleton_t skel;
    Skel_Init(&skel);

    vec4_t rot = {0.0f, 0.0f, 0.0f, 1.0f};
    vec3_t scale = {1.0f, 1.0f, 1.0f};
    vec3_t pos = {0.0f, 0.0f, 0.0f};

    Skel_AddBone(&skel, "root", -1, pos, rot, scale);
    Skel_CalculateInverseBindPose(&skel);

    /* Create a pose with translation */
    bone_keyframe_t pose[1];
    vec3_t newPos = {5.0f, 0.0f, 0.0f};
    VectorCopy(newPos, pose[0].position);
    QuatCopy(rot, pose[0].rotation);
    VectorCopy(scale, pose[0].scale);

    mat4_t boneMatrices[1];
    Skel_CalculateWorldTransforms(&skel, pose, boneMatrices);

    /* Test: transform origin should give (5,0,0) */
    vec3_t origin = {0.0f, 0.0f, 0.0f};
    vec3_t result;
    Mat4_TransformPoint(boneMatrices[0], origin, result);

    vec3_t expected = {5.0f, 0.0f, 0.0f};
    ASSERT(vec3_eq(result, expected),
           "Bone with translation should move vertices");
    PASS();
}

TEST(skeleton_hierarchy_transforms)
{
    skeleton_t skel;
    Skel_Init(&skel);

    vec4_t rot = {0.0f, 0.0f, 0.0f, 1.0f};
    vec3_t scale = {1.0f, 1.0f, 1.0f};

    /* Two bones: parent at origin, child 5 units up */
    vec3_t parentPos = {0.0f, 0.0f, 0.0f};
    vec3_t childPos = {0.0f, 5.0f, 0.0f};

    Skel_AddBone(&skel, "parent", -1, parentPos, rot, scale);
    Skel_AddBone(&skel, "child", 0, childPos, rot, scale);
    Skel_CalculateInverseBindPose(&skel);

    /* Create a pose where parent moves to (10,0,0) */
    bone_keyframe_t pose[2];
    vec3_t newParentPos = {10.0f, 0.0f, 0.0f};
    VectorCopy(newParentPos, pose[0].position);
    QuatCopy(rot, pose[0].rotation);
    VectorCopy(scale, pose[0].scale);

    VectorCopy(childPos, pose[1].position);  /* Child stays at local offset */
    QuatCopy(rot, pose[1].rotation);
    VectorCopy(scale, pose[1].scale);

    mat4_t boneMatrices[2];
    Skel_CalculateWorldTransforms(&skel, pose, boneMatrices);

    /* A vertex at bind position (0,5,0) for child bone
       should now be at (10,5,0) */
    vec3_t bindPos = {0.0f, 5.0f, 0.0f};
    vec3_t result;
    Mat4_TransformPoint(boneMatrices[1], bindPos, result);

    vec3_t expected = {10.0f, 5.0f, 0.0f};
    ASSERT(vec3_eq(result, expected),
           "Child bone should inherit parent translation");
    PASS();
}

/* ============== ANIMATION TESTS ============== */

TEST(animation_sample_first_frame)
{
    animation_clip_t clip;
    strcpy(clip.name, "test");
    clip.numFrames = 2;
    clip.fps = 30.0f;
    clip.numBones = 1;

    bone_keyframe_t frames[2];
    vec3_t pos0 = {0.0f, 0.0f, 0.0f};
    vec3_t pos1 = {10.0f, 0.0f, 0.0f};
    vec4_t rot = {0.0f, 0.0f, 0.0f, 1.0f};
    vec3_t scale = {1.0f, 1.0f, 1.0f};

    VectorCopy(pos0, frames[0].position);
    QuatCopy(rot, frames[0].rotation);
    VectorCopy(scale, frames[0].scale);

    VectorCopy(pos1, frames[1].position);
    QuatCopy(rot, frames[1].rotation);
    VectorCopy(scale, frames[1].scale);

    clip.frames = frames;

    bone_keyframe_t outPose[1];
    Anim_SamplePose(&clip, 0.0f, outPose);

    ASSERT(vec3_eq(outPose[0].position, pos0),
           "Sampling at t=0 should return first frame");
    PASS();
}

TEST(animation_sample_interpolation)
{
    animation_clip_t clip;
    strcpy(clip.name, "test");
    clip.numFrames = 2;
    clip.fps = 1.0f;  /* 1 FPS = 1 second per frame */
    clip.numBones = 1;

    bone_keyframe_t frames[2];
    vec3_t pos0 = {0.0f, 0.0f, 0.0f};
    vec3_t pos1 = {10.0f, 0.0f, 0.0f};
    vec4_t rot = {0.0f, 0.0f, 0.0f, 1.0f};
    vec3_t scale = {1.0f, 1.0f, 1.0f};

    VectorCopy(pos0, frames[0].position);
    QuatCopy(rot, frames[0].rotation);
    VectorCopy(scale, frames[0].scale);

    VectorCopy(pos1, frames[1].position);
    QuatCopy(rot, frames[1].rotation);
    VectorCopy(scale, frames[1].scale);

    clip.frames = frames;

    bone_keyframe_t outPose[1];
    Anim_SamplePose(&clip, 0.5f, outPose);

    /* At t=0.5s with 1 FPS, should be halfway between frames */
    vec3_t expected = {5.0f, 0.0f, 0.0f};
    ASSERT(vec3_eq(outPose[0].position, expected),
           "Sampling at t=0.5 should interpolate to halfway position");
    PASS();
}

TEST(animation_blend_poses)
{
    bone_keyframe_t poseA[1], poseB[1], outPose[1];

    vec3_t posA = {0.0f, 0.0f, 0.0f};
    vec3_t posB = {10.0f, 0.0f, 0.0f};
    vec4_t rot = {0.0f, 0.0f, 0.0f, 1.0f};
    vec3_t scale = {1.0f, 1.0f, 1.0f};

    VectorCopy(posA, poseA[0].position);
    QuatCopy(rot, poseA[0].rotation);
    VectorCopy(scale, poseA[0].scale);

    VectorCopy(posB, poseB[0].position);
    QuatCopy(rot, poseB[0].rotation);
    VectorCopy(scale, poseB[0].scale);

    Anim_BlendPoses(poseA, poseB, 1, 0.5f, outPose);

    vec3_t expected = {5.0f, 0.0f, 0.0f};
    ASSERT(vec3_eq(outPose[0].position, expected),
           "Blend at t=0.5 should produce halfway position");
    PASS();
}

TEST(animation_blend_rotation)
{
    bone_keyframe_t poseA[1], poseB[1], outPose[1];

    vec3_t pos = {0.0f, 0.0f, 0.0f};
    vec3_t axis = {0.0f, 1.0f, 0.0f};
    vec4_t rotA, rotB;
    vec3_t scale = {1.0f, 1.0f, 1.0f};

    Quat_FromAxisAngle(axis, 0.0f, rotA);          /* 0 degrees */
    Quat_FromAxisAngle(axis, (float)M_PI / 2.0f, rotB);  /* 90 degrees */

    VectorCopy(pos, poseA[0].position);
    QuatCopy(rotA, poseA[0].rotation);
    VectorCopy(scale, poseA[0].scale);

    VectorCopy(pos, poseB[0].position);
    QuatCopy(rotB, poseB[0].rotation);
    VectorCopy(scale, poseB[0].scale);

    Anim_BlendPoses(poseA, poseB, 1, 0.5f, outPose);

    /* Expected: 45 degrees around Y */
    vec4_t expected;
    Quat_FromAxisAngle(axis, (float)M_PI / 4.0f, expected);

    ASSERT(vec4_eq(outPose[0].rotation, expected),
           "Blend at t=0.5 should produce halfway rotation");
    PASS();
}

/* ============== SKINNING TESTS ============== */

TEST(skinning_single_bone)
{
    skeleton_t skel;
    Skel_Init(&skel);

    vec4_t rot = {0.0f, 0.0f, 0.0f, 1.0f};
    vec3_t scale = {1.0f, 1.0f, 1.0f};
    vec3_t pos = {0.0f, 0.0f, 0.0f};

    Skel_AddBone(&skel, "root", -1, pos, rot, scale);
    Skel_CalculateInverseBindPose(&skel);

    /* Create mesh with one vertex */
    skinned_mesh_t mesh;
    vec3_t bindPos = {1.0f, 2.0f, 3.0f};
    vec3_t bindNormal = {0.0f, 1.0f, 0.0f};
    vertex_skin_t skinData;

    mesh.numVertices = 1;
    mesh.bindPosePositions = &bindPos;
    mesh.bindPoseNormals = &bindNormal;
    mesh.skinData = &skinData;

    skinData.numWeights = 1;
    skinData.weights[0].boneIndex = 0;
    skinData.weights[0].weight = 1.0f;

    /* Create pose with translation */
    bone_keyframe_t pose[1];
    vec3_t newPos = {10.0f, 0.0f, 0.0f};
    VectorCopy(newPos, pose[0].position);
    QuatCopy(rot, pose[0].rotation);
    VectorCopy(scale, pose[0].scale);

    mat4_t boneMatrices[1];
    Skel_CalculateWorldTransforms(&skel, pose, boneMatrices);

    vec3_t outPositions[1];
    vec3_t outNormals[1];
    Skin_TransformVertices(&mesh, boneMatrices, &skel, outPositions, outNormals);

    vec3_t expectedPos = {11.0f, 2.0f, 3.0f};  /* Original + translation */
    ASSERT(vec3_eq(outPositions[0], expectedPos),
           "Single bone skinning should translate vertex");
    PASS();
}

TEST(skinning_two_bones_equal_weight)
{
    skeleton_t skel;
    Skel_Init(&skel);

    vec4_t rot = {0.0f, 0.0f, 0.0f, 1.0f};
    vec3_t scale = {1.0f, 1.0f, 1.0f};
    vec3_t pos = {0.0f, 0.0f, 0.0f};

    Skel_AddBone(&skel, "bone0", -1, pos, rot, scale);
    Skel_AddBone(&skel, "bone1", -1, pos, rot, scale);
    Skel_CalculateInverseBindPose(&skel);

    /* Create mesh with one vertex affected by both bones equally */
    skinned_mesh_t mesh;
    vec3_t bindPos = {0.0f, 0.0f, 0.0f};
    vertex_skin_t skinData;

    mesh.numVertices = 1;
    mesh.bindPosePositions = &bindPos;
    mesh.bindPoseNormals = NULL;
    mesh.skinData = &skinData;

    skinData.numWeights = 2;
    skinData.weights[0].boneIndex = 0;
    skinData.weights[0].weight = 0.5f;
    skinData.weights[1].boneIndex = 1;
    skinData.weights[1].weight = 0.5f;

    /* Bone 0 moves to (10,0,0), Bone 1 moves to (0,10,0) */
    bone_keyframe_t pose[2];
    vec3_t pos0 = {10.0f, 0.0f, 0.0f};
    vec3_t pos1 = {0.0f, 10.0f, 0.0f};

    VectorCopy(pos0, pose[0].position);
    QuatCopy(rot, pose[0].rotation);
    VectorCopy(scale, pose[0].scale);

    VectorCopy(pos1, pose[1].position);
    QuatCopy(rot, pose[1].rotation);
    VectorCopy(scale, pose[1].scale);

    mat4_t boneMatrices[2];
    Skel_CalculateWorldTransforms(&skel, pose, boneMatrices);

    vec3_t outPositions[1];
    Skin_TransformVertices(&mesh, boneMatrices, &skel, outPositions, NULL);

    /* Expected: average of (10,0,0) and (0,10,0) = (5,5,0) */
    vec3_t expected = {5.0f, 5.0f, 0.0f};
    ASSERT(vec3_eq(outPositions[0], expected),
           "Two bones with equal weights should average positions");
    PASS();
}

TEST(skinning_rotation)
{
    skeleton_t skel;
    Skel_Init(&skel);

    vec4_t rot = {0.0f, 0.0f, 0.0f, 1.0f};
    vec3_t scale = {1.0f, 1.0f, 1.0f};
    vec3_t pos = {0.0f, 0.0f, 0.0f};

    Skel_AddBone(&skel, "root", -1, pos, rot, scale);
    Skel_CalculateInverseBindPose(&skel);

    /* Vertex at (1, 0, 0) */
    skinned_mesh_t mesh;
    vec3_t bindPos = {1.0f, 0.0f, 0.0f};
    vertex_skin_t skinData;

    mesh.numVertices = 1;
    mesh.bindPosePositions = &bindPos;
    mesh.bindPoseNormals = NULL;
    mesh.skinData = &skinData;

    skinData.numWeights = 1;
    skinData.weights[0].boneIndex = 0;
    skinData.weights[0].weight = 1.0f;

    /* Rotate 90 degrees around Y axis */
    bone_keyframe_t pose[1];
    vec3_t axis = {0.0f, 1.0f, 0.0f};
    vec4_t newRot;
    Quat_FromAxisAngle(axis, (float)M_PI / 2.0f, newRot);

    VectorCopy(pos, pose[0].position);
    QuatCopy(newRot, pose[0].rotation);
    VectorCopy(scale, pose[0].scale);

    mat4_t boneMatrices[1];
    Skel_CalculateWorldTransforms(&skel, pose, boneMatrices);

    vec3_t outPositions[1];
    Skin_TransformVertices(&mesh, boneMatrices, &skel, outPositions, NULL);

    /* Vertex at (1,0,0) rotated 90 degrees around Y should be at (0,0,-1) */
    vec3_t expected = {0.0f, 0.0f, -1.0f};
    ASSERT(vec3_eq(outPositions[0], expected),
           "Rotation should transform vertex correctly");
    PASS();
}

/* ============== INTEGRATION TESTS ============== */

TEST(integration_arm_rotation)
{
    /* Test a 2-bone arm where the upper arm rotates */
    skeleton_t skel;
    Skel_Init(&skel);

    vec4_t rot = {0.0f, 0.0f, 0.0f, 1.0f};
    vec3_t scale = {1.0f, 1.0f, 1.0f};

    /* Upper arm at origin, lower arm 5 units along X */
    vec3_t upperPos = {0.0f, 0.0f, 0.0f};
    vec3_t lowerPos = {5.0f, 0.0f, 0.0f};

    Skel_AddBone(&skel, "upper", -1, upperPos, rot, scale);
    Skel_AddBone(&skel, "lower", 0, lowerPos, rot, scale);
    Skel_CalculateInverseBindPose(&skel);

    /* Vertex at the end of lower arm (10,0,0 in world space) */
    skinned_mesh_t mesh;
    vec3_t bindPos = {10.0f, 0.0f, 0.0f};
    vertex_skin_t skinData;

    mesh.numVertices = 1;
    mesh.bindPosePositions = &bindPos;
    mesh.bindPoseNormals = NULL;
    mesh.skinData = &skinData;

    skinData.numWeights = 1;
    skinData.weights[0].boneIndex = 1;  /* Attached to lower arm */
    skinData.weights[0].weight = 1.0f;

    /* Rotate upper arm 90 degrees around Z (raising the arm) */
    bone_keyframe_t pose[2];
    vec3_t axis = {0.0f, 0.0f, 1.0f};
    vec4_t newRot;
    Quat_FromAxisAngle(axis, (float)M_PI / 2.0f, newRot);

    VectorCopy(upperPos, pose[0].position);
    QuatCopy(newRot, pose[0].rotation);
    VectorCopy(scale, pose[0].scale);

    VectorCopy(lowerPos, pose[1].position);
    QuatCopy(rot, pose[1].rotation);  /* Lower arm stays straight */
    VectorCopy(scale, pose[1].scale);

    mat4_t boneMatrices[2];
    Skel_CalculateWorldTransforms(&skel, pose, boneMatrices);

    vec3_t outPositions[1];
    Skin_TransformVertices(&mesh, boneMatrices, &skel, outPositions, NULL);

    /* Arm raised 90 degrees: (10,0,0) should become (0,10,0) */
    vec3_t expected = {0.0f, 10.0f, 0.0f};
    ASSERT(vec3_eq(outPositions[0], expected),
           "Rotating upper arm should move lower arm vertex");
    PASS();
}

/* ============== MAIN ============== */

int main(void)
{
    printf("===========================================\n");
#ifdef TEST_SOLUTION
    printf("Skeletal Animation Tests (SOLUTION VERSION)\n");
#else
    printf("Skeletal Animation Tests (GAME VERSION)\n");
#endif
    printf("===========================================\n\n");

    /* Quaternion tests */
    printf("--- Quaternion Tests ---\n");
    run_test_quat_identity();
    run_test_quat_normalize();
    run_test_quat_multiply_identity();
    run_test_quat_multiply_inverse();
    run_test_quat_slerp_endpoints();
    run_test_quat_slerp_halfway();
    run_test_quat_slerp_shortest_path();
    run_test_quat_to_matrix_identity();
    run_test_quat_to_matrix_90_y();

    /* Matrix tests */
    printf("\n--- Matrix Tests ---\n");
    run_test_mat4_identity();
    run_test_mat4_multiply_identity();
    run_test_mat4_invert();
    run_test_mat4_transform_point();

    /* Skeleton tests */
    printf("\n--- Skeleton Tests ---\n");
    run_test_skeleton_init();
    run_test_skeleton_add_bone();
    run_test_skeleton_hierarchy();
    run_test_skeleton_world_transforms_identity();
    run_test_skeleton_world_transforms_translation();
    run_test_skeleton_hierarchy_transforms();

    /* Animation tests */
    printf("\n--- Animation Tests ---\n");
    run_test_animation_sample_first_frame();
    run_test_animation_sample_interpolation();
    run_test_animation_blend_poses();
    run_test_animation_blend_rotation();

    /* Skinning tests */
    printf("\n--- Skinning Tests ---\n");
    run_test_skinning_single_bone();
    run_test_skinning_two_bones_equal_weight();
    run_test_skinning_rotation();

    /* Integration tests */
    printf("\n--- Integration Tests ---\n");
    run_test_integration_arm_rotation();

    /* Summary */
    printf("\n===========================================\n");
    printf("RESULTS: %d passed, %d failed\n", tests_passed, tests_failed);
    printf("===========================================\n");

    return (tests_failed > 0) ? 1 : 0;
}
