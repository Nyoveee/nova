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
const float PI = 3.14159265358979323846;

// === LIGHT PROPERTIES ===
struct Cluster
{
    vec4 minPoint;
    vec4 maxPoint;
    uint pointLightCount;
    uint spotLightCount;
    uint reflectionProbesCount;
    uint pointLightIndices[25];
    uint spotLightIndices[25];
    uint reflectionProbesIndices[4];
};

struct PointLight{
    vec3 position;		
    vec3 viewPosition;
    vec3 color;		
    vec3 attenuation; 
    float radius;
    float intensity;
    int shadowMapIndex;
};

struct SpotLight {
    vec3 position;
    vec3 viewPosition;
    vec3 direction;
    vec3 viewDirection;
    vec3 color;
    vec3 attenuation;
	float cutOffAngle;
	float outerCutOffAngle;
    float radius;
    float intensity;
    int shadowMapIndex;
};

struct DirectionalLight {
    vec3 direction;
    vec3 color;
};

struct ReflectionProbe {
	vec3 worldMin;
	vec3 worldMax;
    vec3 viewMin;
	vec3 viewMax;
    vec3 worldProbePosition;
    int indexToProbeArray;
    float blendFallOff;
    float intensity;
};

const int MAX_SHADOW_CASTER = 15;

layout(std140, binding = 0) uniform Camera {
    mat4 view;
    mat4 projection;
    mat4 cameraProjectionView;
    mat4 inverseView;
    mat4 inverseProjection;
    mat4 inverseProjectionView;
    mat4 previousViewProjection;    // for TAA

    vec3 cameraPosition;

    uvec3 gridSize;
    uvec2 screenDimensions;
    float zNear;
    float zFar;
};

layout(std140, binding = 1) uniform ShadowCasterMatrixes {
    mat4 shadowCasterMatrix[MAX_SHADOW_CASTER];
};

layout(std140, binding = 2) uniform PBRUBO {
    vec4 samples[64];   
	mat4 directionalLightSpaceMatrix;
	vec3 directionalLightDir;
	float timeElapsed;
	bool toEnableSSAO;
	bool hasDirectionalLightShadowCaster;
	bool toEnableIBL;
	bool toOutputNormal;
    float iblDiffuseStrength;
    float iblSpecularStrength;
};

