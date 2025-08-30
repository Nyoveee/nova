#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

struct Vertex {
	glm::vec3 pos;
	glm::vec2 textureUnit;
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec3 bitangent;
};

struct SimpleVertex {
	glm::vec3 pos;
};

// this struct will be used to send to SSBO.
// our SSBO follows the std430 alignment rule,
// this caveat means that we have to be mightful of alignments of 
// our native types, especially for vec3s like color for an example.

#pragma warning( push )
#pragma warning(disable : 4324)			// disable warning about structure being padded, that's exactly what i wanted.

struct alignas(16) LightData {
	alignas(16) glm::vec3 lightPos;		// this will represent light direction for directional light
	alignas(16) glm::vec3 color;		// strength of the light, not limited to range of [0, 1]
	unsigned int type;					// this will hold the corresponding enum values of Light::Type
};

#pragma warning( pop )