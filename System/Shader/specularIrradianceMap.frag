// https://learnopengl.com/PBR/IBL/Specular-IBL

#version 450 core

out vec4 FragColor;
in vec3 localPos;

uniform samplerCube cubemap;
uniform float roughness;

const float PI = 3.14159265359;
const float resolution = 512; // resolution of source cubemap (per face)

float RadicalInverse_VdC(uint bits);
vec2 Hammersley(uint i, uint N);
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness);
float ggxDistribution(float nDotH, float roughness);

void main() {		
    vec3 normal = normalize(vec3(localPos.x, -localPos.y, localPos.z));
    
    vec3 N = normal;    
    vec3 R = N;
    vec3 V = R;

    const uint SAMPLE_COUNT = 32768u;

#if 0
    float totalWeight = 0.0;   
    vec3 prefilteredColor = vec3(0.0);    

    for(uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        vec3 H  = ImportanceSampleGGX(Xi, N, roughness);

        float NdotH = max(dot(N, H), 0.0);

        float D   = ggxDistribution(NdotH, roughness);
        float pdf = (D * NdotH / (4.0)) + 0.0001; 

        // with a higher resolution, we should sample coarser mipmap levels
        float saTexel = 4.0 * PI / (6.0 * resolution * resolution);
        
        // as we take more samples, we can sample from a finer mipmap.
        // And places where H is more likely to be sampled (higher pdf) we
        // can use a finer mipmap, otherwise use courser mipmap.
        // The tutorial treats this portion as optional to reduce noise but I think it's
        // actually necessary for importance sampling to get the correct result
        float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);

        float mipLevel = 0.5 * log2(saSample / saTexel); 

        prefilteredColor += textureLod(cubemap, H, mipLevel).rgb * NdotH;
        totalWeight += NdotH;
    }

    prefilteredColor = prefilteredColor / totalWeight;
#else
    float totalWeight = 0.0;   
    vec3 prefilteredColor = vec3(0.0);     
 
    for(uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        vec3 H  = ImportanceSampleGGX(Xi, N, roughness);
        vec3 L  = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(dot(N, L), 0.0);

        float NdotH = max(dot(N, H), 0.0);
        float D   = ggxDistribution(NdotH, roughness);
        float pdf = (D * NdotH / (4.0)) + 0.0001; 

        // with a higher resolution, we should sample coarser mipmap levels
        float saTexel  = 4.0 * PI / (6.0 * resolution * resolution);
        float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);

        float mipLevel = roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel); 

        if(NdotL > 0.0)
        {
            prefilteredColor += textureLod(cubemap, L, mipLevel).rgb * NdotL;
            totalWeight      += NdotL;
        }
    }
    prefilteredColor = prefilteredColor / totalWeight;
#endif
    FragColor = vec4(prefilteredColor, 1.0);
} 

// The following 2 functions is responsbile for generating low-discrepancy sequence, utilising Quasi-Monte Carlo method
// for faster convergence.
// https://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
float RadicalInverse_VdC(uint bits) 
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
// ----------------------------------------------------------------------------
vec2 Hammersley(uint i, uint N)
{
    return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}  

// This function is responsible for orienting and biasing our sample vector towards the specular lobe..
// Sample vector is now somewhat oriented around the expected microsurface's halfway vector based on some 
// input roughness and the low-discrepancy sequence value Xi.
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
    float a = roughness*roughness;
	
    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
	
    // from spherical coordinates to cartesian coordinates
    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;
	
    // from tangent-space vector to world-space sample vector
    vec3 up        = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent   = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);
	
    vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
} 

float ggxDistribution(float nDotH, float roughness) 
{
    float alpha2 = roughness * roughness * roughness * roughness;
    float d = (nDotH * nDotH) * (alpha2 - 1.0f) + 1.0f;
    return alpha2 / max(PI * d * d, 0.00000001f);
}