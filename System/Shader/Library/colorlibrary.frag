#version 450 core

// Very simple library

in VS_OUT {
    vec2 textureUnit;
    vec3 normal;
    vec3 fragWorldPos;
    vec3 fragViewPos;
    vec4 fragOldClipPos;
    vec4 fragCurrentClipPos;
    vec3 boundingBoxUVW;
} fsIn;

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
	int renderOutputMode;
    float iblDiffuseStrength;
    float iblSpecularStrength;
};

const int renderNormals = 0;
const int renderColors = 1;
const int renderTransparency = 2;

layout (binding = 3) uniform sampler2D depthTexture;

// explicit location.
layout (location = 20) uniform bool toUseNormalMap;
layout (location = 30) uniform bool toUseEmissiveMap;
layout (location = 31) uniform bool toUseAlphaMap;

vec2 UVTileAndOffset(vec2 textureCoordinates, vec2 UVTiling, vec2 UVOffset) {
    return textureCoordinates * UVTiling + UVOffset;
}

float linearizeDepth(float depth) {
    float ndcDepth = depth * 2.0 - 1.0; 
    return (2.0 * zNear * zFar) / (zFar + zNear - ndcDepth * (zFar - zNear));
}

// https://sugulee.wordpress.com/2021/06/21/temporal-anti-aliasingtaa-tutorial/
vec2 calculateVelocityUV(vec4 fragCurrentClipPos, vec4 fragOldClipPos) {
    fragOldClipPos /= fragOldClipPos.w;                             // perspective divide..
    fragOldClipPos.xy = (fragOldClipPos.xy + 1.0) / 2.0;            // transform [-1, 1] to [0, 1] (uv range)
    
    fragCurrentClipPos /= fragCurrentClipPos.w;                     // perspective divide..
    fragCurrentClipPos.xy = (fragCurrentClipPos.xy + 1.0) / 2.0;    // transform [-1, 1] to [0, 1] (uv range)
    
    // return delta..
    return (fragCurrentClipPos - fragOldClipPos).xy;
}

// User shader entry point.
vec4 __internal__main__();
layout (location = 0) out vec4 FragColor;

// gbuffer details.. for SSAO and TAA..
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec2 velocityUV;

// transparency.. for OIT..
layout (location = 3) out vec4 accumulation;
layout (location = 4) out float revealage;

// Wrapper around user entry point.
void main() { 
    if(renderOutputMode == renderNormals) {
        gNormal = fsIn.normal;
        velocityUV = calculateVelocityUV(fsIn.fragCurrentClipPos, fsIn.fragOldClipPos);
    }
    else if(renderOutputMode == renderTransparency) {
        vec4 color = __internal__main__();

        // weight function.. (:O tbh.)
        float weight = clamp(pow(min(1.0, color.a * 10.0) + 0.01, 3.0) * 1e8 * 
                            pow(1.0 - gl_FragCoord.z * 0.9, 3.0), 1e-2, 3e3);

        // store pixel color accumulation
        accumulation = vec4(color.rgb * color.a, color.a) * weight;

        // store pixel revealage thresholda
        revealage = color.a;
    }
    else {
        FragColor = __internal__main__();   
    }
}