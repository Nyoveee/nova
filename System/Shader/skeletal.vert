#version 450

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 aTextureUnit;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

layout (location = 5) in ivec4 boneIds;
layout (location = 6) in vec4 boneWeights;

layout(std140, binding = 0) uniform Camera {
    mat4 view;
    mat4 projection;
};

layout(std430, binding = 3) buffer Bones {
    mat4 bonesFinalMatrices[];
};

uniform mat4 model;

out vec2 textureUnit;

const int MAX_NUMBER_OF_BONES = 4;
const int INVALID_BONE = -1;

void main()
{
    mat4 boneTransform = mat4(1.0);

    for(int i = 0; i < MAX_NUMBER_OF_BONES; ++i) {
        // out of all the max number of bones, we iterate through each bones for each vertex..
        int boneId = boneIds[i];
        float boneWeight = boneWeights[i];

        // if this is invalid bone, we skip..
        if(boneId == INVALID_BONE) {
            continue;
        }

        // retrieve the corresponding bone final matrix and scale it according to weight..
        boneTransform += bonesFinalMatrices[boneId] * boneWeight;
    }  
    
    gl_Position = projection * view * model * boneTransform * vec4(pos, 1.0);
    textureUnit = aTextureUnit;
}