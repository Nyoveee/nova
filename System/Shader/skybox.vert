#version 450 core

// This vertex shader passes a 3D box over the screen
// This is used for post processing.

// A VBO-less cube   render.
// 8 unique vertices of a unit cube (centered at origin, spanning -1..+1)
const vec3 vertexPos[8] = vec3[8](
    vec3(-1.0, -1.0, -1.0), // 0: left,  bottom, back
    vec3( 1.0, -1.0, -1.0), // 1: right, bottom, back
    vec3( 1.0,  1.0, -1.0), // 2: right, top,    back
    vec3(-1.0,  1.0, -1.0), // 3: left,  top,    back
    vec3(-1.0, -1.0,  1.0), // 4: left,  bottom, front
    vec3( 1.0, -1.0,  1.0), // 5: right, bottom, front
    vec3( 1.0,  1.0,  1.0), // 6: right, top,    front
    vec3(-1.0,  1.0,  1.0)  // 7: left,  top,    front
);

// 12 triangles (36 indices total)
const int indices[36] = int[36](
    // back face (-Z)
    0, 2, 1,
    2, 0, 3,

    // front face (+Z)
    4, 5, 6,
    6, 7, 4,

    // left face (-X)
    0, 7, 3,
    7, 0, 4,

    // right face (+X)
    1, 2, 6,
    6, 5, 1,

    // bottom face (-Y)
    0, 1, 5,
    5, 4, 0,

    // top face (+Y)
    3, 7, 6,
    6, 2, 3
);

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

out vec3 localPos;

void main() {
    int index = indices[gl_VertexID];

    localPos = vertexPos[index];

    mat4 viewWithoutTranslation = mat4(mat3(view));
    gl_Position = projection * viewWithoutTranslation * vec4(vertexPos[index], 1);
}