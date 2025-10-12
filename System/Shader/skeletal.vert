#version 450

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 aTextureUnit;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

layout (location = 5) in ivec4 boneIds;
layout (location = 6) in vec4 boneWeights;

layout(std140, binding = 0) uniform Camera {
    mat4 view;
    mat4 projection;
};

layout(std430, binding = 3) buffer Bones {
    uint numOfBones;
    mat4 bonesFinalMatrices[];
};

uniform mat4 model;

out vec2 textureUnit;

void main()
{  
    gl_Position = projection * view * model * vec4(pos, 1.0);
    textureUnit = aTextureUnit;
}