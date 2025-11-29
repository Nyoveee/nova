#version 450 core

// A VBO-less square render.
// This is used for post processing.

// ====================================
// CENTER ANCHOR
// ====================================
const vec3 centralisedVertexPos[4] = vec3[4](
	vec3(-0.5, -0.5,  0),	// bottom left
	vec3( 0.5, -0.5,  0),	// bottom right
	vec3( 0.5,  0.5,  0),	// top right
	vec3(-0.5,  0.5,  0) 	// top left
);

// ====================================
// BOTTOM LEFT ANCHOR
// ====================================
const vec3 originBottomLeft[4] = vec3[4](
	vec3(0.0, 0.0, 0.0),	// bottom left (origin)
	vec3(1.0, 0.0, 0.0),	// bottom right
	vec3(1.0, 1.0, 0.0),	// top right
	vec3(0.0, 1.0, 0.0) 	// top left
);

// ====================================
// BOTTOM RIGHT ANCHOR
// ====================================
const vec3 originBottomRight[4] = vec3[4](
	vec3(-1.0, 0.0, 0.0),	// bottom left
	vec3( 0.0, 0.0, 0.0),	// bottom right (origin)
	vec3( 0.0, 1.0, 0.0),	// top right
	vec3(-1.0, 1.0, 0.0) 	// top left
);

// ====================================
// TOP RIGHT ANCHOR
// ====================================
const vec3 originTopRight[4] = vec3[4](
	vec3(-1.0, -1.0, 0.0),	// bottom left
	vec3( 0.0, -1.0, 0.0),	// bottom right
	vec3( 0.0,  0.0, 0.0),	// top right (origin)
	vec3(-1.0,  0.0, 0.0) 	// top left
);

// ====================================
// TOP LEFT ANCHOR
// ====================================
const vec3 originTopLeft[4] = vec3[4](
	vec3(0.0, -1.0, 0.0),	// bottom left
	vec3(1.0, -1.0, 0.0),	// bottom right
	vec3(1.0,  0.0, 0.0),	// top right
	vec3(0.0,  0.0, 0.0) 	// top left (origin)
);

const vec2 textureCoordinates[4] = vec2[4](
    vec2(0, 0),
    vec2(1, 0),
    vec2(1, 1),
    vec2(0, 1)
);

const int indices[6] = int[6](0, 2, 1, 2, 0, 3);

out vec2 textureCoords;

uniform mat4 model;
uniform mat4 uiProjection;
uniform int anchorMode;
uniform bool toFlip;

const int CENTER_ANCHOR = 0;
const int BOTTOM_LEFT_ANCHOR = 1;
const int BOTTOM_RIGHT_ANCHOR = 2;
const int TOP_LEFT_ANCHOR = 3;
const int TOP_RIGHT_ANCHOR = 4;

void main() {
    int index = indices[gl_VertexID];

    vec4 localPosition;

    switch (anchorMode) {
        case BOTTOM_LEFT_ANCHOR:
            localPosition = vec4(originBottomLeft[index], 1);
            break;

        case BOTTOM_RIGHT_ANCHOR:
            localPosition = vec4(originBottomRight[index], 1);
            break;
                    
        case TOP_LEFT_ANCHOR:
            localPosition = vec4(originTopLeft[index], 1);
            break;
                    
        case TOP_RIGHT_ANCHOR:
            localPosition = vec4(originTopRight[index], 1);
            break;        

        default:
            // We default to center anchor..
            localPosition = vec4(centralisedVertexPos[index], 1);
            break;
    }

    gl_Position = uiProjection * model * localPosition;

    textureCoords = textureCoordinates[index];

    if(!toFlip) {
        textureCoords.y = 1 - textureCoords.y;
    }
}