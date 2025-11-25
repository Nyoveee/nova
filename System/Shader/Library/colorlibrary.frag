#version 450 core

// Very simple library

in VS_OUT {
    vec2 textureUnit;
} fsIn;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;  

void renderBloomBrightColors() {
    // check whether fragment output is higher than threshold, if so output as brightness color
    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));

    if(brightness > 1.0)
        BrightColor = vec4(FragColor.rgb, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}