/**
 * @file    pbrlibrary.frag
 * @brief   File provides library functions, appropriate constants and uniforms required for PBR pipeline.
 *
 * This file provides exposes the necessary variables and functions for our custom shaders to utilises.
 * 
 * Important variables like 
 * - PBRCaculation(...) function, that calculates the final color based on input
 * - Light SSBOs (and in the future shadow maps)
 * - Interpolated input variables (passed from vertex shader, like vertex normals, texture units, fragWorldPos, TBN matrix.)
 * 
 * Refer to this file for the exact items exposed.
 */

#version 450 core

/**
 * @brief Calculates the final color following the PBR lighting model. (based on Cook-Torrance BRDF)
 *
 * @param vec3 albedoColor    The base color of the object.
 * @param vec3 normal         The normal of the object. You can use the vertex normal provided in this pipeline or use a normal map.
 * @param vec3 roughness      Roughness of the object. Ranges from [0, 1].
 * @param vec3 metallic       Metallic of the object. Ranges from [0, 1].
 * @param vec3 occulusion     Occulusion of the object. Ranges from [0, 1]. If unsure, use 0.
 *
 * @return The resulting color of the fragment after through the PBR lighting calculation. 
 */
vec3 PBRCaculation(vec3 albedoColor, vec3 normal, float roughness, float metallic, float occulusion);

// ====================================
// These are set by the pipeline, and exposed. 
// ==================================== 
const float ambientFactor = 0.2;
const float PI = 3.14159265358979323846;

