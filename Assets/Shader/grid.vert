#version 450 core

uniform mat4 view;
uniform mat4 projection;
uniform vec3 cameraPos;

// A VBO-less grid render.
const vec3 vertexPos[4] = vec3[4](
		vec3(-1, 0, -1),	// bottom left
		vec3( 1, 0, -1),	// bottom right
		vec3( 1, 0,  1),	// top right
		vec3(-1, 0,  1) 	// top left
);

const int indices[6] = int[6](0, 2, 1, 2, 0, 3);

out vec3 worldPos;

void main() {
    int index = indices[gl_VertexID];
    vec3 pos = vertexPos[index];
    pos *= 100;

    pos.x += cameraPos.x;
    pos.z += cameraPos.z;
    worldPos = pos;

    gl_Position = projection * view * vec4(pos, 1);
}