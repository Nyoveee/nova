#version 450 core

// This vertex shader passes a 2D plane over the screen, regardless of camera position.
// This is used for post processing.

// A VBO-less square render.
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

out vec2 textureCoords;

void main() {
    int index = indices[gl_VertexID];

    gl_Position = vec4(vertexPos[index], 1);
    textureCoords = textureCoordinates[index];
}