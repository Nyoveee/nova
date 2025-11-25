#version 450 core

uniform sampler2D image;
in vec2 textureCoords;
out vec4 FragColor;

void main()
{             
    vec2 tex_offset = 1.0 / textureSize(image, 0); // size of one texel
    int radius = 9; // blur radius
    vec3 result = vec3(0.0);
    float count = 0.0;

    // Sample in both X and Y (2D blur)
    for (int x = -radius; x <= radius; ++x)
    {
        for (int y = -radius; y <= radius; ++y)
        {
            vec2 offset = vec2(tex_offset.x * x, tex_offset.y * y);
            result += texture(image, textureCoords + offset).rgb;
            count += 1.0;
        }
    }

    result /= count;
    FragColor = vec4(result, 1.0);
}