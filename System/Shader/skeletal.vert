#version 450

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 textureUnit;
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
uniform mat3 normalMatrix;
uniform bool isUsingNormalMap;

out VS_OUT {
    vec2 textureUnit;
    vec3 normal;
    vec3 fragWorldPos;
    mat3 TBN;
} vsOut;

const int MAX_NUMBER_OF_BONES = 4;
const int INVALID_BONE = -1;

void main()
{
    vec4 localPosition = vec4(0.0);
    vec3 localNormal = vec3(0.0);
    vec3 localTangent = vec3(0.0);

    for(int i = 0; i < MAX_NUMBER_OF_BONES; ++i) {
        // out of all the max number of bones, we iterate through each bones for each vertex..
        int boneId = boneIds[i];
        float boneWeight = boneWeights[i];

        // if this is invalid bone, we skip..
        if(boneId == INVALID_BONE) {
            continue;
        }

        // retrieve the corresponding bone final matrix and scale it according to weight..
        localPosition += (bonesFinalMatrices[boneId] * vec4(pos, 1.0)) * boneWeight;

        // dealing with normals..
        mat3 normalBoneTransform = inverse(transpose(mat3(bonesFinalMatrices[boneId]))) * boneWeight;
        localNormal += normalBoneTransform * normal;
        localTangent += normalBoneTransform * tangent;
    }  
    
    gl_Position = projection * view * model * localPosition;
    vsOut.textureUnit = textureUnit;

    // we interpolate the world position of the vertices to obtain fragment positions
    // because theres translation, we cannot simply do vec3(). we need to convert from 
    // homogeneous coordinates to cartesian coordinates.
    vec4 homoWorldPos = model * localPosition;
    vsOut.fragWorldPos = vec3(homoWorldPos.x / homoWorldPos.w, homoWorldPos.y / homoWorldPos.w, homoWorldPos.z / homoWorldPos.w);
    vsOut.normal = normalMatrix * localNormal;

    // TBN matrix.
    if(!isUsingNormalMap) {
        return;
    }

    // these vectors are now in world space.
    vec3 T = normalize(normalMatrix * localTangent);
    vec3 N = normalize(normalMatrix * localNormal);

    // T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    // vec3 B = normalize(normalMatrix * bitangent);

    // we construct the TBN matrix, a change-of-basis matrix allowing us to transform any vectors from world space to tangent space. (specific for each primitive.)
    vsOut.TBN = mat3(T, B, N);
}