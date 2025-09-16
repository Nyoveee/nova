#version 330 core

out vec2 TexCoords;

void main()
{
    // Fullscreen triangle technique - no vertex buffer needed
    // Creates a large triangle that covers the entire screen

    float x = -1.0 + float((gl_VertexID & 1) << 2);
    float y = -1.0 + float((gl_VertexID & 2) << 1);
   
    TexCoords.x = (x + 1.0) * 0.5;
    TexCoords.y = (y + 1.0) * 0.5;
    gl_Position = vec4(x, y, 0.0, 1.0);
}