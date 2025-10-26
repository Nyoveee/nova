#version 450 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 textureUnit;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 tangent;

layout(std140, binding = 0) uniform Camera {
    mat4 view;
    mat4 projection;
};

uniform mat4 model;
uniform mat3 normalMatrix;

out VS_OUT {
    vec2 textureUnit;
    vec3 normal;
    vec3 fragWorldPos;
    mat3 TBN;
} vsOut;

void main()
{  
    gl_Position = projection * view * model * vec4(pos, 1.0);
    
    // we interpolate the world position of the vertices to obtain fragment positions
    vsOut.textureUnit = textureUnit;
    
    vsOut.fragWorldPos = vec3(model * vec4(pos, 1.0));
    vsOut.TBN = calculateTBN(normal, tangent, normalMatrix);
}