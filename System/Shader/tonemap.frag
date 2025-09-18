#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D hdrBuffer;
uniform float exposure;
uniform float gamma;
uniform bool useSRGBFramebuffer;
uniform int toneMappingMethod; // 0 = Exposure, 1 = Reinhard, 2 = ACES

void main()
{
    vec3 hdrColor = texture(hdrBuffer, TexCoords).rgb;
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
        // Default to exposure if invalid method
        mapped = vec3(1.0) - exp(-hdrColor * exposure);
    }

    // If using sRGB framebuffer, hardware will handle gamma correction automatically
    // Otherwise, apply manual gamma correction
    if (!useSRGBFramebuffer) {
        mapped = pow(mapped, vec3(1.0 / gamma));
    }

    FragColor = vec4(mapped, 1.0);
}