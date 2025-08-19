#version 450 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out uint ObjectId;

in vec2 textureUnit;
in vec3 normal;
in vec3 fragWorldPos;

uniform uint objectId;

uniform vec3 albedo;
uniform bool isUsingAlbedoMap;
uniform sampler2D albedoMap;

uniform float ambientFactor;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 cameraPos;

void main() {
    // this is really from world position to light position. 
    // we do this to align with the normal.
    vec3 lightDir = normalize(lightPos - fragWorldPos);
    vec3 normal = normalize(normal);

    // hehe we will be using this in the PBR rendering next time!!
    float cosTheta = max(dot(lightDir, normal), 0);

    // this is our diffuse.
    vec3 baseColor = isUsingAlbedoMap ? vec3(texture(albedoMap, textureUnit)) : albedo;
    vec3 diffuseColor = baseColor * cosTheta * lightColor;

    // ambient is the easiest.
    vec3 ambientColor = baseColor * ambientFactor;

    // let's calculate specular
    vec3 viewDir = normalize(cameraPos - fragWorldPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    vec3 specularColor = pow(max(dot(normal, halfwayDir), 0.0), 32.0) * lightColor;

    FragColor = vec4(ambientColor + diffuseColor + specularColor, 1);

    // For object picking, not part of blinn-phong.
    ObjectId = objectId;
}