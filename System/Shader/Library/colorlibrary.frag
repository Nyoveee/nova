#version 450 core

// Very simple library

in VS_OUT {
    vec2 textureUnit;
    vec3 normal;
    vec3 fragWorldPos;
    vec3 fragViewPos;
    vec4 fragOldClipPos;
    vec4 fragCurrentClipPos;
} fsIn;

layout (location = 0) out vec4 FragColor; 
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec2 velocityUV;

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
};

vec2 UVTileAndOffset(vec2 textureCoordinates, vec2 UVTiling, vec2 UVOffset) {
    return textureCoordinates * UVTiling + UVOffset;
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

// Wrapper around user entry point.
void main() { 
    if(toOutputNormal) {
        gNormal = fsIn.normal;
        velocityUV = calculateVelocityUV(fsIn.fragCurrentClipPos, fsIn.fragOldClipPos);
    }
    else {
        FragColor = __internal__main__();   
    }
}