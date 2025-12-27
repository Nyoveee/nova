#version 450 core

uniform sampler2D image;

in vec4 color;
in vec2 textureUnit;

out vec4 FragColor;

void main()
{
    FragColor = color * texture(image,textureUnit);
}