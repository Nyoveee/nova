#pragma once
#include <type_traits>
#include <concepts>
#include "type_alias.h"
#include "physics.h"

template<class T>
concept IsTypedResourceID = requires(T x) {
	{ TypedResourceID{ x } } -> std::same_as<T>;
};

template<class T>
concept IsEnum = std::is_enum_v<T> && !requires(T x) {
	{ T{ x } } -> std::same_as<entt::entity>;
};

template<class T>
concept IsFundamental = std::is_fundamental_v<T> || requires(T x) {
	{ T{ x } } -> std::same_as<std::string>;
};

template<class T>
concept NonSerializableTypes = requires(T x) { { T{ x } } -> std::same_as<PhysicsRay>; }
                            || requires(T x) { { T{ x } } -> std::same_as<PhysicsRayCastResult>; };

template<typename T>
concept isVector = requires {
	typename T::value_type; // Check if it has a value_type member alias
	requires std::is_same_v<T, std::vector<typename T::value_type, typename T::allocator_type>>;
};

template<typename T>
concept isUnorderedMap = requires {
	typename T::key_type;
	typename T::mapped_type;

		requires std::is_same_v<T, std::unordered_map<typename T::key_type, typename T::mapped_type>>;
};

template<typename T>
concept isUnorderedSet = requires {
	typename T::key_type;

	requires std::is_same_v<T, std::unordered_set<typename T::key_type>>;
};

template<typename T>
concept isPair = requires {
	typename T::first_type;
	typename T::second_type;

	requires std::is_same_v<T, std::pair<typename T::first_type, typename T::second_type>>;
};

// Old school type traits.
template <typename T>
struct is_std_array : std::false_type {};

template <typename T, std::size_t N>
struct is_std_array<std::array<T, N>> : std::true_type {};