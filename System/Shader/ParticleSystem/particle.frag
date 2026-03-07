#version 450 core

uniform sampler2D image;

in vec4 color;
in vec2 textureUnit;

out vec4 FragColor;

void main()
{
    vec4 textureColor = texture(image, textureUnit);
    
    // reverse premultiply alpha..
    if(textureColor.a != 0) {
        textureColor.rgb /= textureColor.a;
    }

    FragColor = color * textureColor;
}