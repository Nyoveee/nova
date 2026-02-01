#version 450

in vec2 textureCoords;

out vec4 FragColor;

uniform sampler2D scene;
uniform sampler2D historyTexture;
uniform sampler2D velocityUvTexture;

// Explanation to handle specular aliasing
// http://behindthepixels.io/assets/files/TemporalAA.pdf
// page 5, 3.3.1. Sample accumulation in HDR color space

/*
    filtering operations such as antialiasing are best applied in the post-tonemapped space to produce correct
    edge gradient on display. 

    however, we want to apply TAA before post processing effect..

    workaround is to tonemap the color, apply TAA, then reverse the tonemap..
*/

// Store weight in w component
vec4 adjustHDRColor(vec3 color) {
    float luminance = dot(color, vec3(0.299, 0.587, 0.114));
    float luminanceWeight = 1.0 / (1.0 + luminance);
    return vec4(color, 1.0 * luminanceWeight);
}

void main() {
    vec2 velocityUvOffset = texture(velocityUvTexture, textureCoords).rg;
    // vec2 velocityUvOffset = vec2(0);

    vec4 currentColor = adjustHDRColor(texture(scene, textureCoords).rgb);
    vec3 historyColor = texture(historyTexture, textureCoords - velocityUvOffset).rgb;

    // https://www.elopezr.com/temporal-aa-and-the-quest-for-the-holy-trail/
    // Color clamping...
    // Arbitrary out of range numbers
    vec3 minColor = vec3(9999.0);
    vec3 maxColor = vec3(-9999.0);
    
    // Sample a 3x3 neighborhood to create a box in color space
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            vec2 offset = vec2(x, y) / textureSize(scene, 0);
            vec3 color = texture(scene, textureCoords + offset).rgb;

            minColor = min(minColor, color); // Take min and max
            maxColor = max(maxColor, color);
        }
    }
    
    // Clamp previous color to min/max bounding box
    vec4 historyColorClamp = adjustHDRColor(clamp(historyColor, minColor, maxColor));
    // vec4 historyColorClamp = vec4(clamp(historyColor, minColor, maxColor), 1);

    // Blend.. (alpha was modified by luminance calculation..)
    // We multiply the weight by luminance as a form of tonemapping...
    float currentWeight = 0.1 * currentColor.a;
    float previousWeight = 0.9 * historyColorClamp.a;
    
    vec3 finalColor = (currentColor.rgb * currentWeight + historyColorClamp.rgb * previousWeight);
    finalColor /= (currentWeight + previousWeight); // Normalize back from tonemapped color space to original linear space..

    FragColor = vec4(finalColor, 1);
}