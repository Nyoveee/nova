#version 450 core
out float FragColor;
  
in vec2 textureCoords;

uniform sampler2D depthMap;
uniform sampler2D normalMap;
uniform sampler2D noiseTexture;

uniform vec2 screenDimensions;


layout(std140, binding = 0) uniform Camera {
    mat4 view;
    mat4 projection;
    mat4 cameraProjectionView;
    vec4 samples[64];
};

const int kernelSize = 64;
const float radius = 0.5;

vec3 WorldPositionFromDepth(float depth, vec2 textureCoords) {
    // 1. Map to NDC space [-1, 1]
    float z = depth * 2.0 - 1.0;
    vec4 clipSpacePos = vec4(textureCoords * 2.0 - 1.0, z, 1.0);
    
    // 2. Transform to World Space
    vec4 worldSpacePos = inverse(cameraProjectionView) * clipSpacePos;
    
    // 3. Perspective Division
    return worldSpacePos.xyz / worldSpacePos.w;
}

void main() {
    // tile noise texture over screen, based on screen dimensions divided by noise size
    const vec2 noiseScale = screenDimensions / 4.0;

    float depth    = texture(depthMap, textureCoords).r;
    vec3 normal    = texture(normalMap, textureCoords).rgb;
    vec3 randomVec = texture(noiseTexture, textureCoords * noiseScale).rgb;  

    vec3 worldPosition = WorldPositionFromDepth(depth, textureCoords);

    // learnopengl follows view space, but we are following world space
    vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN       = mat3(tangent, bitangent, normal);  // world space TBN.

    float occlusion = 0.0;

    for(int i = 0; i < kernelSize; ++i) {
        // we retrieve a random sample, and offset from the current fragment..
        vec3 samplePos = TBN * vec3(samples[i]);          // from tangent to world space
        samplePos = worldPosition + samplePos * radius; 

        // we convert our sample position to screen space, so we can query for depth.
        vec4 offset = vec4(samplePos, 1.0);
        offset      = cameraProjectionView * offset;    // from world to clip-space
        offset.xyz /= offset.w;                         // perspective divide
        offset.xyz  = offset.xyz * 0.5 + 0.5;           // transform to range 0.0 - 1.0  

        // depth of the sampled fragment
        float sampleDepth = texture(depthMap, offset.xy).r; 

        const float bias = 0.025;
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(worldPosition.z - sampleDepth));
        occlusion  += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck; 
    }  

    occlusion = 1.0 - (occlusion / kernelSize);
    // FragColor = occlusion;  

    FragColor = texture(depthMap, textureCoords).r; 
}