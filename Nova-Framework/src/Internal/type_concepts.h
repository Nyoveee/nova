#pragma once
#include <type_traits>
#include <concepts>
#include "type_alias.h"
template<class T>
concept IsTypedResourceID = requires(T x) {
	{ TypedResourceID{ x } } -> std::same_as<T>;
};
template<class T>
concept IsEnum = std::is_enum_v<T>;