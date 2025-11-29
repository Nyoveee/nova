#version 450 core

in vec2 textureCoords;
out vec4 FragColor;

uniform sampler2D scene;

uniform vec3 offset;
uniform float vignette;

void main()
{             
    vec4 color = texture2D(scene, textureCoords); 

    float distance = length(vec2(0.5) - textureCoords);

    float vignetteMultiplier = clamp(vignette - distance, 0, 1);
    FragColor = vec4(vec3(color) * vignetteMultiplier, color.a);  

    // FragColor = vec4(vec3(1.0 - texture(scene, textureCoords)), 1.0);
}  