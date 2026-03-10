#version 450 core

in vec2 textureCoords;
out vec4 FragColor;

uniform sampler2D image;
uniform vec4 tintColor;

uniform vec2 textureCoordinatesRange;
uniform bool toTile;
uniform bool isAlphaMap;

uniform vec2 textureCoordinatesStart;
uniform vec2 textureCoordinatesEnd;

void main()
{   
    if(
            textureCoords.x > textureCoordinatesEnd.x
        ||  textureCoords.x < textureCoordinatesStart.x
        ||  (1 - textureCoords.y) > textureCoordinatesEnd.y
        ||  (1 - textureCoords.y) < textureCoordinatesStart.y
    ) {
        discard;
    }

    vec4 textureColor = texture(image, textureCoords);

    if(isAlphaMap) {
        FragColor = vec4(tintColor.rgb, tintColor.a * textureColor.r);
    }
    else {
        FragColor = textureColor * tintColor;
    }
}  