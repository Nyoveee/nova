
#version 450 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 textureUnit;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 tangent;
layout (location = 4) in ivec4 boneIds;
layout (location = 5) in vec4 boneWeights;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;
uniform mat4 localScale;
uniform mat3 normalMatrix;

const int MAX_NUMBER_OF_BONES = 4;
const int INVALID_BONE = -1;

layout(std430, binding = 3) buffer Bones {
    uint isSkinnedMesh;
    mat4 bonesFinalMatrices[];
};

out VS_OUT {
    vec3 normal;
    vec2 textureUnit;
    mat3 TBN;
} vsOut;

invariant gl_Position;

uniform bool toUseNormalMap;

// This function calculates the TBN matrix given normal and tangent.
mat3 calculateTBN(vec3 N, vec3 T) {
    N = normalize(N);
    T = normalize(T);
    
    // these vectors are now in world space.
    vec3 B = cross(N, T);

    // we construct the TBN matrix, a change-of-basis matrix allowing us to transform any vectors from world space to tangent space. (specific for each primitive.)
    return mat3(T, B, N);
}

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

    vec3 worldSpaceNormal  = normalize(normalMatrix * normal);
    vec3 worldSpaceTangent = normalize(normalMatrix * tangent);

    vsOut.normal = worldSpaceNormal;
    vsOut.textureUnit = textureUnit;
    
    if(toUseNormalMap) {
        vsOut.TBN = calculateTBN(worldSpaceNormal, worldSpaceTangent);
    }
}  