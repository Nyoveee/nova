#version 450 core

layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;

uniform mat4 projection;
uniform float offset; // offset due to alignment.

void main()
{
    gl_Position = projection * vec4(vertex.x + offset, vertex.y, 0.0, 1.0);
    TexCoords = vertex.zw;
}  