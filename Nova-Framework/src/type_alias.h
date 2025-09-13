#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/quaternion.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <cstddef>
#include <limits>

// We provide strong type alias here. Type strongly describes the value and can catch errors in compile time. 
// Very useful for not using values unintended. For an example, you won't want to add GUIDs together, and 
// you don't want any arbitrary int to act as id.

// ========================================
// DEGREE AND ANGLES.
// ========================================

struct Radian;

struct Degree {
	constexpr Degree();
	constexpr Degree(float angle);
	constexpr Degree(Radian radian);

	constexpr explicit operator Radian() const;
	constexpr operator float() const;

	constexpr Degree& operator+=(float);

private:
	float angle;
};

struct Radian {
	constexpr Radian();
	constexpr Radian(float angle);
	constexpr Radian(Degree radian);

	constexpr explicit operator Degree() const;
	constexpr operator float() const;

	constexpr Radian& operator+=(float);
private:
	float angle;
};

// ========================================
// Color
// ========================================
struct Color {
	constexpr Color();
	constexpr Color(glm::vec3 color);
	constexpr Color(float r, float g, float b);

	constexpr operator glm::vec3() const;
private:
	glm::vec3 color;
};

struct ColorA {
	constexpr ColorA();
	constexpr ColorA(glm::vec4 color);
	constexpr ColorA(float r, float g, float b, float a);

	constexpr operator glm::vec4() const;
	constexpr explicit operator glm::vec3() const;
	constexpr explicit operator Color() const;
private:
	glm::vec4 color;
};

// ========================================
// IDs
// ========================================

// Every InputEvent (a type) in Input Manager will be associated with a unique number.

struct EventID {
	constexpr explicit EventID(std::size_t id);

public:
	constexpr friend bool operator==(EventID const& lhs, EventID const& rhs);
	constexpr friend bool operator<(EventID const& lhs, EventID const& rhs);
	friend struct std::hash<EventID>;

private:
	std::size_t id;
};

// Game objects hold reference to assets by using an ID.
struct ResourceID {
	constexpr ResourceID();
	constexpr ResourceID(std::size_t id);
	constexpr explicit operator std::size_t() const;

public:
	constexpr friend bool operator==(ResourceID const& lhs, ResourceID const& rhs);
	constexpr friend bool operator<(ResourceID const& lhs, ResourceID const& rhs);
	friend struct std::hash<ResourceID>;

private:
	std::size_t id;
};

// it's essentially AssetID but carrying additional type info of the original asset type. 
// useful in editor to retrieve the original asset type.
// TypedAssetID can implicitly convert to AssetID anytime, and vice versa.
template <typename T>
struct TypedResourceID : public ResourceID {
	using AssetType = T;
};

// an id given to the template type parameter of assets.
// for an example, type Texture could have id of 5, and Model could have id of 7.
struct ResourceTypeID {
	constexpr ResourceTypeID();
	constexpr ResourceTypeID(std::size_t id);
	constexpr explicit operator std::size_t() const;

public:
	constexpr friend bool operator==(ResourceTypeID const& lhs, ResourceTypeID const& rhs);
	constexpr friend bool operator<(ResourceTypeID const& lhs, ResourceTypeID const& rhs);
	friend struct std::hash<ResourceTypeID>;

private:
	std::size_t id;
};

struct FolderID {
	constexpr FolderID();
	constexpr FolderID(std::size_t id);
	constexpr explicit operator std::size_t() const;

public:
	constexpr friend bool operator==(FolderID const& lhs, FolderID const& rhs);
	constexpr friend bool operator<(FolderID const& lhs, FolderID const& rhs);
	friend struct std::hash<FolderID>;

private:
	std::size_t id;
};

struct AudioInstanceID {
	constexpr AudioInstanceID();
	constexpr AudioInstanceID(std::size_t id);
	constexpr explicit operator std::size_t() const;

public:
	constexpr friend bool operator==(AudioInstanceID const& lhs, AudioInstanceID const& rhs);
	constexpr friend bool operator<(AudioInstanceID const& lhs, AudioInstanceID const& rhs);
	friend struct std::hash<AudioInstanceID>;

private:
	std::size_t id;
};

// ========================================
// Euler angles (in radians)
// ========================================
struct EulerAngles {
	constexpr EulerAngles(glm::vec3 eulerAngles);
	EulerAngles(glm::quat quartenion);

	constexpr operator glm::quat() const;
	constexpr explicit operator glm::vec3() const;
	constexpr friend bool operator==(EulerAngles const& lhs, EulerAngles const& rhs);

public:
	glm::vec3 angles;
};

#include "type_alias.ipp"