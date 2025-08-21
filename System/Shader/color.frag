#version 450 core

uniform uint objectId;
uniform vec3 color;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out uint ObjectId;

void main()
{
    FragColor = vec4(color, 1);
    ObjectId = objectId;
}