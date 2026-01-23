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

layout(std140, binding = 8) buffer VolumetricFogBuffer {
    FogData[] fogDatas;
};


void main()
{             
    vec4 color = texture2D(scene, textureCoords); 

    // Fog
    uvec2 pixelPos = uvec2(textureCoords.x * screenResolution.x, textureCoords.y * screenResolution.y);
    uint fogIndex = pixelPos.x + pixelPos.y * screenResolution.x;
    color *= fogDatas[fogIndex].transmittance;
    color += vec4(fogDatas[fogIndex].radiance, 0.0f);

    FragColor = color;
    // Vignette
    // float distance = length(vec2(0.5) - textureCoords);
    // float vignetteMultiplier = clamp(vignette - distance, 0, 1);
    // FragColor = vec4(vec3(color) * vignetteMultiplier, color.a);  
    // FragColor = vec4(1.0); 
}  