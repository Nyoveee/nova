#version 450 core

out vec4 FragColor;

in VS_OUT {
    vec2 textureUnit;
    vec3 normal;
    vec3 fragWorldPos;
    vec4 fragPosLightSpace;
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

// LIGHT FUNCTIONS
float ggxDistribution(float nDotH);
float geomSmith(float nDotL) ;
vec3 schlickFresnel(float lDotH, vec3 baseColor);
vec3 microfacetModelPoint(vec3 position, vec3 n, vec3 baseColor, PointLight light);
vec3 microfacetModelDir(vec3 position, vec3 n, vec3 baseColor, DirectionalLight light);
vec3 microfacetModelSpot(vec3 position, vec3 n, vec3 baseColor, SpotLight light);
// Helper function
vec3 BRDFCalculation(vec3 n, vec3 v, vec3 l, vec3 lightIntensity, vec3 baseColor);

// === NORMAL MAPPING ===
uniform bool isUsingNormalMap;
uniform sampler2D normalMap;

// === SHADOW MAPPING ===
uniform sampler2D shadowMap;
uniform bool enableShadows;

float calculateShadow(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    // Perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    // Outside the light's frustum
    if(projCoords.z > 1.0)
        return 0.0;

    // Get closest depth value from light's perspective
    float closestDepth = texture(shadowMap, projCoords.xy).r;

    // Get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    // Add bias to reduce shadow acne
    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.001);

    // PCF (Percentage Closer Filtering) for softer shadows
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    return shadow;
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
    vec3 finalColor = ambientFactor * baseColor * cfg.occulusion;

    // Calculate diffuse and specular light for each light.
    for(int i = 0; i < pointLightCount; ++i) {
        // Calculate point light contribution using microfacet model
        finalColor += microfacetModelPoint(fsIn.fragWorldPos, normal, baseColor, pointLights[i]);
    }
    for(int i = 0; i < dirLightCount; ++i) {
        vec3 lightContribution = microfacetModelDir(fsIn.fragWorldPos, normal, baseColor, dirLights[i]);

        // Apply shadows only to the first directional light (for now)
        if(i == 0 && enableShadows) {
            vec3 lightDir = normalize(-dirLights[i].direction);
            float shadow = calculateShadow(fsIn.fragPosLightSpace, normal, lightDir);
            lightContribution *= (1.0 - shadow);
        }

        finalColor += lightContribution;
    }
    for(int i = 0; i < spotLightCount; ++i) {
        finalColor += microfacetModelSpot(fsIn.fragWorldPos, normal, baseColor, spotLights[i]);
    }

    // Output in linear space (HDR) - gamma correction happens in tone mapping pass
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
vec3 microfacetModelPoint(vec3 position, vec3 n, vec3 baseColor, PointLight light) 
{  
    vec3 l = light.position - position;
    float dist = length(l);
    l = normalize(l);
    vec3 lightIntensity = light.color;
    lightIntensity *= 1.0 / (dist * dist); 
    
    vec3 v = normalize(cameraPos - position);
    return BRDFCalculation(n, v, l, lightIntensity, baseColor);
}

vec3 microfacetModelDir(vec3 position, vec3 n, vec3 baseColor, DirectionalLight light) 
{  
    vec3 l = normalize(-light.direction);
    vec3 lightIntensity = light.color;
    
    vec3 v = normalize(cameraPos - position);
    return BRDFCalculation(n, v, l, lightIntensity, baseColor);
}

vec3 microfacetModelSpot(vec3 position, vec3 n, vec3 baseColor, SpotLight light) 
{  
    vec3 l = light.position - position;
    float dist = length(l);
    l = normalize(l);
    // Cutoff angles for spotlight
    float cutOffAngle = cos(light.cutOffAngle);
    float outerCutOffAngle = cos(light.outerCutOffAngle);
    float theta     = dot(l, normalize(-light.direction));
    float epsilon   = cutOffAngle - outerCutOffAngle;
    float spotIntensity = (theta - outerCutOffAngle) / epsilon; 

    // Early return if outside spotlight
    if (spotIntensity <= 0.0) {
        return vec3(0.0);
    }
    spotIntensity = clamp(spotIntensity, 0.0, 1.0);
    vec3 lightIntensity = light.color;
    lightIntensity *= 1.0 / (dist * dist); 
    
    vec3 v = normalize(cameraPos - position);
    return BRDFCalculation(n, v, l, lightIntensity * spotIntensity, baseColor);
}

vec3 BRDFCalculation(vec3 n, vec3 v, vec3 l, vec3 lightIntensity, vec3 baseColor)
{
    vec3 h = normalize(v + l);
    float nDotH = dot(n, h);
    float lDotH = dot(l, h);
    float nDotL = max(dot(n, l), 0.0f);
    float nDotV = dot(n, v);

    vec3 fresnel = schlickFresnel(lDotH, baseColor);
    vec3 specBrdf = 0.25f * ggxDistribution(nDotH) * fresnel 
                            * geomSmith(nDotL) * geomSmith(nDotV);
    vec3 diffuseBrdf = vec3(1.0) - fresnel;
    diffuseBrdf *= 1.0 - cfg.metallic;

    return (diffuseBrdf * baseColor / PI + specBrdf) * lightIntensity * nDotL;
    
}