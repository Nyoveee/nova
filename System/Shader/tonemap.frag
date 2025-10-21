#version 450 core
out vec4 FragColor;

in vec2 textureCoords;

uniform sampler2D hdrBuffer;
uniform float exposure;
uniform float gamma;

uniform int toneMappingMethod; // 0 = Exposure, 1 = Reinhard, 2 = ACES, 3 = None
uniform bool toGammaCorrect;

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

    if(toGammaCorrect) {
        // Gamma correction.
        mapped = pow(mapped, vec3(1.0 / gamma));
    }

    FragColor = vec4(mapped, 1.0);
}