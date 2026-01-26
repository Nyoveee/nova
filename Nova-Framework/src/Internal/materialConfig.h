#pragma once

enum class BlendingConfig {
	AlphaBlending,
	AdditiveBlending,
	PureAdditiveBlending,
	PremultipliedAlpha,
	Disabled
};

enum class DepthTestingMethod {
	DepthTest,
	NoDepthWrite,
	NoDepthWriteTest
};

enum class CullingConfig {
	Enable,
	Disable
};

#include "reflection.h"
#include <string>
#include <variant>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <unordered_map>
#include <unordered_set>
#include "type_alias.h"
#include "systemResource.h"

class Texture;

#define AllUniformTypes \
	bool, int, unsigned int, float, glm::vec2, glm::vec3, glm::vec4, glm::mat3, glm::mat4, TypedResourceID<Texture>, Color, ColorA, NormalizedFloat

using UniformValue = std::variant<AllUniformTypes>;

struct UniformData {
	std::string glslType;
	std::string identifier;
	UniformValue value;

	REFLECTABLE(
		glslType,
		identifier,
		value
	)
};

#if 0
static inline const std::unordered_set<std::string> validGlslPrimitive{
	"bool", "int", "uint", "float", "vec2", "vec3", "vec4", "mat3", "mat4", "sampler2D"
};

static inline const std::unordered_set<std::string> validCustomTypes{
	"Color", "ColorA", "NormalizedFloat"
};
#endif

// maps custom glsl type to native glsl type.
static inline const std::unordered_map<std::string, std::string> customTypeToGlslPrimitive{
	{ "Color",				"vec3"		},
	{ "ColorA",				"vec4"		},
	{ "NormalizedFloat",	"float"		},
};

// maps all glsl type to the correct corresponding uniform value/
static inline const std::unordered_map<std::string, UniformValue> allValidShaderTypes = {
	{ "bool",				bool{}										},
	{ "int",				int{}										},
	{ "uint",				unsigned{}									},
	{ "float",				float{}										}, 
	{ "vec2",				glm::vec2{}									},
	{ "vec3",				glm::vec3{}									}, 
	{ "vec4",				glm::vec4{}									},
	{ "mat3",				glm::mat3{}									}, 
	{ "mat4",				glm::mat4{}									},
	{ "sampler2D",			TypedResourceID<Texture>{ NONE_TEXTURE_ID } },

	// custom glsl type..
	{ "Color",				Color{ 1.f, 1.f, 1.f }						},
	{ "ColorA",				ColorA{ 1.f, 1.f, 1.f, 1.f }				},
	{ "NormalizedFloat",	NormalizedFloat{}							}
};