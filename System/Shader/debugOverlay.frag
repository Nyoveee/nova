#version 450 core

uniform sampler2D debugOverlay;

in vec2 textureCoords;
out vec4 FragColor;

void main()
{
    FragColor = texture(debugOverlay, textureCoords);
}