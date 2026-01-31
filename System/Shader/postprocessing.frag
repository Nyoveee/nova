#version 450 core
struct FogData{
    vec3 radiance;
    float transmittance;
};

in vec2 textureCoords;
out vec4 FragColor;

uniform sampler2D scene;

uniform vec3 offset;
uniform float vignette;
uniform uvec2 screenResolution;

uniform vec3 fogColor;

layout(std140, binding = 8) buffer VolumetricFogBuffer {
    FogData[] fogDatas;
};

void main()
{             
    vec3 color = texture2D(scene, textureCoords).rgb; 

    // Fog
    uvec2 pixelPos = uvec2(textureCoords.x * screenResolution.x, textureCoords.y * screenResolution.y);
    uint fogIndex = pixelPos.x + pixelPos.y * screenResolution.x;
    
    float transmittance = clamp(fogDatas[fogIndex].transmittance, 0, 1);

    // blend scene color with fog color.. based on transmittance value..
    color = (color * transmittance) + (fogColor * (1 - transmittance));
    // color = (color * transmittance);
    // color = (fogColor * (1 - transmittance));

    color += fogDatas[fogIndex].radiance;
    
    // color = vec3(transmittance);
    FragColor = vec4(color, 1.0);
}  