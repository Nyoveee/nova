#version 450 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 textureUnit;

layout(std140, binding = 0) uniform Camera {
    mat4 view;
    mat4 projection;
};

uniform mat4 model;
uniform mat4 localScale;

out VS_OUT {
    out vec2 textureUnit;
} vsOut;

void main()
{  
    gl_Position = projection * view * model * localScale * vec4(pos, 1.0);
    vsOut.textureUnit = textureUnit;
}