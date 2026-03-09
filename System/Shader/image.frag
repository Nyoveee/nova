#version 450 core

uniform sampler2D image;
uniform uint objectId;

uniform vec2 textureCoordinatesMultiplier;
uniform bool toTile;

in VS_OUT {
    vec2 textureUnit;
} fsIn;

layout (location = 0) out vec4 FragColor;

void main()
{
    vec4 textureColor = texture(image, fsIn.textureUnit);
    
    // reverse premultiply alpha..
    if(textureColor.a != 0) {
        textureColor.rgb /= textureColor.a;
    }

    // vec4 color = texture(image, fsIn.textureUnit);
    FragColor = textureColor;
}