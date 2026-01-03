#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include "nova_math.h"

#include "component.h"
#include "xresource_guid.h"

// Reference: https://stackoverflow.com/questions/17918033/glm-decompose-mat4-into-translation-and-rotation
namespace Math {
	DecomposedMatrix decomposeMatrix(glm::mat4 const& modelMatrix) {
		glm::vec3 scale;
		glm::quat rotation;
		glm::vec3 translation;
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(modelMatrix, scale, rotation, translation, skew, perspective);

		return {
			translation,
			rotation,
			scale
		};
	}

	glm::mat4 composeMatrix(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale)
	{
		glm::mat4 translation = glm::translate(glm::mat4(1.0f), position);
		glm::mat4 rotationMatrix = glm::mat4_cast(rotation);  // Converts quaternion to a rotation matrix
		glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);

		return translation * rotationMatrix * scaleMatrix;
	}

    std::size_t getGUID() {
		xresource::instance_guid guid = xresource::instance_guid::GenerateGUIDCopy();
		return guid.m_Value;
    }

	float smoothstep(float t) {
		return t * t * (3.0f - 2.0f * t);
	}

	float sinestep(float interval) {
		float sine_input = (interval * std::numbers::pi_v<float>) - (std::numbers::pi_v<float> / 2.0f);
		return (std::sin(sine_input) + 1.0f) / 2.0f;
	}

	bool isPointInRect(glm::vec2 point, glm::vec2 center, glm::vec2 scale) {
		// Calculate half dimensions
		scale /= 2.f;

		// Calculate the min and max bounds of the rectangle (bottom-left and top-right corners)
		glm::vec2 minBounds = glm::vec2(center.x - scale.x, center.y - scale.y);
		glm::vec2 maxBounds = glm::vec2(center.x + scale.x, center.y + scale.y);

		// Check if the point's coordinates are within the min and max bounds
		bool withinX = point.x >= minBounds.x && point.x <= maxBounds.x;
		bool withinY = point.y >= minBounds.y && point.y <= maxBounds.y;

		// The point is inside the rectangle if it is within both X and Y bounds
		return withinX && withinY;
	}
}

