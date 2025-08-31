#version 450 core

uniform sampler2D image;
uniform uint objectId;

in vec2 textureUnit;

layout (location = 0) out vec4 FragColor;

void main()
{
    vec4 color = texture(image, textureUnit);
    FragColor = color;
}