#pragma once
#include "export.h"
#include "glm/glm.hpp"
#include "type_alias.h"
namespace RandomRange
{
	FRAMEWORK_DLL_API float Float(float min, float max);
	FRAMEWORK_DLL_API int Int(int min, int max);
	FRAMEWORK_DLL_API glm::vec2 Vec2(glm::vec2 min, glm::vec2 max);
	FRAMEWORK_DLL_API glm::vec3 Vec3(glm::vec3 min, glm::vec3 max);
	FRAMEWORK_DLL_API glm::vec4 Vec4(glm::vec4 min, glm::vec4 max);
};

