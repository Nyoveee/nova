#version 450 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D image;
uniform vec3 tintColor;

void main()
{    
    FragColor = texture(image, TexCoords) * vec4(tintColor, 1.0);
}  