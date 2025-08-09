#version 450 core

in vec2 textureUnit;
out vec4 FragColor;

uniform sampler2D image;

void main()
{
    FragColor = texture(image, textureUnit);
}