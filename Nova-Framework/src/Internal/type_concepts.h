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