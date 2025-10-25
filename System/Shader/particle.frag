#version 450 core

uniform sampler2D image;
uniform vec3 color;

in vec2 textureUnit;

out vec4 FragColor;

void main()
{
    vec4 color = vec4(color,1.0) * texture(image, textureUnit);
    FragColor = color;
}