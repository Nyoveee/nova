// https://learnopengl.com/PBR/IBL/Diffuse-irradiance

#version 450 core

out vec4 FragColor;
in vec3 localPos;

uniform sampler2D equirectangularMap;

// We convert from spherical coordinate system to cartesian.
const vec2 invAtan = vec2(0.1591, 0.3183);

vec2 SampleSphericalMap(vec3 v) {
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main() {
    // i need to flip the y direction because the image is loaded upside down?
    vec3 directionPos = normalize(vec3(localPos.x, -localPos.y, localPos.z));

    vec2 uv = SampleSphericalMap(directionPos); // make sure to normalize localPos
    vec3 color = texture(equirectangularMap, uv).rgb;
    
    FragColor = vec4(color, 1.0);
}