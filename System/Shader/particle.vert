#version 450 core

layout(std140, binding = 0) uniform Camera {
    mat4 view;
    mat4 projection;
};

layout (location = 0) in vec3 localpos;
layout (location = 1) in vec3 worldPos;
layout (location = 2) in vec2 textureCoordinates;
layout (location = 3) in vec4 vertColor;
layout (location = 4) in float rotation;

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

void main() {
    mat4 translate = mat4(1.0);
    translate[3][0] = worldPos.x;
    translate[3][1] = worldPos.y;
    translate[3][2] = worldPos.z;
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
	const glm::vec3 vertexPos[4] = {
		glm::vec3(-1, -1, 0),	// bottom left
		glm::vec3(1, -1, 0),	// bottom right
		glm::vec3(1, 1, 0),		// top right
		glm::vec3(-1, 1, 0)		// top left
	};
	const glm::vec2 textureCoordinates[4] = {
		glm::vec2(0, 0), // bottom left
		glm::vec2(1, 0), // bottom right
		glm::vec2(1, 1), // top right
		glm::vec2(0, 1)  // top left
	};
    gl_Position = projection * modelView * RotationMatrix(rotation) * vec4(localpos,1);
    textureUnit = textureCoordinates;
    color = vertColor;
}
