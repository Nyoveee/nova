#version 450 core

uniform sampler2D overlay;

in vec2 textureCoords;
out vec4 FragColor;

const float gamma = 2.2;

void main()
{
    vec4 finalColor = texture(overlay, textureCoords);
    FragColor = vec4(pow(finalColor.rgb, vec3(1.0 / gamma)), finalColor.a);
}