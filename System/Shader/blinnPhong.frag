#version 450 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out uint ObjectId;

in VS_OUT {
    vec2 textureUnit;
    vec3 normal;
    vec3 fragWorldPos;
    mat3 TBN;
} fsIn;

uniform uint objectId;

// === ALBEDO FOR BLINN PHONG ===
uniform vec3 albedo;
uniform bool isUsingAlbedoMap;
uniform sampler2D albedoMap;

uniform float ambientFactor;

// === LIGHT PROPERTIES ===
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 cameraPos;

// === NORMAL MAPPING ===
uniform bool isUsingNormalMap;
uniform sampler2D normalMap;

void main() {
    // For object picking, not part of blinn-phong.
    ObjectId = objectId;

    vec3 normal;

    if(isUsingNormalMap) {
        vec3 sampledNormal = vec3(texture(normalMap, fsIn.textureUnit));
        sampledNormal = sampledNormal * 2.0 - 1.0; 
        // sampledNormal = vec3(sampledNormal.r, sampledNormal.g, sampledNormal.b);
        // FragColor = vec4(fsIn.TBN * sampledNormal, 1);
        
        // return;
        normal = normalize(fsIn.TBN * sampledNormal);
    }
    else {
        normal = normalize(fsIn.normal);
    }

    // this is really from world position to light position. 
    // we do this to align with the normal.
    vec3 lightDir = normalize(lightPos - fsIn.fragWorldPos);

    // hehe we will be using this in the PBR rendering next time!!
    float cosTheta = max(dot(lightDir, normal), 0);

    // this is our diffuse.
    vec3 baseColor = isUsingAlbedoMap ? vec3(texture(albedoMap, fsIn.textureUnit)) : albedo;
    vec3 diffuseColor = baseColor * cosTheta * lightColor;

    // ambient is the easiest.
    vec3 ambientColor = baseColor * ambientFactor;

    // let's calculate specular
    vec3 viewDir = normalize(cameraPos - fsIn.fragWorldPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    vec3 specularColor = pow(max(dot(normal, halfwayDir), 0.0), 32.0) * lightColor * 0.5;

    FragColor = vec4(ambientColor + diffuseColor + specularColor, 1);
}