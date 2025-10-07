#version 450 core

uniform sampler2D scene;

in vec2 textureCoords;
out vec4 FragColor;

uniform sampler2D bloomBlur;
uniform float exposure;

void main()
{             
    const float gamma = 2.2;
    vec3 hdrColor = texture(scene, textureCoords).rgb;      
    vec3 bloomColor = texture(bloomBlur, textureCoords).rgb;
    hdrColor += bloomColor * 1.5; // additive blending
    // tone mapping
     vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
     // also gamma correct while we're at it       
     //result = pow(result, vec3(1.0 / gamma));
    FragColor = vec4(result, 1.0);
}  