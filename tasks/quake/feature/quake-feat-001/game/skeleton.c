/*
 * skeleton.c - Skeletal Animation System (INCOMPLETE)
 *
 * This file contains stub implementations that need to be completed.
 * The skeletal animation system should:
 * 1. Handle quaternion math for rotations (SLERP interpolation)
 * 2. Build bone hierarchy transforms
 * 3. Sample animations at arbitrary times
 * 4. Blend between animations
 * 5. Apply skinning to transform vertices
 *
 * TASK: Implement the TODO sections in each function.
 */

#include "quakedef.h"
#include <stdio.h>
#include <stdlib.h>

/* ============== QUATERNION OPERATIONS ============== */

/*
 * Quat_Identity - Set quaternion to identity (no rotation)
 * Identity quaternion is (0, 0, 0, 1)
 */
void Quat_Identity(vec4_t out)
{
    out[0] = 0.0f;
    out[1] = 0.0f;
    out[2] = 0.0f;
    out[3] = 1.0f;
}

/*
 * Quat_Normalize - Normalize quaternion to unit length
 *
 * A unit quaternion has length 1.0. This is required for
 * proper rotation representation.
 */
void Quat_Normalize(vec4_t q)
{
    float len = sqrtf(q[0]*q[0] + q[1]*q[1] + q[2]*q[2] + q[3]*q[3]);
    if (len > 0.0001f) {
        float invLen = 1.0f / len;
        q[0] *= invLen;
        q[1] *= invLen;
        q[2] *= invLen;
        q[3] *= invLen;
    }
}

/*
 * Quat_Multiply - Multiply two quaternions
 *
 * Quaternion multiplication combines rotations.
 * Result = a * b (apply b first, then a)
 *
 * TODO: Implement quaternion multiplication using the formula:
 *   out.x = a.w*b.x + a.x*b.w + a.y*b.z - a.z*b.y
 *   out.y = a.w*b.y - a.x*b.z + a.y*b.w + a.z*b.x
 *   out.z = a.w*b.z + a.x*b.y - a.y*b.x + a.z*b.w
 *   out.w = a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z
 */
void Quat_Multiply(vec4_t a, vec4_t b, vec4_t out)
{
    /* TODO: Implement quaternion multiplication */
    /* STUB: Currently returns identity - incorrect! */
    out[0] = 0.0f;
    out[1] = 0.0f;
    out[2] = 0.0f;
    out[3] = 1.0f;
}

/*
 * Quat_Slerp - Spherical Linear Interpolation between quaternions
 *
 * Smoothly interpolates between two rotations.
 * t = 0.0 returns 'a', t = 1.0 returns 'b'
 *
 * TODO: Implement SLERP:
 * 1. Calculate dot product of a and b
 * 2. If dot < 0, negate b to take shorter path
 * 3. If dot > 0.9995, use linear interpolation (nearly identical)
 * 4. Otherwise, use spherical interpolation formula
 */
void Quat_Slerp(vec4_t a, vec4_t b, float t, vec4_t out)
{
    /* TODO: Implement SLERP */
    /* STUB: Currently does linear interpolation - incorrect for large angles! */
    out[0] = a[0] + (b[0] - a[0]) * t;
    out[1] = a[1] + (b[1] - a[1]) * t;
    out[2] = a[2] + (b[2] - a[2]) * t;
    out[3] = a[3] + (b[3] - a[3]) * t;
    Quat_Normalize(out);
}

/*
 * Quat_ToMatrix - Convert quaternion to 4x4 rotation matrix
 *
 * TODO: Implement conversion using the standard formula.
 * The matrix should be column-major for OpenGL compatibility.
 */
void Quat_ToMatrix(vec4_t q, mat4_t out)
{
    /* TODO: Implement quaternion to matrix conversion */
    /* STUB: Returns identity matrix - incorrect! */
    Mat4_Identity(out);
}

/*
 * Quat_FromAxisAngle - Create quaternion from axis and angle
 *
 * axis: Unit vector defining rotation axis
 * angle: Rotation angle in radians
 */
void Quat_FromAxisAngle(vec3_t axis, float angle, vec4_t out)
{
    float halfAngle = angle * 0.5f;
    float s = sinf(halfAngle);
    out[0] = axis[0] * s;
    out[1] = axis[1] * s;
    out[2] = axis[2] * s;
    out[3] = cosf(halfAngle);
}

/* ============== MATRIX OPERATIONS ============== */

/*
 * Mat4_Identity - Set matrix to identity
 */
