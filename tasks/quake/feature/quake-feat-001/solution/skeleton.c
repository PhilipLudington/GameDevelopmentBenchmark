/*
 * skeleton.c - Skeletal Animation System (COMPLETE SOLUTION)
 *
 * This file contains the complete, working implementation of
 * the skeletal animation system for Quake.
 */

#include "quakedef.h"
#include <stdio.h>
#include <stdlib.h>

/* ============== QUATERNION OPERATIONS ============== */

/*
 * Quat_Identity - Set quaternion to identity (no rotation)
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
 * Quat_Multiply - Multiply two quaternions (Hamilton product)
 *
 * Result represents rotation a followed by rotation b.
 * Uses the standard quaternion multiplication formula.
 */
void Quat_Multiply(vec4_t a, vec4_t b, vec4_t out)
{
    vec4_t temp;
    temp[0] = a[3]*b[0] + a[0]*b[3] + a[1]*b[2] - a[2]*b[1];
    temp[1] = a[3]*b[1] - a[0]*b[2] + a[1]*b[3] + a[2]*b[0];
    temp[2] = a[3]*b[2] + a[0]*b[1] - a[1]*b[0] + a[2]*b[3];
    temp[3] = a[3]*b[3] - a[0]*b[0] - a[1]*b[1] - a[2]*b[2];
    QuatCopy(temp, out);
}

/*
 * Quat_Slerp - Spherical Linear Interpolation between quaternions
 *
 * Smoothly interpolates rotation from a to b.
 * t = 0.0 returns a, t = 1.0 returns b
 */
void Quat_Slerp(vec4_t a, vec4_t b, float t, vec4_t out)
{
    vec4_t bCopy;
    QuatCopy(b, bCopy);

    /* Calculate dot product (cosine of angle between quaternions) */
    float dot = QuatDot(a, bCopy);

    /* If dot < 0, negate one quaternion to take shorter path */
    if (dot < 0.0f) {
        bCopy[0] = -bCopy[0];
        bCopy[1] = -bCopy[1];
        bCopy[2] = -bCopy[2];
        bCopy[3] = -bCopy[3];
        dot = -dot;
    }

    /* If quaternions are nearly identical, use linear interpolation */
    if (dot > 0.9995f) {
        out[0] = a[0] + (bCopy[0] - a[0]) * t;
        out[1] = a[1] + (bCopy[1] - a[1]) * t;
        out[2] = a[2] + (bCopy[2] - a[2]) * t;
        out[3] = a[3] + (bCopy[3] - a[3]) * t;
        Quat_Normalize(out);
        return;
    }

    /* Spherical interpolation */
    float theta = acosf(dot);
    float sinTheta = sinf(theta);
    float wa = sinf((1.0f - t) * theta) / sinTheta;
    float wb = sinf(t * theta) / sinTheta;

    out[0] = wa * a[0] + wb * bCopy[0];
    out[1] = wa * a[1] + wb * bCopy[1];
    out[2] = wa * a[2] + wb * bCopy[2];
    out[3] = wa * a[3] + wb * bCopy[3];
}

/*
 * Quat_ToMatrix - Convert quaternion to 4x4 rotation matrix
 *
 * Uses the standard quaternion to rotation matrix formula.
 * Matrix is row-major with translation in [row][3].
 */
void Quat_ToMatrix(vec4_t q, mat4_t out)
{
    float xx = q[0] * q[0];
    float yy = q[1] * q[1];
    float zz = q[2] * q[2];
    float xy = q[0] * q[1];
    float xz = q[0] * q[2];
    float yz = q[1] * q[2];
    float wx = q[3] * q[0];
    float wy = q[3] * q[1];
    float wz = q[3] * q[2];

    /* Row 0 */
    out[0][0] = 1.0f - 2.0f * (yy + zz);
    out[0][1] = 2.0f * (xy - wz);
    out[0][2] = 2.0f * (xz + wy);
    out[0][3] = 0.0f;

    /* Row 1 */
    out[1][0] = 2.0f * (xy + wz);
    out[1][1] = 1.0f - 2.0f * (xx + zz);
    out[1][2] = 2.0f * (yz - wx);
    out[1][3] = 0.0f;

    /* Row 2 */
    out[2][0] = 2.0f * (xz - wy);
    out[2][1] = 2.0f * (yz + wx);
    out[2][2] = 1.0f - 2.0f * (xx + yy);
    out[2][3] = 0.0f;

    /* Row 3 */
    out[3][0] = 0.0f;
    out[3][1] = 0.0f;
    out[3][2] = 0.0f;
    out[3][3] = 1.0f;
}

