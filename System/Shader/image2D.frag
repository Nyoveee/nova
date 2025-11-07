#version 450 core

in vec2 textureCoords;
out vec4 FragColor;

uniform sampler2D image;
uniform vec4 tintColor;

void main()
{    
    FragColor = texture(image, textureCoords) * tintColor;

}  