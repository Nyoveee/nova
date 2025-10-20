#version 450 core

uniform sampler2D image;
in vec2 textureCoords;
out vec4 FragColor;

void main()
{
    // Offsets for kernel
    vec2 tex_offset = 1.0 / textureSize(image, 0); // size of one texel
    vec2 offsets[9] = vec2[](
        vec2(-tex_offset.x,  tex_offset.y),  // top-left
        vec2(0.0,             tex_offset.y),  // top-center
        vec2(tex_offset.x,    tex_offset.y),  // top-right
        vec2(-tex_offset.x,  0.0),           // center-left
        vec2(0.0,            0.0),           // center
        vec2(tex_offset.x,   0.0),           // center-right
        vec2(-tex_offset.x, -tex_offset.y),  // bottom-left
        vec2(0.0,           -tex_offset.y),  // bottom-center
        vec2(tex_offset.x,  -tex_offset.y)   // bottom-right
    );

    //blur kernel
    float kernel[9] = float[](
        1.0/16, 2.0/16, 1.0/16,
        2.0/16, 4.0/16, 2.0/16,
        1.0/16, 2.0/16, 1.0/16
    );

    vec3 sampleTex[9];
    for (int i = 0; i < 9; i++)
        sampleTex[i] = texture(image, textureCoords + offsets[i]).rgb;

    vec3 color = vec3(0.0);
    for (int i = 0; i < 9; i++)
        color += sampleTex[i] * kernel[i];

    FragColor = vec4(color, 1.0);
}