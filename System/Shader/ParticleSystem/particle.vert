#version 450 core

struct ParticleVertex{
	vec4 color;
	vec3 position;
	float rotation;
	float currentSize;
};

struct Particle{
    vec4 startColor;
    vec4 endColor;
    vec3 velocity;
    vec3 force;
    vec3 lightattenuation;
    float colorInterpolation;
    float sizeInterpolation;
    float lightIntensity;
    float lightRadius;
    float angularVelocity;
    float startSize;
    float endSize;
    float currentLifeTime;
    float lifeTime;
    bool colorOverLifetime;
    bool sizeOverLifetime;
    bool b_Active;
};

layout(std140, binding = 0) uniform Camera {
    mat4 view;
    mat4 projection;
    mat4 cameraProjectionView;
    mat4 inverseView;
    mat4 inverseProjection;
    mat4 inverseProjectionView;
    mat4 previousViewProjection;    // for TAA

    vec3 cameraPosition;
   
    uvec3 gridSize;
    uvec2 screenDimensions;
    float zNear;
    float zFar;
};

layout(std430, binding = 4) buffer ParticleVertexList {
    ParticleVertex particleVertices[];
};
layout(std430, binding = 5) buffer ParticleList {
    int particleCount;
    Particle particles[];
};
out vec4 color;
out vec2 textureUnit;

mat4 RotationMatrix(float rotationValue){
    mat4 rotation = mat4(1.0);
    rotation[0][0] = cos(rotationValue);
    rotation[0][1] = -sin(rotationValue);
    rotation[1][0] = sin(rotationValue);
    rotation[1][1] = cos(rotationValue);
    return rotation; 
}


const vec3 vertexPos[4] = {
	vec3(-1, -1, 0),	// bottom left
	vec3(1, -1, 0),	// bottom right
	vec3(1, 1, 0),		// top right
	vec3(-1, 1, 0)		// top left
};
const vec2 textureCoordinates[4] = {
	vec2(0, 0), // bottom left
	vec2(1, 0), // bottom right
	vec2(1, 1), // top right
	vec2(0, 1)  // top left
};
uniform uint textureIndex;
uniform uint maxParticlesPerTexture;

void main() {
    uint index = textureIndex* maxParticlesPerTexture + gl_InstanceID;
    if(!particles[index].b_Active)
        return;
    mat4 translate = mat4(1.0);
    translate[3][0] = particleVertices[index].position.x;
    translate[3][1] = particleVertices[index].position.y;
    translate[3][2] = particleVertices[index].position.z;
    mat4 modelView = view * translate;
    
    // Rotate to face camera(Bill Boarding)
    modelView[0][0] = 1;
    modelView[0][1] = 0;
    modelView[0][2] = 0;

    modelView[1][0] = 0;
    modelView[1][1] = -1;
    modelView[1][2] = 0;

    modelView[2][0] = 0;
    modelView[2][1] = 0;
    modelView[2][2] = 1;

    vec3 localpos = vertexPos[gl_VertexID] * particleVertices[index].currentSize;
    gl_Position = projection * modelView * RotationMatrix(particleVertices[index].rotation) * vec4(localpos,1);
    textureUnit = textureCoordinates[gl_VertexID];
    color = particleVertices[index].color;
}