void Mat4_Identity(mat4_t out)
{
    memset(out, 0, sizeof(mat4_t));
    out[0][0] = 1.0f;
    out[1][1] = 1.0f;
    out[2][2] = 1.0f;
    out[3][3] = 1.0f;
}

/*
 * Mat4_Multiply - Multiply two 4x4 matrices
 */
void Mat4_Multiply(mat4_t a, mat4_t b, mat4_t out)
{
    mat4_t temp;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            temp[i][j] = a[i][0] * b[0][j] +
                         a[i][1] * b[1][j] +
                         a[i][2] * b[2][j] +
                         a[i][3] * b[3][j];
        }
    }
    memcpy(out, temp, sizeof(mat4_t));
}

/*
 * Mat4_Invert - Invert a 4x4 matrix (assumes affine transform)
 *
 * For affine transforms (rotation + translation + scale),
 * we can use a simplified inversion method.
 *
 * TODO: Implement matrix inversion for affine transforms.
 */
void Mat4_Invert(mat4_t m, mat4_t out)
{
    /* TODO: Implement proper matrix inversion */
    /* STUB: Returns identity - incorrect! */
    Mat4_Identity(out);
}

/*
 * Mat4_TransformPoint - Transform a point by a matrix
 *
 * Applies full 4x4 transform including translation.
 */
void Mat4_TransformPoint(mat4_t m, vec3_t in, vec3_t out)
{
    vec3_t temp;
    temp[0] = m[0][0] * in[0] + m[0][1] * in[1] + m[0][2] * in[2] + m[0][3];
    temp[1] = m[1][0] * in[0] + m[1][1] * in[1] + m[1][2] * in[2] + m[1][3];
    temp[2] = m[2][0] * in[0] + m[2][1] * in[1] + m[2][2] * in[2] + m[2][3];
    VectorCopy(temp, out);
}

/*
 * Mat4_TransformVector - Transform a direction vector by a matrix
 *
 * Applies rotation/scale only (no translation).
 * Used for normals and directions.
 */
void Mat4_TransformVector(mat4_t m, vec3_t in, vec3_t out)
{
    vec3_t temp;
    temp[0] = m[0][0] * in[0] + m[0][1] * in[1] + m[0][2] * in[2];
    temp[1] = m[1][0] * in[0] + m[1][1] * in[1] + m[1][2] * in[2];
    temp[2] = m[2][0] * in[0] + m[2][1] * in[1] + m[2][2] * in[2];
    VectorCopy(temp, out);
}

/*
 * Mat4_FromPosRotScale - Build matrix from position, rotation, and scale
 */
void Mat4_FromPosRotScale(vec3_t pos, vec4_t rot, vec3_t scale, mat4_t out)
{
    /* Convert quaternion to rotation matrix */
    Quat_ToMatrix(rot, out);

    /* Apply scale to rotation part */
    out[0][0] *= scale[0]; out[0][1] *= scale[0]; out[0][2] *= scale[0];
    out[1][0] *= scale[1]; out[1][1] *= scale[1]; out[1][2] *= scale[1];
    out[2][0] *= scale[2]; out[2][1] *= scale[2]; out[2][2] *= scale[2];

    /* Set translation */
    out[0][3] = pos[0];
    out[1][3] = pos[1];
    out[2][3] = pos[2];
}

/* ============== SKELETON OPERATIONS ============== */

/*
 * Skel_Init - Initialize an empty skeleton
 */
void Skel_Init(skeleton_t *skel)
{
    skel->numBones = 0;
    memset(skel->bones, 0, sizeof(skel->bones));
}

/*
 * Skel_AddBone - Add a bone to the skeleton
 */
void Skel_AddBone(skeleton_t *skel, const char *name, int parent,
                  vec3_t pos, vec4_t rot, vec3_t scale)
{
    if (skel->numBones >= MAX_BONES) {
        return;
    }

    bone_t *bone = &skel->bones[skel->numBones];
    strncpy(bone->name, name, MAX_BONE_NAME - 1);
    bone->parent = parent;
    VectorCopy(pos, bone->localPos);
    QuatCopy(rot, bone->localRot);
    VectorCopy(scale, bone->localScale);

    skel->numBones++;
}

/*
 * Skel_CalculateInverseBindPose - Calculate inverse bind pose matrices
 *
 * The inverse bind pose is needed to transform vertices from
 * bind pose (T-pose) to bone-local space before applying animation.
 *
 * TODO: Implement this function:
 * 1. Calculate world transform for each bone in bind pose
 * 2. Invert each world transform to get inverse bind pose
 */