/*
 * Quat_FromAxisAngle - Create quaternion from axis and angle
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
 * Mat4_Invert - Invert a 4x4 affine transformation matrix
 *
 * For affine transforms (rotation + translation + uniform scale),
 * we use a simplified inversion:
 * - Transpose the 3x3 rotation part
 * - Divide by scale squared
 * - Transform and negate the translation
 */
void Mat4_Invert(mat4_t m, mat4_t out)
{
    mat4_t temp;

    /* Calculate scale (assumes uniform scale per axis) */
    float scaleX = sqrtf(m[0][0]*m[0][0] + m[1][0]*m[1][0] + m[2][0]*m[2][0]);
    float scaleY = sqrtf(m[0][1]*m[0][1] + m[1][1]*m[1][1] + m[2][1]*m[2][1]);
    float scaleZ = sqrtf(m[0][2]*m[0][2] + m[1][2]*m[1][2] + m[2][2]*m[2][2]);

    float invScaleX = (scaleX > 0.0001f) ? 1.0f / scaleX : 0.0f;
    float invScaleY = (scaleY > 0.0001f) ? 1.0f / scaleY : 0.0f;
    float invScaleZ = (scaleZ > 0.0001f) ? 1.0f / scaleZ : 0.0f;

    /* Transpose and scale the rotation part */
    temp[0][0] = m[0][0] * invScaleX * invScaleX;
    temp[0][1] = m[1][0] * invScaleX * invScaleY;
    temp[0][2] = m[2][0] * invScaleX * invScaleZ;
    temp[0][3] = 0.0f;

    temp[1][0] = m[0][1] * invScaleY * invScaleX;
    temp[1][1] = m[1][1] * invScaleY * invScaleY;
    temp[1][2] = m[2][1] * invScaleY * invScaleZ;
    temp[1][3] = 0.0f;

    temp[2][0] = m[0][2] * invScaleZ * invScaleX;
    temp[2][1] = m[1][2] * invScaleZ * invScaleY;
    temp[2][2] = m[2][2] * invScaleZ * invScaleZ;
    temp[2][3] = 0.0f;

    /* Transform and negate translation */
    vec3_t trans = { m[0][3], m[1][3], m[2][3] };
    temp[0][3] = -(temp[0][0] * trans[0] + temp[0][1] * trans[1] + temp[0][2] * trans[2]);
    temp[1][3] = -(temp[1][0] * trans[0] + temp[1][1] * trans[1] + temp[1][2] * trans[2]);
    temp[2][3] = -(temp[2][0] * trans[0] + temp[2][1] * trans[1] + temp[2][2] * trans[2]);

    temp[3][0] = 0.0f;
    temp[3][1] = 0.0f;
    temp[3][2] = 0.0f;
    temp[3][3] = 1.0f;

    memcpy(out, temp, sizeof(mat4_t));
}

/*
 * Mat4_TransformPoint - Transform a point by a matrix
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
    out[0][0] *= scale[0]; out[0][1] *= scale[1]; out[0][2] *= scale[2];
    out[1][0] *= scale[0]; out[1][1] *= scale[1]; out[1][2] *= scale[2];
    out[2][0] *= scale[0]; out[2][1] *= scale[1]; out[2][2] *= scale[2];

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
 * The inverse bind pose transforms vertices from world space to bone space.
 */
