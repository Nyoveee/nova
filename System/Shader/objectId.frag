#version 450 core

uniform uint objectId;
out uint ObjectId;

void main()
{    
    ObjectId = objectId;
}