#version 450 core

layout (location = 0) in vec3 aPos;

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

uniform mat4 model = mat4(1.0);

void main()
{  
    gl_Position = cameraProjectionView * model * vec4(aPos, 1.0);
}