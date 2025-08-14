#version 450

out vec4 FragColor;
in vec3 worldPos;

const vec3 gridColor = vec3(0, 0, 0);
const float gridSize = 0.1;

void main() {
    float dx = length(vec2(dFdx(worldPos.x), dFdy(worldPos.x)));
    float dz = length(vec2(dFdx(worldPos.z), dFdy(worldPos.z)));

    // float line = step(fract(worldPos.x), thickness) || step(fract(worldPos.y), thickness);
    vec2 zMod = vec2(1.0) - (mod(worldPos.xz, gridSize) / (4 * vec2(dx, dz)));

    // float xMod = mod(worldPos.x, gridSize) / gridSize;
    // xMod = step(0.98, xMod);

    FragColor = vec4(0, 0, 0, max(zMod.x, zMod.y));
}