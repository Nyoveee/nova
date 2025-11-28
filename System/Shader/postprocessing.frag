#version 450 core

in vec2 textureCoords;
out vec4 FragColor;

uniform sampler2D scene;

uniform vec3 offset;

void main()
{             
    float rValue = texture2D(scene, textureCoords - offset.x).r; 
    float gValue = texture2D(scene, textureCoords - offset.y).g;
    float bValue = texture2D(scene, textureCoords - offset.z).g;  

    float average = (rValue + gValue + bValue) / 3.0;
    FragColor = vec4(average, average, average, 1.0);

    // FragColor = vec4(rValue, gValue, bValue, 1.0);  

    // FragColor = vec4(vec3(1.0 - texture(scene, textureCoords)), 1.0);
}  