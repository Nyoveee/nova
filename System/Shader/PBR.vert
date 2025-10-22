#version 450 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 textureUnit;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

layout(std140, binding = 0) uniform Camera {
    mat4 view;
    mat4 projection;
};

uniform mat4 model;
uniform mat3 normalMatrix;
uniform bool isUsingNormalMap;

out VS_OUT {
    vec2 textureUnit;
    vec3 normal;
    vec3 fragWorldPos;
    vec4 fragPosLightSpace;
    mat3 TBN;
} vsOut;

uniform mat4 lightSpaceMatrix;

void main()
{
    gl_Position = projection * view * model * vec4(pos, 1.0);

    // we interpolate the world position of the vertices to obtain fragment positions
    vsOut.fragWorldPos = vec3(model * vec4(pos, 1.0));
    vsOut.fragPosLightSpace = lightSpaceMatrix * vec4(vsOut.fragWorldPos, 1.0);
    vsOut.textureUnit = textureUnit;
    vsOut.normal = normalMatrix * normal;

    // TBN matrix.
    if(!isUsingNormalMap) {
        return;
    }

    // these vectors are now in world space.
    vec3 T = normalize(normalMatrix * tangent);
    vec3 N = normalize(normalMatrix * normal);

    // T = normalize(T - dot(T, N) * N);
    // vec3 B = cross(N, T);
    vec3 B = normalize(normalMatrix * bitangent);

    // we construct the TBN matrix, a change-of-basis matrix allowing us to transform any vectors from world space to tangent space. (specific for each primitive.)
    vsOut.TBN = mat3(T, B, N);
}