layout(std140, binding = 3) uniform ReflectionProbes {
    uint reflectionProbesCount;
    ReflectionProbe reflectionProbes[30];
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

layout(std430, binding = 6) buffer LightParticleList {
    int lightParticleCount;
    bool b_AmountExceeded;
    PointLight particlePointLights[];
};

layout(std430, binding = 7) buffer clusterSSBO
{
    Cluster clusters[];
};
layout(std430, binding = 9) buffer OldBones {
    mat4 oldBonesFinalMatrices[];
};

// Samplers..
layout (binding = 0) uniform sampler2D directionalShadowMap;
layout (binding = 1) uniform sampler2D ssao;
layout (binding = 2) uniform sampler2D brdfLUT;
layout (binding = 3) uniform sampler2D depthTexture;
layout (binding = 4) uniform sampler2DArray spotlightShadowMaps;    
layout (binding = 5) uniform samplerCube diffuseIrradianceMap;
layout (binding = 6) uniform samplerCube prefilterMap;
layout (binding = 7) uniform samplerCubeArray reflectionProbesPrefilterMap;

layout (location = 0) out vec4 FragColor; 
layout (location = 1) out vec3 gNormal; // for depth pre pass..
layout (location = 2) out vec2 velocityUV;

in VS_OUT {
    vec2 textureUnit;
    vec3 normal;
    vec3 fragWorldPos;
    vec3 fragViewPos;
    vec4 fragDirectionalLightPos;
    vec4 fragOldClipPos;
    vec4 fragCurrentClipPos;
    vec3 boundingBoxUVW;
    mat3 TBN;
} fsIn;

// explicit location.
layout (location = 20) uniform bool toUseNormalMap;

// ====================================
// End of pipeline setup. 
// ==================================== 

vec2 UVTileAndOffset(vec2 textureCoordinates, vec2 UVTiling, vec2 UVOffset) {
    return textureCoordinates * UVTiling + UVOffset;
}

// ============= Function Declaration =============
float ggxDistribution               (float nDotH);
float geomSmith                     (float nDotL);
vec3  schlickFresnel                (float lDotH, vec3 baseColor);
vec3  fresnelSchlickRoughness       (float cosTheta, vec3 F0, float roughness);
vec3  microfacetModelPoint          (vec3 position, vec3 n, vec3 baseColor, float roughness, float metallic, PointLight light);
vec3  microfacetModelDir            (vec3 position, vec3 n, vec3 baseColor, float roughness, float metallic, DirectionalLight light);
vec3  microfacetModelSpot           (vec3 position, vec3 n, vec3 baseColor, float roughness, float metallic, SpotLight light);
vec3  BRDFCalculation               (vec3 n, vec3 v, vec3 l, vec3 lightIntensity, vec3 baseColor, float roughness, float metallic);

float directionalShadowCalculation  (sampler2D shadowMap, vec4 fragmentLightPos);
float spotlightShadowCalculation    (sampler2DArray spotlightShadowMaps, int shadowMapIndex, vec4 fragmentLightPos);
float getShadowFactor               (int lightShadowIndex);
Cluster getCluster                  ();

vec3 getPrefilteredColor            (Cluster cluster, float roughness, vec3 reflectDir);

vec3 boxProjectionReflection        (vec3 reflectDir, vec3 fragmentPos, vec3 probeMin, vec3 probeMax, vec3 probePos);
float getBlendFactor                (vec3 position, ReflectionProbe reflectionProbe);

float linearizeDepth                (float depth);
vec3  getNormalFromMap              (sampler2D normalMap, vec2 uv); 

// =================================================================================
// IMPLEMENTATION DETAILS.
// =================================================================================

// !! THE public facing function API to be used.
vec3 PBRCaculation(vec3 albedoColor, vec3 normal, float roughness, float metallic, float occulusion) {
    normal = normalize(normal);

    vec3 finalColor = vec3(0.0, 0.0, 0.0);

    // Locating which cluster this fragment is part of
    Cluster cluster = getCluster();

    // for(uint i = 0; i < cluster.reflectionProbesCount; ++i) {
    //     ReflectionProbe reflectionProbe = reflectionProbes[cluster.reflectionProbesIndices[i]];
    //     return vec3(getBlendFactor(fsIn.fragWorldPos, reflectionProbe));
    // }

    // --------------------------------------------------------------------
    // Calculate direct diffuse and specular light for each light.
    for(uint i = 0; i < cluster.pointLightCount; ++i) {
        PointLight pointLight = pointLights[cluster.pointLightIndices[i]];
        finalColor += microfacetModelPoint(fsIn.fragWorldPos, normal, albedoColor, roughness, metallic, pointLight);
    }

    for(uint i = 0; i < cluster.spotLightCount; ++i) {
        SpotLight spotLight = spotLights[cluster.spotLightIndices[i]];

        vec3 directLight = microfacetModelSpot(fsIn.fragWorldPos, normal, albedoColor, roughness, metallic, spotLight);
        directLight *= (1.0 - getShadowFactor(spotLight.shadowMapIndex));

        finalColor += directLight;
    }

    // particle point light..
    for(int i = 0; i < lightParticleCount; ++i) {
        finalColor += microfacetModelPoint(fsIn.fragWorldPos, normal, albedoColor, roughness, metallic, particlePointLights[i]);
    }

    // No clustering for directional lights, it affects all fragments.
    for(int i = 0; i < dirLightCount; ++i) {
        finalColor += microfacetModelDir(fsIn.fragWorldPos, normal, albedoColor, roughness, metallic, dirLights[i]);
    }

    // --------------------------------------------------------------------
    // We calculate the fragment's shadow factor due to directional light..
    if(hasDirectionalLightShadowCaster) {
        finalColor *= (1.0 - directionalShadowCalculation(directionalShadowMap, fsIn.fragDirectionalLightPos));
    }

    // --------------------------------------------------------------------
    // Indirect lighting
    vec3 ambient = vec3(0);

    if(toEnableIBL) {
        vec3 viewDir = normalize(cameraPosition - fsIn.fragWorldPos);
        float NdotV = max(dot(normal, viewDir), 0.0);

        vec3 reflectDir = reflect(-viewDir, normal);   

        // Get specular and diffuse components respectively..
        vec3 F0         = vec3(0.04); // Dielectrics
        F0              = mix(F0, albedoColor, metallic);

        vec3 kS         = fresnelSchlickRoughness(NdotV, F0, roughness); 
        vec3 kD         = vec3(1.0) - kS;
        kD              *= 1.0 - metallic;
        
        // IBL Diffuse..
        vec3 irradiance = texture(diffuseIrradianceMap, normal).rgb;
        vec3 diffuse    = irradiance * albedoColor * occulusion * iblDiffuseStrength;

        // IBL Specular..
        vec2 envBRDF  = texture(brdfLUT, vec2(NdotV, roughness)).rg;
        vec3 prefilteredColor = getPrefilteredColor(cluster, roughness, reflectDir);  
        vec3 specular = prefilteredColor * (kS * envBRDF.x + envBRDF.y) * iblSpecularStrength;

        ambient         = (kD * diffuse + specular); 
    }
    else {
        ambient = albedoColor * 0.01 * occulusion;
    }

    // --------------------------------------------------------------------
    // SSAO
    if(toEnableSSAO) {
        // Because SSAO is screen space, we need a way to retrieve SSAO texture in screenspace.
        vec2 screenSpaceTextureCoordinates = gl_FragCoord.xy / screenDimensions;
        float ambientOcculusion = texture(ssao, screenSpaceTextureCoordinates).r;
        ambient *= ambientOcculusion; 
    }

    finalColor += ambient;
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
    return alpha2 / max(PI * d * d, 0.00000001f);
}

// The Smith Masking-Shadowing Function describes th    e probability that microfacets with 
// a given normal are visible from both the light direction and the view direction.
//
// Parameter is cosine of the angle between the normal vector and the view direction.
//
float geomSmith(float nDotL, float roughness) 
{
    float k = (roughness + 1.0f) * (roughness + 1.0f) / 8.0f;
    float denom = nDotL * (1.0f - k) + k;
    return 1.0f / max(denom, 0.00000001f);
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

// Schlick approximation for Fresnel reflection, but for diffuse indirect lighting.
// https://seblagarde.wordpress.com/2011/08/17/hello-world/
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
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
    vec3 lightIntensity = light.color * light.intensity;
    lightIntensity *= calculateAttenuation(dist, light.attenuation, light.radius); 
    
    vec3 v = normalize(cameraPosition - position);
    return BRDFCalculation(n, v, l, lightIntensity, baseColor, roughness, metallic);
}

// Calculates the incoming light directional vector & light color for directional light.
vec3 microfacetModelDir(vec3 position, vec3 n, vec3 baseColor, float roughness, float metallic, DirectionalLight light) 
{  
    vec3 l = normalize(-light.direction);
    vec3 lightIntensity = light.color;
    
    vec3 v = normalize(cameraPosition - position);
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
    vec3 lightIntensity = light.color * light.intensity;
    lightIntensity *= calculateAttenuation(dist, light.attenuation, light.radius); 
    
    vec3 v = normalize(cameraPosition - position);
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

Cluster getCluster() {
    uint zTile = uint((log(abs(fsIn.fragViewPos.z) / zNear) * gridSize.z) / log(zFar / zNear));      // we find the z value of this tile..
    vec2 tileSize = screenDimensions / gridSize.xy;                                     
    uvec3 tile = uvec3(gl_FragCoord.xy / tileSize, zTile);                                  // we found our tile index for x, y and z.
    uint tileIndex = tile.x + (tile.y * gridSize.x) + (tile.z * gridSize.x * gridSize.y);   // convert it to index.

    return clusters[tileIndex];
}

vec3 getProjectionCoords(vec4 fragmentLightPos) {
    // We perform perspective divide on fragment light position.
    vec3 projCoords = fragmentLightPos.xyz / fragmentLightPos.w;

    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    return projCoords;
}

float directionalShadowCalculation(sampler2D shadowMap, vec4 fragmentLightPos) {
    vec3 projCoords = getProjectionCoords(fragmentLightPos);
    
    // Fragment is outside of the shadow map.
    if(projCoords.z > 1.0 || projCoords.z < 0.0) {
        return 0.0;
    }
    
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    
    // check whether current frag pos is in shadow
    float bias = 0.003;  
    
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

float spotlightShadowCalculation(sampler2DArray spotlightShadowMaps, int shadowMapIndex, vec4 fragmentLightPos) {
    vec3 projCoords = getProjectionCoords(fragmentLightPos);
    
    // Fragment is outside of the shadow map.
    if(projCoords.z > 1.0 || projCoords.z < 0.0) {
        return 0.0;
    }
    
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    
    // check whether current frag pos is in shadow
    float bias = 0;  
    
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(spotlightShadowMaps, 0).xy;
    
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(spotlightShadowMaps, vec3(projCoords.xy + vec2(x, y) * texelSize, float(shadowMapIndex))).r; 
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }

    shadow /= 9.0;

    return shadow;
}

float getShadowFactor(int lightShadowIndex) {    
    // doesn't have an active shadow map..
    if(lightShadowIndex == -1) {
        return 0;
    }

    // retrieve shadow caster view projection matrix, to transform fragment world position to light caster's space.
    vec4 fragmentLightPos = shadowCasterMatrix[lightShadowIndex] * vec4(fsIn.fragWorldPos, 1);

    return spotlightShadowCalculation(spotlightShadowMaps, lightShadowIndex, fragmentLightPos);
}

bool is_within_range(vec3 value, vec3 min_val, vec3 max_val) {
    // Check if all components of 'value' are >= all components of 'min_val'
    bvec3 greater_than_min = greaterThanEqual(value, min_val);

    // Check if all components of 'value' are <= all components of 'max_val'
    bvec3 less_than_max = lessThanEqual(value, max_val);

    // Return true only if both conditions are true for ALL components
    return all(greater_than_min) && all(less_than_max);
}

// box projection..
// these are in world space..
vec3 boxProjectionReflection(vec3 reflectDir, vec3 fragmentPos, vec3 probeMin, vec3 probeMax, vec3 probePos) {
    // https://www.gamedev.net/forums/topic/568829-box-projected-cubemap-environment-mapping/
    // 1. Find the intersection from fragmentPos going in the reflectDir direction to probe's AABB

    // BPCEM (Box Projected Cubemap Environment Map)
    vec3 tMin = (probeMin - fragmentPos) / reflectDir;
    vec3 tMax = (probeMax - fragmentPos) / reflectDir;

    vec3 t1 = min(tMin, tMax);
    vec3 t2 = max(tMin, tMax);

    // X(t) = P + tHit * r; (ray equation indicating intersection)
    float tHit = min(t2.x, min(t2.y, t2.z));
    vec3 intersection = fragmentPos + reflectDir * tHit;

    // 2. Return new reflection direction from probe position to intersection 
    return normalize(intersection - probePos);
}

// https://iquilezles.org/articles/distfunctions/ [Box SDF]
// https://seblagarde.wordpress.com/2012/09/29/image-based-lighting-approaches-and-parallax-corrected-cubemap/ [Local cubemaps blending weights calculation]
// ^ inspiration..
float getBlendFactor(vec3 position, ReflectionProbe reflectionProbe) {
    // The general idea is to calculate distance a given fragment is to the reflection probe's AABB center.
    vec3 boxExtents = (reflectionProbe.worldMax - reflectionProbe.worldMin) / 2.0;

    // We first transform fragment's world position to the reflection probe's local space..
    vec3 localPosition = position - reflectionProbe.worldProbePosition;

    // We don't really want distance, but some normalized factor ranging from [0, 1]..
    // So let's scale down our local space by the extents of the AABB.
    localPosition /= boxExtents;

    // We focus on the positive quadrant..
    localPosition = abs(localPosition);

    // We want to calculate an appropriate factor, but not via vector length.
    // We want to make sure that points at the boundary is 1, even at diagonals..
    // We follow how SDF implements with maxcomp
    float factor = max(localPosition.x, max(localPosition.y, localPosition.z));

    // This factor is based on distance, so let's "inverse" it.. (if distance is 0, factor is highest..)
    factor = max(1.0 - factor, 0.0);
    
    // We scale factor based on blend fall off, so fragment with high factor will be multiplied and stay at 1.
    // This is how we ensure factor is 1 when inside the blend fall off.
    factor = min(factor / max(reflectionProbe.blendFallOff, 0.0001), 1);

    return factor;
}

// IBL, Reflection probe
vec3 getPrefilteredColor(Cluster cluster, float roughness, vec3 reflectDir) {
    const float MAX_REFLECTION_LOD = 4.0;

    int indexToReflectionProbeMap = -1;

    // We collate all reflection probes that is influencing..
    struct InfluencingProbe {
        uint index; // to UBO of reflection probes..
        float blendFactor;
    };

    InfluencingProbe influencingProbes[4];
    uint numOfInfluencingProbes = 0;
    float totalBlendFactor = 0; // we keep this for normalisation, weigted factors..

    // For each fragment, let's find the nearby reflection probe that influences it.. and calculate their blending factor..
    for(uint i = 0; i < cluster.reflectionProbesCount; ++i) {
        ReflectionProbe reflectionProbe = reflectionProbes[cluster.reflectionProbesIndices[i]];
        
        // Calculate blend factor for this reflection probe..
        float blendFactor = getBlendFactor(fsIn.fragWorldPos, reflectionProbe);

        // Outside the reflection probe's influence..
        if(blendFactor == 0.0) {
            continue;
        }

        // Within a reflection probe..
        influencingProbes[numOfInfluencingProbes].index = cluster.reflectionProbesIndices[i];
        influencingProbes[numOfInfluencingProbes].blendFactor = blendFactor;
        totalBlendFactor += blendFactor;
        numOfInfluencingProbes++;
    }
    
    // We add environment into the mix if there's 0 or 1 reflection probe affecting it.. (resulting totalBlendFactor < 0)
    float environmentBlendFactor = 0;

    if(totalBlendFactor < 1) {
        environmentBlendFactor = max(1 - totalBlendFactor, 0); // jic 
        totalBlendFactor = 1;
    }

    vec3 finalPrefilteredColor = vec3(0.0);

    // We reweight the blend factors, by normalising them.. then adding the colours up..
    for(uint i = 0; i < numOfInfluencingProbes; ++i) {
        InfluencingProbe influenceProbe = influencingProbes[i];
        influenceProbe.blendFactor /= totalBlendFactor;

        ReflectionProbe reflectionProbe = reflectionProbes[influenceProbe.index];
        
        // Get index to loaded reflection probes..
        indexToReflectionProbeMap = reflectionProbe.indexToProbeArray;

        // Calculate box projected reflection..
        vec3 boxProjectedReflection = boxProjectionReflection(reflectDir, fsIn.fragWorldPos, reflectionProbe.worldMin, reflectionProbe.worldMax, reflectionProbe.worldProbePosition);

        // Sample from reflection probe map..
        finalPrefilteredColor += textureLod(reflectionProbesPrefilterMap, vec4(boxProjectedReflection, indexToReflectionProbeMap), roughness * MAX_REFLECTION_LOD).rgb * influenceProbe.blendFactor * reflectionProbe.intensity;
    }
    
    // Blend in environment if appropriate..
    if(environmentBlendFactor > 0) {
        finalPrefilteredColor += textureLod(prefilterMap, reflectDir, roughness * MAX_REFLECTION_LOD).rgb * environmentBlendFactor;
    }

    return finalPrefilteredColor;
}

// https://sugulee.wordpress.com/2021/06/21/temporal-anti-aliasingtaa-tutorial/
vec2 calculateVelocityUV(vec4 fragCurrentClipPos, vec4 fragOldClipPos) {
    fragOldClipPos /= fragOldClipPos.w;                             // perspective divide..
    fragOldClipPos.xy = (fragOldClipPos.xy + 1.0) / 2.0;            // transform [-1, 1] to [0, 1] (uv range)
    
    fragCurrentClipPos /= fragCurrentClipPos.w;                     // perspective divide..
    fragCurrentClipPos.xy = (fragCurrentClipPos.xy + 1.0) / 2.0;    // transform [-1, 1] to [0, 1] (uv range)
    
    // return delta..
    return (fragCurrentClipPos - fragOldClipPos).xy;
}

vec3 getNormalFromMap(sampler2D normalMap, vec2 uv) {
    // We assume that our normal map is compressed into BC5.
    // Since BC5 only stores 2 channels, we need to calculate z in runtime.
    vec2 bc5Channels = vec2(texture(normalMap, uv));
    
    // We shift the range from [0, 1] to  [-1, 1]
    bc5Channels = bc5Channels * 2.0 - 1.0; 

    // We calculate the z portion of the normal..
    vec3 sampledNormal = vec3(bc5Channels, sqrt(max(0.0, 1.0 - dot(bc5Channels.xy, bc5Channels.xy))));
    return normalize(fsIn.TBN * sampledNormal);
}

float linearizeDepth(float depth) {
    float ndcDepth = depth * 2.0 - 1.0; 
    return (2.0 * zNear * zFar) / (zFar + zNear - ndcDepth * (zFar - zNear));
}

// User shader entry point.
vec4 __internal__main__();

// Wrapper around user entry point.
void main() { 
    if(toOutputNormal) {
        gNormal = fsIn.normal;
        velocityUV = calculateVelocityUV(fsIn.fragCurrentClipPos, fsIn.fragOldClipPos);
    }
    else {
        FragColor = __internal__main__();
        // FragColor = vec4(calculateVelocityUV(fsIn.fragCurrentClipPos, fsIn.fragOldClipPos) * 10, 0, 1);
    }
}