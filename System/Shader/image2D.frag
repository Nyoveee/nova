#version 450 core

in vec2 textureCoords;
out vec4 FragColor;

uniform sampler2D image;
uniform vec4 tintColor;

uniform vec2 textureCoordinatesRange;
uniform bool toTile;

void main()
{   
    vec2 newTextureCoords = textureCoords;

    if(!toTile) {
        if(
                textureCoords.x > textureCoordinatesRange.x
            ||  (1 - textureCoords.y) > textureCoordinatesRange.y
        ){
            discard;
        }
    }
    else {
        newTextureCoords.x = mod(textureCoords.x, textureCoordinatesRange.x);
        newTextureCoords.y = mod((1 - textureCoords.y), textureCoordinatesRange.y);
    }

    FragColor = texture(image, newTextureCoords) * tintColor;
}  