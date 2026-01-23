/*
 * r_alias.c - Alias model rendering (INCOMPLETE - needs skeletal integration)
 *
 * This file shows Quake's alias model rendering with hooks for skeletal
 * animation. The skeletal animation functions in skeleton.c need to be
 * completed for this to work correctly.
 *
 * Original Quake uses vertex animation (morph targets). This version
 * adds support for skeletal animation when model data includes bones.
 */

#include "quakedef.h"
#include <stdio.h>
#include <stdlib.h>

/* ============== ALIAS MODEL STRUCTURES ============== */

/* Original Quake vertex structure (compressed) */
typedef struct {
    unsigned char v[3];     /* Scaled and offset position */
    unsigned char normalIndex;
} trivertx_t;

/* Transformed vertex for rendering */
typedef struct {
    float v[3];             /* World-space position */
    float n[3];             /* World-space normal */
} finalvert_t;

/* Alias model frame (morph target) */
typedef struct {
    trivertx_t *verts;
    char name[16];
} aliasframe_t;

/* Complete alias model */
typedef struct {
    int numVerts;
    int numFrames;
    aliasframe_t *frames;

    /* Skeletal animation extension (optional) */
    qboolean hasSkeleton;
    skeleton_t skeleton;
    skinned_mesh_t skinnedMesh;
    animation_clip_t *clips;
    int numClips;
} aliasmodel_t;

/* Current entity being rendered */
typedef struct {
    aliasmodel_t *model;
    int frame;              /* Current frame (for morph) */
    int prevFrame;          /* Previous frame (for interpolation) */
    float lerpFrac;         /* Interpolation factor */

    /* Skeletal animation state */
    int clipIndex;          /* Current animation clip */
    float animTime;         /* Current time in animation */
    int blendClipIndex;     /* Blend target clip (-1 if none) */
    float blendFactor;      /* Blend factor (0-1) */
} entity_state_t;

/* ============== MORPH ANIMATION (ORIGINAL QUAKE) ============== */

/*
 * R_AliasTransformVertex_Morph - Transform vertex using morph targets
 *
 * Interpolates between two frames using linear interpolation.
 */
static void R_AliasTransformVertex_Morph(aliasmodel_t *model,
                                         int frame1, int frame2,
                                         float lerp, int vertIndex,
                                         finalvert_t *out)
{
    trivertx_t *v1 = &model->frames[frame1].verts[vertIndex];
    trivertx_t *v2 = &model->frames[frame2].verts[vertIndex];

    /* Decompress and interpolate position */
    float scale = 1.0f / 16.0f;  /* Quake's compression scale */
    out->v[0] = (v1->v[0] + (v2->v[0] - v1->v[0]) * lerp) * scale;
    out->v[1] = (v1->v[1] + (v2->v[1] - v1->v[1]) * lerp) * scale;
    out->v[2] = (v1->v[2] + (v2->v[2] - v1->v[2]) * lerp) * scale;

    /* Normal would come from a lookup table - simplified here */
    out->n[0] = 0.0f;
    out->n[1] = 0.0f;
    out->n[2] = 1.0f;
}

/*
 * R_AliasDrawModel_Morph - Render using morph target animation
 */
static void R_AliasDrawModel_Morph(entity_state_t *ent, finalvert_t *outVerts)
{
    aliasmodel_t *model = ent->model;

    for (int i = 0; i < model->numVerts; i++) {
        R_AliasTransformVertex_Morph(model,
                                     ent->prevFrame, ent->frame,
                                     ent->lerpFrac, i,
                                     &outVerts[i]);
    }
}

/* ============== SKELETAL ANIMATION (NEW) ============== */

/*
 * R_AliasDrawModel_Skeletal - Render using skeletal animation
 *
 * This function orchestrates the skeletal animation pipeline:
 * 1. Sample the current animation pose
 * 2. Optionally blend with another animation
 * 3. Calculate bone matrices
 * 4. Transform vertices using skinning
 *
 * NOTE: This relies on the functions in skeleton.c being correctly
 * implemented. If those are stubs, the output will be incorrect.
 */
static void R_AliasDrawModel_Skeletal(entity_state_t *ent, finalvert_t *outVerts)
{
    aliasmodel_t *model = ent->model;
    skeleton_t *skel = &model->skeleton;
    skinned_mesh_t *mesh = &model->skinnedMesh;

    /* Allocate temporary storage */
    bone_keyframe_t pose[MAX_BONES];
    bone_keyframe_t blendPose[MAX_BONES];
    mat4_t boneMatrices[MAX_BONES];
    vec3_t positions[MAX_VERTICES];
    vec3_t normals[MAX_VERTICES];

    /* Sample current animation */
    animation_clip_t *clip = &model->clips[ent->clipIndex];
    Anim_SamplePose(clip, ent->animTime, pose);

    /* Blend with secondary animation if active */
    if (ent->blendClipIndex >= 0 && ent->blendFactor > 0.0f) {
        animation_clip_t *blendClip = &model->clips[ent->blendClipIndex];
        Anim_SamplePose(blendClip, ent->animTime, blendPose);
        Anim_BlendPoses(pose, blendPose, skel->numBones, ent->blendFactor, pose);
    }

    /* Calculate bone matrices for skinning */
    Skel_CalculateWorldTransforms(skel, pose, boneMatrices);

    /* Transform vertices */
    Skin_TransformVertices(mesh, boneMatrices, skel, positions, normals);

    /* Copy to output format */
    for (int i = 0; i < mesh->numVertices; i++) {
        VectorCopy(positions[i], outVerts[i].v);
        VectorCopy(normals[i], outVerts[i].n);
    }
}

/* ============== PUBLIC API ============== */

/*
 * R_AliasDrawModel - Main entry point for alias model rendering
 *
 * Automatically selects morph or skeletal animation based on model data.
 */
void R_AliasDrawModel(entity_state_t *ent, finalvert_t *outVerts)
{
    if (!ent || !ent->model || !outVerts) {
        return;
    }

    if (ent->model->hasSkeleton) {
        R_AliasDrawModel_Skeletal(ent, outVerts);
    } else {
        R_AliasDrawModel_Morph(ent, outVerts);
    }
}

/*
 * R_SetupSkeletalModel - Initialize skeletal animation for a model
 *
 * Call this after loading a model with skeleton data.
 */
void R_SetupSkeletalModel(aliasmodel_t *model)
{
    if (!model->hasSkeleton) {
        return;
    }

    /* Calculate inverse bind pose matrices */
    Skel_CalculateInverseBindPose(&model->skeleton);
}

/*
 * R_UpdateAnimation - Update animation state for an entity
 *
 * Call this each frame to advance animation time.
 */
void R_UpdateAnimation(entity_state_t *ent, float deltaTime)
{
    if (!ent || !ent->model || !ent->model->hasSkeleton) {
        return;
    }

    ent->animTime += deltaTime;

    /* Handle animation looping */
    animation_clip_t *clip = &ent->model->clips[ent->clipIndex];
    float duration = (float)(clip->numFrames - 1) / clip->fps;
    while (ent->animTime >= duration) {
        ent->animTime -= duration;
    }
}

/*
 * R_StartAnimationBlend - Begin blending to a new animation
 */
void R_StartAnimationBlend(entity_state_t *ent, int newClipIndex, float blendTime)
{
    if (!ent || newClipIndex < 0 || newClipIndex >= ent->model->numClips) {
        return;
    }

    ent->blendClipIndex = newClipIndex;
    ent->blendFactor = 0.0f;

    (void)blendTime;  /* TODO: Use this to control blend speed */
}
