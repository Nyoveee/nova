// https://learnopengl.com/PBR/IBL/Diffuse-irradiance
// https://learnopengl.com/Advanced-OpenGL/Cubemaps

#version 450 core

out vec4 FragColor;
in vec3 localPos;

uniform samplerCube cubemap; // cubemap texture sampler

void main() {
    // i need to flip the y direction because the image is loaded upside down?
    vec3 directionPos = normalize(vec3(localPos.x, localPos.y, localPos.z));
    vec3 color = texture(cubemap, directionPos).rgb;
    FragColor = vec4(color, 1.0);
}