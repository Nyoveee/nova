#version 450 core

struct WorldSpace {
    vec4 position;
    vec3 normal;
};

// ======= Function declaration =======

// This function calculates the world space of these attributes given local attributes.
// resulting normal and tangent are normalised.
WorldSpace calculateWorldSpace(vec3 localPosition, vec3 localNormal, vec3 localTangent);

// This function calculates the clip space of position given WORLD position. (note the difference vs in Color pipeline)
// This can be passed directly to gl_Position;
vec4 calculateClipPosition(vec4 worldPosition);

// This function calculates the TBN matrix given the world normal and tangent. assumes that these vectors are normalised.
mat3 calculateTBN(vec3 worldNormal, vec3 worldTangent);

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
uniform mat3 normalMatrix;
uniform mat4 localScale;
uniform float timeElapsed;

invariant gl_Position;

out VS_OUT {
    vec2 textureUnit;
    vec3 normal;
    vec3 fragWorldPos;
    vec3 fragViewPos;
} vsOut;

// ======= Implementation =======

vec4 calculateClipPosition(vec4 worldPosition) {
    return cameraProjectionView * worldPosition;
}

WorldSpace calculateWorldSpace(vec3 position, vec3 normal) {
    WorldSpace worldSpace;

    // this is not a skinned mesh.
    if(isSkinnedMesh == 0) {    
        worldSpace.position             = model * localScale * vec4(position, 1.0);
        worldSpace.normal               = normalize(normalMatrix * normal);
    }
    // this is a skinned mesh.
    else {
        vec4 localPosition = vec4(0.0);
        vec3 localNormal = vec3(0.0);

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

            // dealing with normals..
            mat3 normalBoneTransform = inverse(transpose(mat3(bonesFinalMatrices[boneId]))) * boneWeight;
            localNormal += normalBoneTransform * normal;
        }

        worldSpace.position             = model * localScale * localPosition;
        worldSpace.normal               = normalize(normalMatrix * localNormal);
    }

    // Pass attributes to fragment shader.. 
    // vsOut.textureUnit = textureUnit;
    // vsOut.fragWorldPos = worldSpace.position.xyz;
    // vsOut.fragViewPos = vec3(view * worldSpace.position);

    // vsOut.normal = normalize(worldSpace.normal);

    return worldSpace;
}