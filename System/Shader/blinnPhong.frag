#version 450 core

out vec4 FragColor;

in VS_OUT {
    vec2 textureUnit;
    vec3 normal;
    vec3 fragWorldPos;
    mat3 TBN;
} fsIn;

// === ALBEDO FOR BLINN PHONG ===
uniform vec3 albedo;
uniform bool isUsingAlbedoMap;
uniform sampler2D albedoMap;

uniform float ambientFactor;

// === LIGHT PROPERTIES ===
// corresponds to the enum in C++
const int PointLightEnum = 0;
const int DirectionalLightEnum = 1;
const int SpotLightEnum = 2;

struct PointLight {
    vec3 position;
    vec3 color;
    vec3 attenuation;
    int type;
};
struct DirectionalLight {
    vec3 direction;
    vec3 color;
    int type;
};
struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 color;
	float cutoffAngle;
	float outerCutoffAngle;
    int type;
};

layout(std430, binding = 0) buffer PointLights {
    uint pointLightCount;
    PointLight pointLights[];
};

layout(std430, binding = 1) buffer DirectionalLights {
    uint dirLightCount;
    DirectionalLight dirLights[];
};

layout(std430, binding = 2) buffer SpotLights {
    uint spotLightCount;
    PointLight spotLights[];
};

uniform vec3 cameraPos;

// === NORMAL MAPPING ===
uniform bool isUsingNormalMap;
uniform sampler2D normalMap;

// calculate the resulting color caused by this one light.
vec3 calculatePointLight(PointLight light, vec3 normal, vec3 baseColor) {
    // this is really from fragment position to light position. 
    // we do this to align with the normal.
    vec3 lightDiff = light.position - fsIn.fragWorldPos;
    vec3 lightDir = normalize(lightDiff);

    // hehe we will be using this in the PBR rendering next time!!
    float cosTheta = max(dot(lightDir, normal), 0);

    // this is our diffuse.
    vec3 diffuseColor = baseColor * cosTheta * light.color;

    // let's calculate specular
    vec3 viewDir = normalize(cameraPos - fsIn.fragWorldPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    vec3 specularColor = pow(max(dot(normal, halfwayDir), 0.0), 32.0) * light.color;

    // Attenuation vals
    float dist = length(lightDiff);
    float attenVal = 1.0 / (light.attenuation[0] + light.attenuation[1] * dist + 
    		    light.attenuation[2] * (dist * dist));  
    diffuseColor *= attenVal;
    specularColor *= attenVal;

    return diffuseColor + specularColor;
}

// calculate the resulting color caused by this one light.
vec3 calculateDirLight(DirectionalLight light, vec3 normal, vec3 baseColor) {
    vec3 lightDir = normalize(-light.direction);

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
    for(int i = 0; i < pointLightCount; ++i) {
        finalColor += calculatePointLight(pointLights[i], normal, baseColor);
    }
    for(int i = 0; i < dirLightCount; ++i) {
        finalColor += calculateDirLight(dirLights[i], normal, baseColor);
    }
    for(int i = 0; i < spotLightCount; ++i) {
        finalColor += calculatePointLight(spotLights[i], normal, baseColor);
    }

    FragColor = vec4(finalColor, 1);
}