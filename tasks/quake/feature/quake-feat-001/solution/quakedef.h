/*
 * quakedef.h - Quake type definitions for skeletal animation system
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
typedef vec_t vec4_t[4];  /* Quaternion: x, y, z, w */
typedef vec_t mat4_t[4][4];
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

/* Quaternion macros */
#define QuatCopy(a, b) ((b)[0] = (a)[0], (b)[1] = (a)[1], (b)[2] = (a)[2], (b)[3] = (a)[3])
#define QuatDot(a, b) ((a)[0] * (b)[0] + (a)[1] * (b)[1] + (a)[2] * (b)[2] + (a)[3] * (b)[3])

/* Constants */
#define MAX_BONES           128
#define MAX_BONE_NAME       32
#define MAX_BONES_PER_VERTEX 4
#define MAX_ANIMATION_FRAMES 256
#define MAX_VERTICES        4096

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ============== SKELETAL ANIMATION TYPES ============== */

/*
 * bone_t - A single bone in the skeleton hierarchy
 *
 * Bones form a tree structure where each bone's transform is relative
 * to its parent. The root bone(s) have parent = -1.
 */
typedef struct bone_s {
    char        name[MAX_BONE_NAME];
    int         parent;         /* Index of parent bone, -1 for root */
    vec3_t      localPos;       /* Position relative to parent (bind pose) */
    vec4_t      localRot;       /* Quaternion rotation relative to parent (bind pose) */
    vec3_t      localScale;     /* Scale relative to parent (usually 1,1,1) */
} bone_t;

/*
 * bone_keyframe_t - Animation data for a single bone at a single time
 */
typedef struct {
    vec3_t      position;
    vec4_t      rotation;       /* Quaternion */
    vec3_t      scale;
} bone_keyframe_t;

/*
 * animation_clip_t - A complete animation (e.g., "walk", "run", "attack")
 */
typedef struct {
    char        name[MAX_BONE_NAME];
    int         numFrames;
    float       fps;
    int         numBones;
    bone_keyframe_t *frames;    /* Array of [numFrames * numBones] */
} animation_clip_t;

/*
 * skeleton_t - The bone hierarchy definition
 */
typedef struct {
    int         numBones;
    bone_t      bones[MAX_BONES];
    mat4_t      inverseBindPose[MAX_BONES];  /* For skinning */
} skeleton_t;

/*
 * vertex_weight_t - Influence of one bone on a vertex
 */
typedef struct {
    int         boneIndex;
    float       weight;         /* 0.0 - 1.0 */
} vertex_weight_t;

/*
 * vertex_skin_t - All bone influences for one vertex
 */
typedef struct {
    vertex_weight_t weights[MAX_BONES_PER_VERTEX];
    int         numWeights;
} vertex_skin_t;

/*
 * skinned_mesh_t - A mesh with skeletal animation data
 */
typedef struct {
    int         numVertices;
    vec3_t      *bindPosePositions;  /* Vertex positions in bind pose */
    vec3_t      *bindPoseNormals;    /* Vertex normals in bind pose */
    vertex_skin_t *skinData;         /* Per-vertex bone weights */
} skinned_mesh_t;

/*
 * animation_state_t - Current animation playback state
 */
typedef struct {
    animation_clip_t *clip;
    float       time;           /* Current time in seconds */
    float       speed;          /* Playback speed multiplier */
    qboolean    looping;
} animation_state_t;

/*
 * animation_blend_t - For blending two animations
 */
typedef struct {
    animation_state_t stateA;
    animation_state_t stateB;
    float       blendFactor;    /* 0.0 = all A, 1.0 = all B */
} animation_blend_t;

/* ============== FUNCTION DECLARATIONS ============== */

/* Quaternion math */
void Quat_Identity(vec4_t out);
void Quat_Normalize(vec4_t q);
void Quat_Multiply(vec4_t a, vec4_t b, vec4_t out);
void Quat_Slerp(vec4_t a, vec4_t b, float t, vec4_t out);
void Quat_ToMatrix(vec4_t q, mat4_t out);
void Quat_FromAxisAngle(vec3_t axis, float angle, vec4_t out);

/* Matrix operations */
void Mat4_Identity(mat4_t out);
void Mat4_Multiply(mat4_t a, mat4_t b, mat4_t out);
void Mat4_Invert(mat4_t m, mat4_t out);
void Mat4_TransformPoint(mat4_t m, vec3_t in, vec3_t out);
void Mat4_TransformVector(mat4_t m, vec3_t in, vec3_t out);
void Mat4_FromPosRotScale(vec3_t pos, vec4_t rot, vec3_t scale, mat4_t out);

/* Skeleton operations */
void Skel_Init(skeleton_t *skel);
void Skel_AddBone(skeleton_t *skel, const char *name, int parent,
                  vec3_t pos, vec4_t rot, vec3_t scale);
void Skel_CalculateInverseBindPose(skeleton_t *skel);
void Skel_CalculateWorldTransforms(skeleton_t *skel, bone_keyframe_t *pose,
                                   mat4_t *outMatrices);

/* Animation operations */
void Anim_SamplePose(animation_clip_t *clip, float time, bone_keyframe_t *outPose);
void Anim_BlendPoses(bone_keyframe_t *poseA, bone_keyframe_t *poseB,
                     int numBones, float t, bone_keyframe_t *outPose);

/* Skinning */
void Skin_TransformVertices(skinned_mesh_t *mesh, mat4_t *boneMatrices,
                            skeleton_t *skel, vec3_t *outPositions, vec3_t *outNormals);

#endif /* QUAKEDEF_H */
