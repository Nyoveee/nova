#version 450 core

out vec4 FragColor;

in VS_OUT {
    vec2 textureUnit;
    vec3 normal;
    vec3 fragWorldPos;
    mat3 TBN;
    vec3 pointLightViewPos[];
    vec3 dirLightViewPos[];
    vec3 spotLightViewPos[];
} fsIn;

// === MATERIAL FOR PBR ===
uniform vec3 albedo;
uniform bool isUsingAlbedoMap;
uniform sampler2D albedoMap;

struct Config {
	float roughness;
	float metallic;
	float occulusion;
};

uniform Config material;

uniform float ambientFactor;

const float PI = 3.14159265358979323846;
float ggxDistribution(float nDotH);
float geomSmith(float nDotL) ;
vec3 schlickFresnel(float lDotH);
vec3 microfacetModel(vec3 position, vec3 n, vec3 lightIntensity, vec3 lightViewPos);

// === LIGHT PROPERTIES ===
struct PointLight {
    vec3 position;
    vec3 color;
    vec3 attenuation;
};

struct DirectionalLight {
    vec3 direction;
    vec3 color;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 color;
    vec3 attenuation;
	float cutOffAngle;
	float outerCutOffAngle;
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
    SpotLight spotLights[];
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
        // Calculate point light contribution using microfacet model
        finalColor += microfacetModel(fsIn.fragWorldPos, normal, pointLights[i].color, fsIn.pointLightViewPos[i]);
    }

    FragColor = vec4(finalColor, 1);
}

// The Microgeometry Normal Distribution Function, based on GGX/Trowbrodge-Reitz, 
// that describes the relative concentration of microfacet normals 
// in the direction H. It has an effect on the size and shape 
// of the specular highlight.
//
// Parameter is cosine of the angle between the normal and H which is the halfway vector of 
// both the light direction and the view direction
//
float ggxDistribution(float nDotH) 
{
    float alpha2 = material.roughness * material.roughness * material.roughness * material.roughness;
    float d = (nDotH * nDotH) * (alpha2 - 1.0f) + 1.0f;
    return alpha2 / (PI * d * d);
}

// The Smith Masking-Shadowing Function describes the probability that microfacets with 
// a given normal are visible from both the light direction and the view direction.
//
// Parameter is cosine of the angle between the normal vector and the view direction.
//
float geomSmith(float nDotL) 
{
    float k = (material.roughness + 1.0f) * (material.roughness + 1.0f) / 8.0f;
    float denom = nDotL * (1.0f - k) + k;
    return 1.0f / denom;
}

// Schlick approximation for Fresnel reflection that defines the probability of light
// reflected from an optically flat surface.
//
// Parameter is cosine of the angle between the light direction vector and 
// the halfway vector of both the light direction and the view direction
//
vec3 schlickFresnel(float lDotH) 
{
    vec3 f0 = vec3(0.04f); // Dielectrics
    if (material.metallic == 1.0f)
        f0 = albedo;
    return f0 + (1.0f - f0) * pow(1.0f - lDotH, 5);
}

// Bidirectional Reflectance Distribution Function.
// This is the common way to model reflectance based on microfacet theory. 
// This theory was developed to describe reflection from general, non-optically flat surfaces. 
// It models the surface as consisting of small facets that are optically flat (mirrors) and 
// are oriented in various directions. Only those that are oriented correctly to reflect toward 
// the viewer can contribute.
//
// Parameters are the position of a fragment and the surface normal in the view space.
//
vec3 microfacetModel(vec3 position, vec3 n, vec3 lightIntensity, vec3 lightViewPos) 
{  
    vec3 diffuseBrdf = albedo;

    vec3 l = lightViewPos - position;
    float dist = length(l);
    l = normalize(l);
    lightIntensity *= 100 / (dist * dist); // Intensity is normalized, so scale up by 100 first

    vec3 v = normalize(-position);
    vec3 h = normalize(v + l);
    float nDotH = dot(n, h);
    float lDotH = dot(l, h);
    float nDotL = max(dot(n, l), 0.0f);
    float nDotV = dot(n, v);
    vec3 specBrdf = 0.25f * ggxDistribution(nDotH) * schlickFresnel(lDotH) 
                            * geomSmith(nDotL) * geomSmith(nDotV); 

    return (diffuseBrdf + PI * specBrdf) * lightIntensity * nDotL;
}