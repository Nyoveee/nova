#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTextureUnit;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform mat3 normalMatrix;

out vec2 textureUnit;
out vec3 normal;
out vec3 fragWorldPos;

void main()
{  
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    
    // we interpolate the world position of the vertices to obtain fragment positions
    fragWorldPos = vec3(model * vec4(aPos, 1.0));

    textureUnit = aTextureUnit;

    normal = normalMatrix * aNormal;
}