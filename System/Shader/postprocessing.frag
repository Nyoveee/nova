#version 450 core
struct FogData{
    vec3 radiance;
    float transmittance;
};

in vec2 textureCoords;
out vec4 FragColor;

uniform sampler2D scene;
uniform sampler2D depthTexture;

uniform vec3 offset;
uniform float vignette;

uniform vec3 fogColor;

uniform float fogNear;
uniform float fogFar;
uniform float fogDensity;

layout(std140, binding = 8) buffer VolumetricFogBuffer {
    FogData[] fogDatas;
};

// Uniform Buffers
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

float linearizeDepth(float depth) {
    // Example of reconstructing View-Space Z from a [0,1] Depth Map
    float ndcDepth = depth * 2.0 - 1.0; 
    float viewZ = (2.0 * zNear * zFar) / (zFar + zNear - ndcDepth * (zFar - zNear));

    return viewZ;
}

void main()
{             
    vec3 color = texture2D(scene, textureCoords).rgb; 
    float linearDepth = linearizeDepth(texture2D(depthTexture, textureCoords).r);

    float near = max(zNear, fogNear);
    float far = min(zFar, fogFar);

#if 0
    // Linear fog.
    float fogFactor = clamp((linearDepth - near)/ (far - near), 0, 1);
#else
    float linearDistanceFactor = clamp((linearDepth - near)/ (far - near), 0, 1);
    float fogFactor = clamp(exp(-(linearDistanceFactor * fogDensity * linearDistanceFactor * fogDensity)), 0, 1);
#endif

    // // Fog
    // uvec2 pixelPos = uvec2(textureCoords.x * screenResolution.x, textureCoords.y * screenResolution.y);
    // uint fogIndex = pixelPos.x + pixelPos.y * screenResolution.x;
    
    // float transmittance = clamp(fogDatas[fogIndex].transmittance, 0, 1);

    // // blend scene color with fog color.. based on transmittance value..
    // color = (color * transmittance) + (fogColor * (1 - transmittance));
    // // color = (color * transmittance);
    // // color = (fogColor * (1 - transmittance));

    // color += fogDatas[fogIndex].radiance;

    // // color = vec3(transmittance);
    FragColor = vec4(mix(fogColor, color, fogFactor), 1.0);
}  