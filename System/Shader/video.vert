#version 450 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec2 texture_crop_size;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 0.0, 1.0);
    //TexCoord = aTexCoord;

    TexCoord = vec2(aTexCoord.x, 1.0 - aTexCoord.y) * texture_crop_size;
}