// === LIGHT PROPERTIES ===
struct PointLight {
    vec3 position;
    vec3 color;
    vec3 attenuation;
    float radius;
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
    float radius;
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
out vec4 FragColor;

in VS_OUT {
    vec2 textureUnit;
    vec3 normal;
    vec3 fragWorldPos;
    mat3 TBN;
} fsIn;

// ====================================
// End of pipeline setup. 
// ==================================== 

// ============= Function Declaration =============
float ggxDistribution       (float nDotH);
float geomSmith             (float nDotL);
vec3  schlickFresnel        (float lDotH, vec3 baseColor);
vec3  microfacetModelPoint  (vec3 position, vec3 n, vec3 baseColor, float roughness, float metallic, PointLight light);
vec3  microfacetModelDir    (vec3 position, vec3 n, vec3 baseColor, float roughness, float metallic, DirectionalLight light);
vec3  microfacetModelSpot   (vec3 position, vec3 n, vec3 baseColor, float roughness, float metallic, SpotLight light);
vec3  BRDFCalculation       (vec3 n, vec3 v, vec3 l, vec3 lightIntensity, vec3 baseColor, float roughness, float metallic);

// =================================================================================
// IMPLEMENTATION DETAILS.
// =================================================================================

// !! THE public facing function API to be used.
vec3 PBRCaculation(vec3 albedoColor, vec3 normal, float roughness, float metallic, float occulusion) {
    normal = normalize(normal);

    // ambient is the easiest.
    vec3 finalColor = ambientFactor * albedoColor * (occulusion * 0.04);

    // Calculate diffuse and specular light for each light.
    for(int i = 0; i < pointLightCount; ++i) {
        // Calculate point light contribution using microfacet model
        finalColor += microfacetModelPoint(fsIn.fragWorldPos, normal, albedoColor, roughness, metallic, pointLights[i]);
    }
    for(int i = 0; i < dirLightCount; ++i) {
        finalColor += microfacetModelDir(fsIn.fragWorldPos, normal, albedoColor, roughness, metallic, dirLights[i]);
    }
    for(int i = 0; i < spotLightCount; ++i) {
        finalColor += microfacetModelSpot(fsIn.fragWorldPos, normal, albedoColor, roughness, metallic, spotLights[i]);
    }

    return finalColor;
}

// The Microgeometry Normal Distribution Function, based on GGX/Trowbrodge-Reitz, 
// that describes the relative concentration of microfacet normals 
// in the direction H. It has an effect on the size and shape 
// of the specular highlight.
//
// Parameter is cosine of the angle between the normal and H which is the halfway vector of 
// both the light direction and the view direction
//
float ggxDistribution(float nDotH, float roughness) 
{
    float alpha2 = roughness * roughness * roughness * roughness;
    float d = (nDotH * nDotH) * (alpha2 - 1.0f) + 1.0f;
    return alpha2 / max(PI * d * d, 0.0001f);
}

// The Smith Masking-Shadowing Function describes the probability that microfacets with 
// a given normal are visible from both the light direction and the view direction.
//
// Parameter is cosine of the angle between the normal vector and the view direction.
//
float geomSmith(float nDotL, float roughness) 
{
    float k = (roughness + 1.0f) * (roughness + 1.0f) / 8.0f;
    float denom = nDotL * (1.0f - k) + k;
    return 1.0f / max(denom, 0.0001f);
}

// Schlick approximation for Fresnel reflection that defines the probability of light
// reflected from an optically flat surface.
//
// Parameter is cosine of the angle between the light direction vector and 
// the halfway vector of both the light direction and the view direction
//
vec3 schlickFresnel(float lDotH, vec3 baseColor, float metallic) 
{
    vec3 f0 = vec3(0.04f); // Dielectrics
    f0 = mix(f0, baseColor, metallic);
    return f0 + (1.0f - f0) * pow(1.0f - lDotH, 5);
}

// Calculates light attenuation.
float calculateAttenuation(float distance, vec3 attenutationFactor, float radius) {
    // Normalize distance by the radius for clamping purposes
    float normalized_distance = distance / radius; 

    // Calculate a factor that goes from 1.0 at d=0 to 0.0 at d=AttenuationRadius
    // The pow(..., 4.0) term helps control the curve, making the cutoff smoother/faster than a simple linear or quadratic function
    float factor_clamped = clamp(1.0 - pow(normalized_distance, 4.0), 0.0, 1.0);

    // Smooth the factor (square it)
    float smooth_factor = factor_clamped * factor_clamped;

    // Apply inverse square attenuation with a small divisor to prevent division by zero near the light source
    // attenutationFactor represents the constants in the attenuation calculation. 
    // attenutationFactor.x is Kc, constant, attenutationFactor.y is Kl, linear and attenutationFactor.z is Kq, quadratic.
    // https://learnopengl.com/Lighting/Light-casters
    float inverse_square_attenuation = 1.0 / (attenutationFactor.x + attenutationFactor.y * distance + attenutationFactor.z * (distance * distance)); 

    // Combine both to get the final attenuation
    float attenuation = smooth_factor * inverse_square_attenuation;


    return attenuation;
}

// Calculates the incoming light directional vector & light color for point light.
vec3 microfacetModelPoint(vec3 position, vec3 n, vec3 baseColor, float roughness, float metallic, PointLight light) 
{  
    vec3 l = light.position - position;
    float dist = length(l);
    l = normalize(l);
    vec3 lightIntensity = light.color;
    lightIntensity *= calculateAttenuation(dist, light.attenuation, light.radius); 
    
    vec3 v = normalize(cameraPos - position);
    return BRDFCalculation(n, v, l, lightIntensity, baseColor, roughness, metallic);
}

// Calculates the incoming light directional vector & light color for directional light.
vec3 microfacetModelDir(vec3 position, vec3 n, vec3 baseColor, float roughness, float metallic, DirectionalLight light) 
{  
    vec3 l = normalize(-light.direction);
    vec3 lightIntensity = light.color;
    
    vec3 v = normalize(cameraPos - position);
    return BRDFCalculation(n, v, l, lightIntensity, baseColor, roughness, metallic);
}

// Calculates the incoming light directional vector & light color for spotlights.
vec3 microfacetModelSpot(vec3 position, vec3 n, vec3 baseColor, float roughness, float metallic, SpotLight light) 
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
    lightIntensity *= calculateAttenuation(dist, light.attenuation, light.radius); 
    
    vec3 v = normalize(cameraPos - position);
    return BRDFCalculation(n, v, l, lightIntensity * spotIntensity, baseColor, roughness, metallic);
}

// Bidirectional Reflectance Distribution Function.
// This is the common way to model reflectance based on microfacet theory. 
// This theory was developed to describe reflection from general, non-optically flat surfaces. 
// It models the surface as consisting of small facets that are optically flat (mirrors) and 
// are oriented in various directions. Only those that are oriented correctly to reflect toward 
// the viewer can contribute.
vec3 BRDFCalculation(vec3 n, vec3 v, vec3 l, vec3 lightIntensity, vec3 baseColor, float roughness, float metallic)
{
    vec3 h = normalize(v + l);
    float nDotH = max(dot(n, h), 0.0f);
    float lDotH = max(dot(l, h), 0.0f);
    float nDotL = max(dot(n, l), 0.0f);
    float nDotV = max(dot(n, v), 0.0f);

    vec3 fresnel = schlickFresnel(lDotH, baseColor, metallic);
    vec3 specBrdf = 0.25f * ggxDistribution(nDotH, roughness) * fresnel * geomSmith(nDotL, roughness) * geomSmith(nDotV, roughness);
    vec3 diffuseBrdf = vec3(1.0) - fresnel;
    diffuseBrdf *= 1.0 - metallic;

    return (diffuseBrdf * baseColor / PI + specBrdf) * lightIntensity * nDotL;
}