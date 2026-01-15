#version 450 core

// Very simple library

in VS_OUT {
    vec2 textureUnit;
    vec3 normal;
    vec3 fragWorldPos;
    vec3 fragViewPos;
} fsIn;

uniform float timeElapsed;

layout (location = 0) out vec4 FragColor; 
layout (location = 1) out vec3 gNormal;
uniform bool toOutputNormal;

// User shader entry point.
vec4 __internal__main__();

// Wrapper around user entry point.
void main() { 
    if(toOutputNormal) {
        gNormal = fsIn.normal;
    }
    else {
        FragColor = __internal__main__(); 
    }
}