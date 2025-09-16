#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D hdrBuffer;
uniform float exposure;
uniform float gamma;

void main()
{
    vec3 hdrColor = texture(hdrBuffer, TexCoords).rgb;

    // Apply exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);

    // Alternative: Reinhard tone mapping (uncomment to try)
    // vec3 mapped = hdrColor / (hdrColor + vec3(1.0));

    // Alternative: ACES tone mapping (uncomment to try)
    // const float a = 2.51;
    // const float b = 0.03;
    // const float c = 2.43;
    // const float d = 0.59;
    // const float e = 0.14;
    // vec3 mapped = clamp((hdrColor * (a * hdrColor + b)) / (hdrColor * (c * hdrColor + d) + e), 0.0, 1.0);

    // Apply gamma correction
    mapped = pow(mapped, vec3(1.0 / gamma));

    FragColor = vec4(mapped, 1.0);
    // FragColor = vec4(1, 1.0);
}