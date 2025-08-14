#version 450 core

in vec2 textureUnit;
uniform sampler2D image;
uniform uint objectId;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out uint ObjectId;

void main()
{
    vec4 color = texture(image, textureUnit);
    FragColor = vec4(vec3(0.3 + float(objectId) / 4.0), 1);
    ObjectId = objectId;
}