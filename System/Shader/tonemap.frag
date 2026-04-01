#version 450 core
out vec4 FragColor;

in vec2 textureCoords;

uniform sampler2D hdrBuffer;
uniform float exposure;

uniform int toneMappingMethod; // 0 = Exposure, 1 = Reinhard, 2 = ACES, 3 = None
uniform bool toGammaCorrect;
uniform float gamma;

// ranges from [-1, 1]
uniform float temperature;
uniform float tint;
uniform float saturation;
uniform float contrast;
uniform float brightness;

vec3 colorGrading(vec3 color) {
    // brightness
    color += brightness;

    // contrast
    vec3 midpoint = vec3(0.5);
    color = mix(midpoint, color, mix(0.8, 1.f, contrast));

    // saturation
    float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
    color = mix(vec3(luminance), color, saturation + 1.0);

    // temperature
    vec3 warm = vec3(1.0, 0.9, 0.8);
    vec3 cool = vec3(0.8, 0.9, 1.0);

    vec3 temperatureBalance = mix(cool, warm, temperature);
    vec3 tintBalance = vec3(1.0 - tint * 0.1,
                            1.0 + tint * 0.2,
                            1.0 - tint * 0.1);

    return color * temperatureBalance * tintBalance;
}

void main()
{
    vec3 hdrColor = texture(hdrBuffer, textureCoords).rgb;
    vec3 mapped;

    if (toneMappingMethod == 0) {
        // Exposure tone mapping
        mapped = vec3(1.0) - exp(-hdrColor * exposure);
    }
    else if (toneMappingMethod == 1) {
        // Reinhard tone mapping
        mapped = (hdrColor * exposure) / (hdrColor * exposure + vec3(1.0));
    }
    else if (toneMappingMethod == 2) {
        // ACES tone mapping
        const float a = 2.51;
        const float b = 0.03;
        const float c = 2.43;
        const float d = 0.59;
        const float e = 0.14;
        vec3 exposedColor = hdrColor * exposure;
        mapped = clamp((exposedColor * (a * exposedColor + b)) / (exposedColor * (c * exposedColor + d) + e), 0.0, 1.0);
    }
    else {
        mapped = hdrColor;
    }


    if(toneMappingMethod != 3) {
        mapped = colorGrading(mapped);
    }
    
    if(toGammaCorrect) {
        // Gamma correction.
        mapped = pow(mapped, vec3(1.0 / gamma));
    }


    FragColor = vec4(mapped, 1.0);
}