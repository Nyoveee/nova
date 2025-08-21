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
// corresponds to the enum in C++
const int PointLight = 0;
const int DirectionalLight = 1;
const int SpotLight = 2;

struct Light {
    vec3 position;
    vec3 color;
    int type;
};

layout(std430, binding = 0) buffer Lights {
    uint lightCount;
    Light lights[];
};

uniform vec3 cameraPos;

// === NORMAL MAPPING ===
uniform bool isUsingNormalMap;
uniform sampler2D normalMap;

// calculate the resulting color caused by this one light.
vec3 calculateLight(Light light, vec3 normal, vec3 baseColor) {
    // this is really from fragment position to light position. 
    // we do this to align with the normal.
    vec3 lightDir = normalize(light.position - fsIn.fragWorldPos);

    // hehe we will be using this in the PBR rendering next time!!
    float cosTheta = max(dot(lightDir, normal), 0);

    // this is our diffuse.
    vec3 diffuseColor = baseColor * cosTheta * light.color;

    // let's calculate specular
    vec3 viewDir = normalize(cameraPos - fsIn.fragWorldPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    vec3 specularColor = pow(max(dot(normal, halfwayDir), 0.0), 32.0) * light.color;

    return diffuseColor + specularColor;
}

void main() {
    // For object picking, not part of blinn-phong.
    ObjectId = objectId;

    // Getting the normals..
    vec3 normal;

    if(isUsingNormalMap) {
        vec3 sampledNormal = vec3(texture(normalMap, fsIn.textureUnit));
        sampledNormal = sampledNormal * 2.0 - 1.0; 
        normal = normalize(fsIn.TBN * sampledNormal);
    }
    else {
        normal = normalize(fsIn.normal);
    }

    // Getting base color
    vec3 baseColor = isUsingAlbedoMap ? vec3(texture(albedoMap, fsIn.textureUnit)) : albedo;

    // ambient is the easiest.
    vec3 finalColor = baseColor * ambientFactor;

    // Calculate diffuse and specular light for each light.
    for(int i = 0; i < lightCount; ++i) {
        finalColor += calculateLight(lights[i], normal, baseColor);
    }

    FragColor = vec4(finalColor, 1);
}