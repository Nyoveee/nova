#version 450 core

uniform sampler2D image;
uniform uint objectId;

in VS_OUT {
    vec2 textureUnit;
} fsIn;

layout (location = 0) out vec4 FragColor;

void main()
{
    vec4 color = texture(image, fsIn.textureUnit);
    FragColor = color;
}