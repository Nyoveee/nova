#version 450 core

struct WorldSpace {
    vec4 position;
    vec3 normal;
    vec3 tangent;
    vec4 previousPosition;
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
layout (location = 3) in vec3 tangent;
layout (location = 4) in ivec4 boneIds;
layout (location = 5) in vec4 boneWeights;

layout(std140, binding = 0) uniform Camera {
    mat4 view;
    mat4 projection;
    mat4 cameraProjectionView;
    mat4 inverseView;
    mat4 inverseProjection;
    mat4 inverseProjectionView;
    mat4 previousViewProjection;    // for TAA

    vec3 cameraPosition;

    uvec3 gridSize;
    uvec2 screenDimensions;
    float zNear;
    float zFar;
};

layout(std140, binding = 2) uniform PBRUBO {
    vec4 samples[64];   
	mat4 directionalLightSpaceMatrix;
	vec3 directionalLightDir;
	float timeElapsed;
	bool toEnableSSAO;
	bool hasDirectionalLightShadowCaster;
	bool toEnableIBL;
	bool toOutputNormal;
    float iblDiffuseStrength;
    float iblSpecularStrength;
};

layout(std140, binding = 4) uniform TAAUBO {
	vec4 haltonSequence[16]; // only first 2 elements used.
	int frameIndex;
    bool isTAAEnabled;
};

layout(std430, binding = 3) buffer Bones {
    mat4 bonesFinalMatrices[];
};

layout(std430, binding = 9) buffer OldBones {
    mat4 oldBonesFinalMatrices[];
};

const int MAX_NUMBER_OF_BONES = 4;
const int INVALID_BONE = -1;

layout (location = 0) uniform mat4 model;
layout (location = 4) uniform mat4 localScale;
layout (location = 8) uniform mat4 previousModel;
layout (location = 12) uniform mat3 normalMatrix;
layout (location = 16) uniform uint isSkinnedMesh;
layout (location = 17) uniform vec3 boundingBoxMin;
layout (location = 18) uniform vec3 boundingBoxMax;

layout (location = 21) uniform uint hasNoBones; // the model is a skinned mesh, but this particular model has no bones..
layout (location = 22) uniform mat4 globalTransformationMatrix; // since the mesh has no bones, we still need to chain the transformation matrix..

// location will be cached and query in runtime.. 
uniform bool toUseNormalMap;

invariant gl_Position;

out VS_OUT {
    vec2 textureUnit;
    vec3 normal;
    vec3 fragWorldPos;
    vec3 fragViewPos;
    vec4 fragDirectionalLightPos;
    invariant vec4 fragOldClipPos;
    invariant vec4 fragCurrentClipPos;
    vec3 boundingBoxUVW;
    mat3 TBN;
} vsOut;

// ======= Implementation =======

// This function calculates the TBN matrix given normal and tangent.
mat3 calculateTBN(vec3 N, vec3 T) {
    N = normalize(N);
    T = normalize(T);
    
    // these vectors are now in world space.
    vec3 B = cross(N, T);

    // we construct the TBN matrix, a change-of-basis matrix allowing us to transform any vectors from world space to tangent space. (specific for each primitive.)
    return mat3(T, B, N);
}

vec4 calculateClipPosition(vec4 worldPosition) {
    vec4 clipPos = cameraProjectionView * worldPosition;

    if(isTAAEnabled) {
        clipPos.xy += haltonSequence[frameIndex].xy * clipPos.w; // Apply Jittering;
    }

    return clipPos;
}

void passDataToFragment(WorldSpace worldSpace) {
    // Pass attributes to fragment shader.. 
    vsOut.textureUnit = textureUnit;
    vsOut.fragWorldPos = worldSpace.position.xyz;
    vsOut.fragViewPos = vec3(view * worldSpace.position);
    vsOut.fragDirectionalLightPos = directionalLightSpaceMatrix * worldSpace.position;

    // Passing normal..
    vsOut.normal = normalize(worldSpace.normal);
 
    if(toUseNormalMap) {
        vsOut.TBN = calculateTBN(worldSpace.normal, worldSpace.tangent);
    }

    // Passing temporal data over..
    vsOut.fragCurrentClipPos = cameraProjectionView * worldSpace.position;
    vsOut.fragOldClipPos = previousViewProjection * worldSpace.previousPosition;

    // We calculate bounding box UV here.. (its just range mapping..)
    vec3 scaledPosition = mat3(localScale) * position;
    vsOut.boundingBoxUVW = (scaledPosition - boundingBoxMin) / (boundingBoxMax - boundingBoxMin);
}

WorldSpace calculateWorldSpace() {
    WorldSpace worldSpace;

    // this is not a skinned mesh.
    if(isSkinnedMesh == 0) {
        worldSpace.position             = model * localScale * vec4(position, 1.0);
        worldSpace.normal               = normalize(normalMatrix * normal);
        worldSpace.tangent              = normalize(normalMatrix * tangent);
        worldSpace.previousPosition     = previousModel * localScale * vec4(position, 1.0);
    }
    // this is a skinned mesh, but doesnt have any bones..
    else if (hasNoBones == 1) {
        mat3 normalBoneTransform        = inverse(transpose(mat3(globalTransformationMatrix)));
        worldSpace.position             = model * localScale * globalTransformationMatrix * vec4(position, 1.0);
        worldSpace.normal               = normalize(normalMatrix * normalBoneTransform * normal);
        worldSpace.tangent              = normalize(normalMatrix * normalBoneTransform * tangent);
        worldSpace.previousPosition     = worldSpace.position; // screw it.. atp i cba..
    }
    // this is a skinned mesh.
    else {
        vec4 localPosition          = vec4(0.0);
        vec4 localPreviousPosition  = vec4(0.0);
        vec3 localNormal            = vec3(0.0);
        vec3 localTangent           = vec3(0.0);

        for(int i = 0; i < MAX_NUMBER_OF_BONES; ++i) {
            // out of all the max number of bones, we iterate through each bones for each vertex..
            int boneId = boneIds[i];
            float boneWeight = boneWeights[i];

            // if this is invalid bone, we skip..
            if(boneId == INVALID_BONE) {
                continue;
            }

            // retrieve the corresponding bone final matrix and scale it according to weight..
            localPosition           += (bonesFinalMatrices[boneId]      * vec4(position, 1.0)) * boneWeight;
            localPreviousPosition   += (oldBonesFinalMatrices[boneId]   * vec4(position, 1.0)) * boneWeight;

            // dealing with normals..
            mat3 normalBoneTransform = inverse(transpose(mat3(bonesFinalMatrices[boneId]))) * boneWeight;
            localNormal += normalBoneTransform * normal;
            localTangent += normalBoneTransform * tangent;
        }

        worldSpace.position             = model * localScale * localPosition;
        worldSpace.normal               = normalize(normalMatrix * localNormal);
        worldSpace.tangent              = normalize(normalMatrix * localTangent);
        worldSpace.previousPosition     = previousModel * localScale * localPreviousPosition;
    }

    return worldSpace;
}