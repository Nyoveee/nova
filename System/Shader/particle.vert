#version 450 core

const vec3 vertexPos[4] = vec3[4](
	vec3(-1, -1,  0),	// bottom left
	vec3( 1, -1,  0),	// bottom right
	vec3( 1,  1,  0),	// top right
	vec3(-1,  1,  0) 	// top left
);

const vec2 textureCoordinates[4] = vec2[4](
    vec2(0, 0),
    vec2(1, 0),
    vec2(1, 1),
    vec2(0, 1)
);

const int indices[6] = int[6](0, 2, 1, 2, 0, 3);

layout(std140, binding = 0) uniform Camera {
    mat4 view;
    mat4 projection;
};

uniform mat4 model;
uniform mat4 localRotation;
uniform float particleSize;

out vec2 textureUnit;

void main() {
    int index = indices[gl_VertexID];
    mat4 modelView = view * model;
    
    // Eliminate world Rotation(Bill Boarding)
    modelView[0][0] = 1;
    modelView[0][1] = 0;
    modelView[0][2] = 0;

    modelView[1][0] = 0;
    modelView[1][1] = -1;
    modelView[1][2] = 0;

    modelView[2][0] = 0;
    modelView[2][1] = 0;
    modelView[2][2] = 1;

    gl_Position =  projection * modelView * localRotation * vec4(vertexPos[index] * particleSize, 1);
    textureUnit = textureCoordinates[index];
}