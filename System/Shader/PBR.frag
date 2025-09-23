#version 450 core

out vec4 FragColor;

in VS_OUT {
    vec2 textureUnit;
    vec3 normal;
    vec3 fragWorldPos;
    mat3 TBN;
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
uniform bool isUsingPackedTextureMap;
uniform sampler2D packedMap;
Config cfg;

uniform float ambientFactor;

const float PI = 3.14159265358979323846;
float ggxDistribution(float nDotH);
float geomSmith(float nDotL) ;
vec3 schlickFresnel(float lDotH, vec3 baseColor);
vec3 microfacetModel(vec3 position, vec3 n, vec3 lightPos, vec3 lightIntensity, vec3 baseColor);

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

    // Getting base color, converts from sRGB
    vec3 baseColor = isUsingAlbedoMap 
                    ? pow(texture(albedoMap, fsIn.textureUnit).rgb, vec3(2.2)) 
                    : albedo;

    // Getting material texture
    if (isUsingPackedTextureMap) {
        vec3 map = texture(packedMap, fsIn.textureUnit).rgb;
        cfg.metallic   = map.r;
        cfg.roughness  = map.g;
        cfg.occulusion = map.b;
    } else {
        cfg = material;
    }

    // ambient is the easiest.
    vec3 finalColor = baseColor * ambientFactor * cfg.occulusion;

    // Calculate diffuse and specular light for each light.
    for(int i = 0; i < pointLightCount; ++i) {
        // Calculate point light contribution using microfacet model
        finalColor += microfacetModel(fsIn.fragWorldPos, normal, pointLights[i].position, pointLights[i].color, baseColor);
    }
    
    finalColor = pow(finalColor, vec3(1.0/2.2)); 
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
    float alpha2 = cfg.roughness * cfg.roughness * cfg.roughness * cfg.roughness;
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
    float k = (cfg.roughness + 1.0f) * (cfg.roughness + 1.0f) / 8.0f;
    float denom = nDotL * (1.0f - k) + k;
    return 1.0f / denom;
}

// Schlick approximation for Fresnel reflection that defines the probability of light
// reflected from an optically flat surface.
//
// Parameter is cosine of the angle between the light direction vector and 
// the halfway vector of both the light direction and the view direction
//
vec3 schlickFresnel(float lDotH, vec3 baseColor) 
{
    vec3 f0 = vec3(0.04f); // Dielectrics
    f0 = mix(f0, baseColor, cfg.metallic);
    return f0 + (1.0f - f0) * pow(1.0f - lDotH, 5);
}

// Bidirectional Reflectance Distribution Function.
// This is the common way to model reflectance based on microfacet theory. 
// This theory was developed to describe reflection from general, non-optically flat surfaces. 
// It models the surface as consisting of small facets that are optically flat (mirrors) and 
// are oriented in various directions. Only those that are oriented correctly to reflect toward 
// the viewer can contribute.
//
// Parameters are the position of a fragment and the surface normal in the world space.
//
vec3 microfacetModel(vec3 position, vec3 n, vec3 lightPos, vec3 lightIntensity, vec3 baseColor) 
{  
    vec3 l = lightPos - position;
    float dist = length(l);
    l = normalize(l);
    lightIntensity *= 1.0 / (dist * dist); 

    vec3 v = normalize(cameraPos - position);
    vec3 h = normalize(v + l);
    float nDotH = dot(n, h);
    float lDotH = dot(l, h);
    float nDotL = max(dot(n, l), 0.0f);
    float nDotV = dot(n, v);

    vec3 fresnel = schlickFresnel(lDotH, baseColor);
    vec3 specBrdf = 0.25f * ggxDistribution(nDotH) * fresnel 
                            * geomSmith(nDotL) * geomSmith(nDotV);
    vec3 diffuseBrdf = baseColor;
    //diffuseBrdf *= 1.0 - cfg.metallic;

    return (diffuseBrdf + PI * specBrdf) * lightIntensity * nDotL;
}