void Skel_CalculateInverseBindPose(skeleton_t *skel)
{
    /* TODO: Calculate inverse bind pose matrices */
    /* STUB: Sets all to identity - incorrect! */
    for (int i = 0; i < skel->numBones; i++) {
        Mat4_Identity(skel->inverseBindPose[i]);
    }
}

/*
 * Skel_CalculateWorldTransforms - Calculate world-space bone matrices
 *
 * Given a pose (array of bone_keyframe_t), calculate the world-space
 * transform matrix for each bone.
 *
 * TODO: Implement this function:
 * 1. Process bones in parent-first order (parents have lower indices)
 * 2. For each bone:
 *    a. Build local transform matrix from pose
 *    b. If has parent, multiply by parent's world transform
 *    c. Store result in outMatrices[boneIndex]
 * 3. For skinning, multiply each result by inverse bind pose
 */
void Skel_CalculateWorldTransforms(skeleton_t *skel, bone_keyframe_t *pose,
                                   mat4_t *outMatrices)
{
    /* TODO: Implement world transform calculation */
    /* STUB: Sets all to identity - incorrect! */
    for (int i = 0; i < skel->numBones; i++) {
        Mat4_Identity(outMatrices[i]);
    }
}

/* ============== ANIMATION OPERATIONS ============== */

/*
 * Anim_SamplePose - Sample an animation at a specific time
 *
 * Interpolates between keyframes to get the pose at 'time'.
 *
 * TODO: Implement this function:
 * 1. Convert time to frame index (float)
 * 2. Get keyframes before and after
 * 3. Calculate interpolation factor
 * 4. For each bone:
 *    a. LERP position
 *    b. SLERP rotation
 *    c. LERP scale
 */
void Anim_SamplePose(animation_clip_t *clip, float time, bone_keyframe_t *outPose)
{
    if (!clip || !clip->frames || clip->numFrames == 0) {
        return;
    }

    /* TODO: Implement proper animation sampling with interpolation */
    /* STUB: Just copies first frame - no interpolation! */
    for (int i = 0; i < clip->numBones; i++) {
        VectorCopy(clip->frames[i].position, outPose[i].position);
        QuatCopy(clip->frames[i].rotation, outPose[i].rotation);
        VectorCopy(clip->frames[i].scale, outPose[i].scale);
    }
}

/*
 * Anim_BlendPoses - Blend between two poses
 *
 * t = 0.0 returns poseA, t = 1.0 returns poseB
 *
 * TODO: Implement pose blending:
 * 1. For each bone:
 *    a. LERP positions
 *    b. SLERP rotations
 *    c. LERP scales
 */
void Anim_BlendPoses(bone_keyframe_t *poseA, bone_keyframe_t *poseB,
                     int numBones, float t, bone_keyframe_t *outPose)
{
    /* TODO: Implement pose blending */
    /* STUB: Just copies poseA - no blending! */
    for (int i = 0; i < numBones; i++) {
        VectorCopy(poseA[i].position, outPose[i].position);
        QuatCopy(poseA[i].rotation, outPose[i].rotation);
        VectorCopy(poseA[i].scale, outPose[i].scale);
    }

    (void)poseB;  /* Unused in stub */
    (void)t;      /* Unused in stub */
}

/* ============== SKINNING ============== */

/*
 * Skin_TransformVertices - Apply skeletal deformation to mesh vertices
 *
 * This is the core skinning function that transforms vertices from
 * bind pose to animated pose using bone influences.
 *
 * TODO: Implement vertex skinning:
 * 1. For each vertex:
 *    a. Start with zero position/normal
 *    b. For each bone weight:
 *       - Transform bind pose position by bone's skinning matrix
 *       - Multiply by weight
 *       - Accumulate into output position
 *    c. Normalize output normal
 *
 * The skinning matrix for each bone = boneWorldMatrix * inverseBindPose
 */
void Skin_TransformVertices(skinned_mesh_t *mesh, mat4_t *boneMatrices,
                            skeleton_t *skel, vec3_t *outPositions, vec3_t *outNormals)
{
    /* TODO: Implement vertex skinning */
    /* STUB: Just copies bind pose - no skinning applied! */
    for (int i = 0; i < mesh->numVertices; i++) {
        VectorCopy(mesh->bindPosePositions[i], outPositions[i]);
        if (outNormals && mesh->bindPoseNormals) {
            VectorCopy(mesh->bindPoseNormals[i], outNormals[i]);
        }
    }

    (void)boneMatrices;  /* Unused in stub */
    (void)skel;          /* Unused in stub */
}
