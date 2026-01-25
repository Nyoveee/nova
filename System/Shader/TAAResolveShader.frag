#version 450

in vec2 textureCoords;

out vec4 FragColor;

uniform sampler2D scene;
uniform sampler2D historyTexture;

void main() {
    vec3 currentColor = texture(scene, textureCoords).rgb;
    vec3 historyColor = texture(historyTexture, textureCoords).rgb;

    // simple lerp..
    vec3 finalColor = mix(currentColor, historyColor, 0.9);
    FragColor = vec4(finalColor, 1);
}