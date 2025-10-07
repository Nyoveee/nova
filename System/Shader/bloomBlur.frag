#version 450 core

uniform sampler2D image;
in vec2 textureCoords;
out vec4 FragColor;

uniform bool horizontal;

void main()
{             
    vec2 tex_offset = 1.0 / textureSize(image, 0); // size of one texel
    vec3 result = vec3(0.0);

    int radius = 9; 
    int count = 0;

    if(horizontal)
    {
        for(int i = -radius; i <= radius; ++i)
        {
            result += texture(image, textureCoords + vec2(tex_offset.x * i, 0.0)).rgb;
            count++;
        }
    }
    else
    {
        for(int i = -radius; i <= radius; ++i)
        {
            result += texture(image, textureCoords + vec2(0.0, tex_offset.y * i)).rgb;
            count++;
        }
    }

    FragColor = vec4(result / float(count), 1.0); // average
}