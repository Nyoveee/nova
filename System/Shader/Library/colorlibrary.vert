#version 450 core

// ======= Function declaration =======

// This function calculates the clip space of position given local position.
// This can be passed directly to gl_Position;
vec4 calculateClipPosition(vec3 localPosition);

// ======= Vertex attributes, uniforms and inputs set by the pipeline =======
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 textureUnit;
layout (location = 2) in vec3 normal;
layout (location = 4) in ivec4 boneIds;
layout (location = 5) in vec4 boneWeights;

layout(std140, binding = 0) uniform Camera {
    mat4 view;
    mat4 projection;
    mat4 cameraProjectionView;
};

layout(std430, binding = 3) buffer Bones {
    uint isSkinnedMesh;
    mat4 bonesFinalMatrices[];
};

const int MAX_NUMBER_OF_BONES = 4;
const int INVALID_BONE = -1;

uniform mat4 model;
uniform mat4 localScale;
uniform float timeElapsed;

invariant gl_Position;

out VS_OUT {
    out vec2 textureUnit;
} vsOut;

// ======= Implementation =======

// This function calculates the world position given mesh's local position.
vec4 calculateClipPosition(vec3 position) {
    // this is not a skinned mesh.
    if(isSkinnedMesh == 0) {
        return cameraProjectionView * model * localScale * vec4(position, 1.0);
    }
    // this is a skinned mesh.
    else {
        vec4 localPosition = vec4(0.0);

        for(int i = 0; i < MAX_NUMBER_OF_BONES; ++i) {
            // out of all the max number of bones, we iterate through each bones for each vertex..
            int boneId = boneIds[i];
            float boneWeight = boneWeights[i];

            // if this is invalid bone, we skip..
            if(boneId == INVALID_BONE) {
                continue;
            }

            // retrieve the corresponding bone final matrix and scale it according to weight..
            localPosition += (bonesFinalMatrices[boneId] * vec4(position, 1.0)) * boneWeight;
        }

        return cameraProjectionView * model * localScale * localPosition;
    }
}