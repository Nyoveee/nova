#version 450 core

uniform sampler2D overlay;

in vec2 textureCoords;
out vec4 FragColor;

void main()
{
    FragColor = texture(overlay, textureCoords);
}