void Skel_CalculateInverseBindPose(skeleton_t *skel)
{
    mat4_t worldTransforms[MAX_BONES];

    /* First, calculate world transforms for each bone in bind pose */
    for (int i = 0; i < skel->numBones; i++) {
        bone_t *bone = &skel->bones[i];
        mat4_t localTransform;

        /* Build local transform from bind pose */
        Mat4_FromPosRotScale(bone->localPos, bone->localRot, bone->localScale, localTransform);

        if (bone->parent < 0) {
            /* Root bone - local transform IS world transform */
            memcpy(worldTransforms[i], localTransform, sizeof(mat4_t));
        } else {
            /* Child bone - multiply by parent's world transform */
            Mat4_Multiply(worldTransforms[bone->parent], localTransform, worldTransforms[i]);
        }
    }

    /* Now invert each world transform */
    for (int i = 0; i < skel->numBones; i++) {
        Mat4_Invert(worldTransforms[i], skel->inverseBindPose[i]);
    }
}

/*
 * Skel_CalculateWorldTransforms - Calculate world-space bone matrices for skinning
 *
 * Given a pose, calculate the final skinning matrix for each bone.
 * Skinning matrix = worldTransform * inverseBindPose
 */
void Skel_CalculateWorldTransforms(skeleton_t *skel, bone_keyframe_t *pose,
                                   mat4_t *outMatrices)
{
    mat4_t worldTransforms[MAX_BONES];

    /* Calculate world transform for each bone */
    for (int i = 0; i < skel->numBones; i++) {
        bone_t *bone = &skel->bones[i];
        mat4_t localTransform;

        /* Build local transform from pose */
        Mat4_FromPosRotScale(pose[i].position, pose[i].rotation, pose[i].scale, localTransform);

        if (bone->parent < 0) {
            /* Root bone */
            memcpy(worldTransforms[i], localTransform, sizeof(mat4_t));
        } else {
            /* Child bone */
            Mat4_Multiply(worldTransforms[bone->parent], localTransform, worldTransforms[i]);
        }

        /* Multiply by inverse bind pose to get skinning matrix */
        Mat4_Multiply(worldTransforms[i], skel->inverseBindPose[i], outMatrices[i]);
    }
}

/* ============== ANIMATION OPERATIONS ============== */

/*
 * Anim_SamplePose - Sample an animation at a specific time
 *
 * Interpolates between keyframes to produce smooth animation.
 */
void Anim_SamplePose(animation_clip_t *clip, float time, bone_keyframe_t *outPose)
{
    if (!clip || !clip->frames || clip->numFrames == 0) {
        return;
    }

    /* Calculate frame position */
    float duration = (float)(clip->numFrames - 1) / clip->fps;
    float normalizedTime = fmodf(time, duration);
    if (normalizedTime < 0) normalizedTime += duration;

    float frameFloat = normalizedTime * clip->fps;
    int frame0 = (int)frameFloat;
    int frame1 = frame0 + 1;

    /* Clamp to valid range */
    if (frame0 >= clip->numFrames - 1) {
        frame0 = clip->numFrames - 1;
        frame1 = clip->numFrames - 1;
    }
    if (frame1 >= clip->numFrames) {
        frame1 = clip->numFrames - 1;
    }

    float t = frameFloat - (float)frame0;

    /* Interpolate each bone */
    for (int i = 0; i < clip->numBones; i++) {
        bone_keyframe_t *kf0 = &clip->frames[frame0 * clip->numBones + i];
        bone_keyframe_t *kf1 = &clip->frames[frame1 * clip->numBones + i];

        /* LERP position */
        outPose[i].position[0] = kf0->position[0] + (kf1->position[0] - kf0->position[0]) * t;
        outPose[i].position[1] = kf0->position[1] + (kf1->position[1] - kf0->position[1]) * t;
        outPose[i].position[2] = kf0->position[2] + (kf1->position[2] - kf0->position[2]) * t;

        /* SLERP rotation */
        Quat_Slerp(kf0->rotation, kf1->rotation, t, outPose[i].rotation);

        /* LERP scale */
        outPose[i].scale[0] = kf0->scale[0] + (kf1->scale[0] - kf0->scale[0]) * t;
        outPose[i].scale[1] = kf0->scale[1] + (kf1->scale[1] - kf0->scale[1]) * t;
        outPose[i].scale[2] = kf0->scale[2] + (kf1->scale[2] - kf0->scale[2]) * t;
    }
}

