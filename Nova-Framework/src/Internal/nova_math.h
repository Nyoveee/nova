#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include "export.h"

struct Transform;

namespace Math {
	struct DecomposedMatrix {
		glm::vec3 position;
		glm::quat rot;
		glm::vec3 scale;
	};

	FRAMEWORK_DLL_API DecomposedMatrix decomposeMatrix(glm::mat4 const& m);
	FRAMEWORK_DLL_API glm::mat4 composeMatrix(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale);
	FRAMEWORK_DLL_API std::size_t getGUID();

	// smooth's the interval using a sine curve.
	FRAMEWORK_DLL_API float smoothstep(float interval);
	FRAMEWORK_DLL_API float sinestep(float interval);

	FRAMEWORK_DLL_API bool isPointInRect(glm::vec2 point, glm::vec2 center, glm::vec2 scale);
}
