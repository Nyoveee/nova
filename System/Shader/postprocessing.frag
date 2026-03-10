#version 450 core
struct FogData{
    vec3 radiance;
    float transmittance;
};

in vec2 textureCoords;
out vec4 FragColor;

uniform sampler2D scene;
uniform sampler2D depthTexture;

uniform float vignette;
uniform vec3 vignetteColor;

uniform vec3 fogColor;

uniform float fogNear;
uniform float fogFar;
uniform float fogDensity;

uniform bool isFogEnabled;

uniform bool isBlurEnabled;

uniform bool chromaticAberration;
uniform vec3 chromaticAberrationOffset;

layout(std140, binding = 8) buffer VolumetricFogBuffer {
    FogData[] fogDatas;
};

// Uniform Buffers
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

float linearizeDepth(float depth) {
    // Example of reconstructing View-Space Z from a [0,1] Depth Map
    float ndcDepth = depth * 2.0 - 1.0; 
    float viewZ = (2.0 * zNear * zFar) / (zFar + zNear - ndcDepth * (zFar - zNear));

    return viewZ;
}

const float blurKernel[9] = float[](
    1.0 / 16, 2.0 / 16, 1.0 / 16,
    2.0 / 16, 4.0 / 16, 2.0 / 16,
    1.0 / 16, 2.0 / 16, 1.0 / 16  
);

const float offset = 1.0 / 300.0;  

const vec2 offsets[9] = vec2[](
    vec2(-offset,  offset), // top-left
    vec2( 0.0f,    offset), // top-center
    vec2( offset,  offset), // top-right
    vec2(-offset,  0.0f),   // center-left
    vec2( 0.0f,    0.0f),   // center-center
    vec2( offset,  0.0f),   // center-right
    vec2(-offset, -offset), // bottom-left
    vec2( 0.0f,   -offset), // bottom-center
    vec2( offset, -offset)  // bottom-right    
);

void main()
{   
    vec3 color = vec3(0.0);;

    // BLUR..
    if (isBlurEnabled) {
        vec3 sampleTex[9];

        for(int i = 0; i < 9; i++)
        {
            sampleTex[i] = vec3(texture(scene, textureCoords + offsets[i]));
        }
        
        for(int i = 0; i < 9; i++) {
            color += sampleTex[i] * blurKernel[i];
        }
    }
    // Chromatic Aberration..
    else if (chromaticAberration) {
        color.r = texture2D(scene, textureCoords + chromaticAberrationOffset.x).r;
        color.g = texture2D(scene, textureCoords + chromaticAberrationOffset.y).g;
        color.b = texture2D(scene, textureCoords + chromaticAberrationOffset.z).b;
    }
    else {
        color = texture2D(scene, textureCoords).rgb; 
    }

    if (isFogEnabled) {
        float linearDepth = linearizeDepth(texture2D(depthTexture, textureCoords).r);

        float near = max(zNear, fogNear);
        float far = min(zFar, fogFar);

        float linearDistanceFactor = clamp((linearDepth - near)/ (far - near), 0, 1);
        float fogFactor = clamp(exp(-(linearDistanceFactor * fogDensity * linearDistanceFactor * fogDensity)), 0, 1);

        color = mix(fogColor, color, fogFactor);
    }

    float dist = distance(vec2(0.5, 0.5), textureCoords);

    FragColor = vec4(mix(color, vignetteColor, clamp(smoothstep(0, 1, dist) * vignette, 0, 1)), 1.0);
}  