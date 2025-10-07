#version 450 core

uniform sampler2D scene;

in vec2 textureCoords;
out vec4 FragColor;

void main()
{
    vec4 color = texture(scene, textureCoords);

    // Extract only bright fragments (threshold)
    float brightness = dot(vec3(color), vec3(0.2126, 0.7152, 0.0722)); // luminance
    if(brightness > 0.7) // threshold
        FragColor = color;
    else
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}