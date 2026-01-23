# Implement Skeletal Animation System

## Overview

Quake uses vertex animation (morph targets) where each animation frame stores complete vertex positions. This is memory-intensive and doesn't allow for animation blending. Implement a skeletal animation system that:

1. Defines a bone hierarchy
2. Stores per-bone transforms instead of per-vertex positions
3. Skinning calculates vertex positions from bone transforms
4. Enables smooth blending between animations

## Background: Quake's Current Animation System

```c
// Current: Morph target animation
typedef struct {
    trivertx_t  v[MAX_VERTS];  // All vertices for this frame
} aliasframe_t;

// Animation = sequence of complete mesh snapshots
// Blending = linear interpolation between two frames
void R_AliasTransformFinalVert(finalvert_t *fv, trivertx_t *v1, trivertx_t *v2, float blend)
{
    fv->v[0] = v1->v[0] + (v2->v[0] - v1->v[0]) * blend;
    fv->v[1] = v1->v[1] + (v2->v[1] - v1->v[1]) * blend;
    fv->v[2] = v1->v[2] + (v2->v[2] - v1->v[2]) * blend;
}
```

## Target: Skeletal Animation

```c
// Bone definition
typedef struct bone_s {
    char        name[32];
    int         parent;         // Index of parent bone, -1 for root
    vec3_t      localPos;       // Position relative to parent
    vec4_t      localRot;       // Quaternion rotation relative to parent
} bone_t;

// Animation keyframe for one bone
typedef struct {
    vec3_t      position;
    vec4_t      rotation;       // Quaternion
    vec3_t      scale;          // Usually (1,1,1)
} bone_keyframe_t;

// Full animation clip
typedef struct {
    char        name[32];
    int         numFrames;
    float       fps;
    bone_keyframe_t *frames;    // [numFrames * numBones]
} animation_clip_t;

// Vertex skinning data
typedef struct {
    int         boneIndex;      // Which bone affects this vertex
    float       weight;         // Influence weight (0-1)
} vertex_weight_t;

// Up to 4 bones can influence each vertex
#define MAX_BONES_PER_VERTEX 4
typedef struct {
    vertex_weight_t weights[MAX_BONES_PER_VERTEX];
} vertex_skin_t;
```

## Implementation Requirements

### 1. Bone Hierarchy

- Define bone structure with parent references
- Implement hierarchy traversal (parent-to-child order)
- Calculate world-space transforms from local transforms

### 2. Animation Sampling

```c
void Anim_SampleBone(animation_clip_t *clip, int boneIndex, float time,
                     vec3_t outPos, vec4_t outRot)
{
    // Find keyframes before and after 'time'
    // Interpolate position (LERP)
    // Interpolate rotation (SLERP for quaternions)
}
```

### 3. Animation Blending

```c
void Anim_Blend(bone_keyframe_t *a, bone_keyframe_t *b, float t,
                bone_keyframe_t *out)
{
    // Blend two poses (e.g., walk and run based on speed)
}
```

### 4. Vertex Skinning

```c
void Skin_TransformVertex(vec3_t inPos, vertex_skin_t *skin,
                         mat4_t *boneMatrices, vec3_t outPos)
{
    // For each bone weight:
    //   Transform vertex by bone's world matrix
    //   Accumulate weighted result
}
```

### 5. Integration with Renderer

- Modify `R_AliasDrawModel()` to use skeletal animation
- Support both old (morph) and new (skeletal) model formats
- Calculate bone matrices once per frame, reuse for shadows/reflections

## Files to Modify

- `game/r_alias.c` - Alias model rendering (add skinning)
- `game/model.c` - Model loading (parse skeletal data)
- `game/gl_model.c` - OpenGL model rendering
- Create new: `game/skeleton.c` - Skeletal animation core

## Mathematics Reference

### Quaternion SLERP
```c
void Quat_Slerp(vec4_t a, vec4_t b, float t, vec4_t out)
{
    float dot = a[0]*b[0] + a[1]*b[1] + a[2]*b[2] + a[3]*b[3];

    // If dot < 0, negate one quaternion to take shorter path
    if (dot < 0) {
        b[0] = -b[0]; b[1] = -b[1]; b[2] = -b[2]; b[3] = -b[3];
        dot = -dot;
    }

    // If nearly identical, use linear interpolation
    if (dot > 0.9995f) {
        Quat_Lerp(a, b, t, out);
        return;
    }

    float theta = acos(dot);
    float sinTheta = sin(theta);
    float wa = sin((1-t) * theta) / sinTheta;
    float wb = sin(t * theta) / sinTheta;

    out[0] = wa*a[0] + wb*b[0];
    out[1] = wa*a[1] + wb*b[1];
    out[2] = wa*a[2] + wb*b[2];
    out[3] = wa*a[3] + wb*b[3];
}
```

### Quaternion to Matrix
```c
void Quat_ToMatrix(vec4_t q, mat4_t out)
{
    float xx = q[0]*q[0], yy = q[1]*q[1], zz = q[2]*q[2];
    float xy = q[0]*q[1], xz = q[0]*q[2], yz = q[1]*q[2];
    float wx = q[3]*q[0], wy = q[3]*q[1], wz = q[3]*q[2];

    out[0][0] = 1 - 2*(yy+zz);  out[0][1] = 2*(xy-wz);      out[0][2] = 2*(xz+wy);
    out[1][0] = 2*(xy+wz);      out[1][1] = 1 - 2*(xx+zz);  out[1][2] = 2*(yz-wx);
    out[2][0] = 2*(xz-wy);      out[2][1] = 2*(yz+wx);      out[2][2] = 1 - 2*(xx+yy);
}
```

## Testing

```bash
# Compile and run tests
make test_skeleton

./test_skeleton
```

Tests include:
- Quaternion math (SLERP, normalization, to/from matrix)
- Bone hierarchy transform propagation
- Animation sampling at various times
- Skinning with different weight configurations
- Visual comparison with reference renders

## Hints

1. Start with a simple 2-bone arm to verify the math
2. Quaternions must be normalized after operations
3. The bind pose (T-pose) defines the "neutral" bone positions
4. Inverse bind pose matrices are needed for correct skinning
5. Performance: calculate all bone matrices first, then transform vertices
