#pragma once

#include <vector>
#include <variant>

#include <glm/mat4x4.hpp>
#include <glm/mat3x3.hpp>
#include <entt/entt.hpp>
#include <string>

#include "type_alias.h"
#include "reflection.h"
#include "physics.h"

class Prefab;
class Model;
class Texture;
class CubeMap;
class ScriptAsset;
class Audio;
class Material;
class Sequencer;

// ============================
// Here we list all the possible types our serialized field can have..
// ============================
#ifndef ALL_FIELD_PRIMITIVES
#define ALL_FIELD_PRIMITIVES \
		bool, int, float, double
#endif

#define ALL_TYPED_RESOURCE_ID \
	TypedResourceID<Prefab>, TypedResourceID<Model>, TypedResourceID<Texture>, TypedResourceID<Material>

#ifndef ALL_FIELD_TYPES
#define ALL_FIELD_TYPES \
		glm::vec2, glm::vec3, glm::vec4, glm::quat, entt::entity, PhysicsRay, PhysicsRayCastResult,	std::string,	\
		ALL_TYPED_RESOURCE_ID,																						\
		ALL_FIELD_PRIMITIVES
#endif

// ============================
// Here we establish recursive relationship between our variant and another containers like list.
// ============================
struct serialized_field_list;

using serialized_field_type = std::variant<
	ALL_FIELD_TYPES,
	serialized_field_list
>;

template <std::size_t I = 0>
serialized_field_type default_construct_serialized_field_type(std::size_t index);

struct serialized_field_list : public std::vector<serialized_field_type> {
	serialized_field_list() = default;

	void setIndex(int p_index) {
		index = p_index;
	}

	// i wanna hide the base class's push_back.
	void push_back(serialized_field_type value) {
		if (value.index() != index) {
			assert(false && "Attempting to add different type into this list.");
			push_back_default_construct();
			return;
		}
		
		std::vector<serialized_field_type>::push_back(value);
	}

	// i wanna hide the base class's resize.
	void resize(std::size_t newSize) {
		if (size() > newSize) {
			std::vector<serialized_field_type>::resize(newSize);
		}
		else {
			reserve(newSize);

			while (size() < newSize) {
				push_back_default_construct();
			}
		}
	}

	int getIndex() const {
		return index;
	}

private:
	void push_back_default_construct() {
		assert(index != -1 && "Invalid serialized list.");
		std::vector<serialized_field_type>::push_back(default_construct_serialized_field_type(index));
	}

	int index = -1;
};

// ====================================================================================
// We allow the default construction of our specific serialized field type at runtime.
// ====================================================================================

template <std::size_t I>
serialized_field_type default_construct_serialized_field_type(std::size_t index) {
	if constexpr (I >= std::variant_size_v<serialized_field_type>) {
		assert(false && "This should never happened. Index is bigger than the std::variant's size.");
		return serialized_field_type{ std::in_place_index<0> };
	}
	else {
		if (index == I) {
			return serialized_field_type{ std::in_place_index<I> };
		}
		else {
			return default_construct_serialized_field_type<I + 1>(index);
		}
	}
}

struct FieldData {
	std::string name;
	serialized_field_type data;

	REFLECTABLE(
		name,
		data
	)
};