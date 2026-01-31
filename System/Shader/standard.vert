#version 450 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 textureUnit;

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


uniform mat4 model;
uniform mat4 localScale;

out VS_OUT {
    out vec2 textureUnit;
} vsOut;

void main()
{  
    gl_Position = cameraProjectionView * model * localScale * vec4(pos, 1.0);
    vsOut.textureUnit = textureUnit;
}