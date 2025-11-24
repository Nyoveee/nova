#version 450 core


in vec2 textureCoords;
out vec4 FragColor;

uniform sampler2D scene;
uniform sampler2D bloomBlur;
uniform float compositePercentage;

void main()
{             
    vec4 originalScene = texture(scene, textureCoords);
    vec3 originalSceneColor = originalScene.rgb;     
 
    vec3 bloomColor = texture(bloomBlur, textureCoords).rgb;
    originalSceneColor += compositePercentage * bloomColor; // additive blending

    FragColor = vec4(originalSceneColor, originalScene.a);
}  