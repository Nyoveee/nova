#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTextureUnit;

layout(std140, binding = 0) uniform Camera {
    mat4 view;
    mat4 projection;
};

uniform mat4 model;

out vec2 textureUnit;

void main()
{  
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    textureUnit = aTextureUnit;
}