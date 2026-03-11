#version 450 core

in vec2 textureCoords;
out vec4 FragColor;

uniform sampler2D scene;
uniform sampler2D depthTexture;

uniform vec3 fogColor;

uniform float fogNear;
uniform float fogFar;
uniform float fogDensity;

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
    float linearDistanceFactor = clamp((linearDepth - near)/ (far - near), 0, 1);
    float fogFactor = clamp(exp(-(linearDistanceFactor * fogDensity * linearDistanceFactor * fogDensity)), 0, 1);

    color = mix(fogColor, color, fogFactor);
    FragColor = vec4(color, 1.0);
}  