/*
 * Anim_BlendPoses - Blend between two poses
 *
 * Used for animation transitions and layered animation.
 */
void Anim_BlendPoses(bone_keyframe_t *poseA, bone_keyframe_t *poseB,
                     int numBones, float t, bone_keyframe_t *outPose)
{
    for (int i = 0; i < numBones; i++) {
        /* LERP position */
        outPose[i].position[0] = poseA[i].position[0] + (poseB[i].position[0] - poseA[i].position[0]) * t;
        outPose[i].position[1] = poseA[i].position[1] + (poseB[i].position[1] - poseA[i].position[1]) * t;
        outPose[i].position[2] = poseA[i].position[2] + (poseB[i].position[2] - poseA[i].position[2]) * t;

        /* SLERP rotation */
        Quat_Slerp(poseA[i].rotation, poseB[i].rotation, t, outPose[i].rotation);

        /* LERP scale */
        outPose[i].scale[0] = poseA[i].scale[0] + (poseB[i].scale[0] - poseA[i].scale[0]) * t;
        outPose[i].scale[1] = poseA[i].scale[1] + (poseB[i].scale[1] - poseA[i].scale[1]) * t;
        outPose[i].scale[2] = poseA[i].scale[2] + (poseB[i].scale[2] - poseA[i].scale[2]) * t;
    }
}

/* ============== SKINNING ============== */

/*
 * Skin_TransformVertices - Apply skeletal deformation to mesh vertices
 *
 * This is the core skinning function. For each vertex:
 * 1. Accumulate weighted transforms from all influencing bones
 * 2. Transform the bind pose position/normal by the accumulated matrix
 */
void Skin_TransformVertices(skinned_mesh_t *mesh, mat4_t *boneMatrices,
                            skeleton_t *skel, vec3_t *outPositions, vec3_t *outNormals)
{
    (void)skel;  /* Used for validation in debug builds */

    for (int v = 0; v < mesh->numVertices; v++) {
        vertex_skin_t *skin = &mesh->skinData[v];
        vec3_t bindPos, bindNormal;
        VectorCopy(mesh->bindPosePositions[v], bindPos);

        if (mesh->bindPoseNormals) {
            VectorCopy(mesh->bindPoseNormals[v], bindNormal);
        }

        /* Clear output */
        VectorClear(outPositions[v]);
        if (outNormals) {
            VectorClear(outNormals[v]);
        }

        /* Accumulate weighted transforms */
        for (int w = 0; w < skin->numWeights; w++) {
            int boneIdx = skin->weights[w].boneIndex;
            float weight = skin->weights[w].weight;

            if (weight <= 0.0f || boneIdx < 0) continue;

            /* Transform position by this bone's skinning matrix */
            vec3_t transformedPos;
            Mat4_TransformPoint(boneMatrices[boneIdx], bindPos, transformedPos);

            /* Accumulate weighted position */
            outPositions[v][0] += transformedPos[0] * weight;
            outPositions[v][1] += transformedPos[1] * weight;
            outPositions[v][2] += transformedPos[2] * weight;

            /* Transform normal if provided */
            if (outNormals && mesh->bindPoseNormals) {
                vec3_t transformedNormal;
                Mat4_TransformVector(boneMatrices[boneIdx], bindNormal, transformedNormal);
                outNormals[v][0] += transformedNormal[0] * weight;
                outNormals[v][1] += transformedNormal[1] * weight;
                outNormals[v][2] += transformedNormal[2] * weight;
            }
        }

        /* Normalize the output normal */
        if (outNormals && mesh->bindPoseNormals) {
            float len = VectorLength(outNormals[v]);
            if (len > 0.0001f) {
                float invLen = 1.0f / len;
                outNormals[v][0] *= invLen;
                outNormals[v][1] *= invLen;
                outNormals[v][2] *= invLen;
            }
        }
    }
}
