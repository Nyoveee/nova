#version 450 core
layout (location = 0) in vec3 position;
layout (location = 4) in ivec4 boneIds;
layout (location = 5) in vec4 boneWeights;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;
uniform mat4 localScale;
uniform uint isSkinnedMesh;

const int MAX_NUMBER_OF_BONES = 4;
const int INVALID_BONE = -1;

layout(std430, binding = 3) buffer Bones {
    mat4 bonesFinalMatrices[];
};

invariant gl_Position;

void main()
{
    // this is not a skinned mesh.
    if(isSkinnedMesh == 0) {
        gl_Position = lightSpaceMatrix * model * localScale * vec4(position, 1.0);
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

        gl_Position = lightSpaceMatrix * model * localScale * localPosition;
    }
}  