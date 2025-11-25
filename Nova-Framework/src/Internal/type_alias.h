#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/quaternion.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <cstddef>
#include <limits>
#include <filesystem>

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

	constexpr float r() const;
	constexpr float g() const;
	constexpr float b() const;

	constexpr operator glm::vec3() const;

public:
	constexpr friend Color operator*(Color const& lhs, Color const& rhs);

private:
	glm::vec3 color;
};

struct ColorA {
	constexpr ColorA();
	constexpr ColorA(glm::vec4 color);
	constexpr ColorA(float r, float g, float b, float a);

	constexpr float r() const;
	constexpr float g() const;
	constexpr float b() const;
	constexpr float a() const;

	constexpr operator glm::vec4() const;
	constexpr explicit operator glm::vec3() const;
	constexpr explicit operator Color() const;

public:
	constexpr friend ColorA operator*(ColorA const& lhs, ColorA const& rhs);

private:
	glm::vec4 color;
};

// ========================================
// Normalized float
// ========================================
struct NormalizedFloat {
	constexpr NormalizedFloat();
	constexpr NormalizedFloat(float value);

	constexpr operator float() const;

private:
	float value;
};

// ========================================
// IDs
// ========================================

// Every InputEvent (a type) in Input Manager will be associated with a unique number.

struct EventID {
	using Underlying_ID = std::size_t;

	constexpr explicit EventID(std::size_t id);

public:
	constexpr friend bool operator==(EventID const& lhs, EventID const& rhs);
	constexpr friend bool operator<(EventID const& lhs, EventID const& rhs);
	friend struct std::hash<EventID>;

private:
	Underlying_ID id;
};

struct ObserverID {
	using Underlying_ID = std::size_t;
	
	constexpr explicit ObserverID(std::size_t id);
	constexpr explicit operator std::size_t() const;

public:
	constexpr friend bool operator==(ObserverID const& lhs, ObserverID const& rhs);
	constexpr friend bool operator<(ObserverID const& lhs, ObserverID const& rhs);
	friend struct std::hash<ObserverID>;

private:
	Underlying_ID id;
};

// Game objects hold reference to assets by using an ID.
struct ResourceID {
	// using Underlying_ID = std::size_t;

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

public:
	// constexpr friend bool operator==(TypedResourceID<T> const& lhs, TypedResourceID<T> const& rhs);
};

// an id given to the template type parameter of assets.
// for an example, type Texture could have id of 5, and Model could have id of 7.
struct ResourceTypeID {
	using Underlying_ID = std::size_t;

	constexpr ResourceTypeID();
	constexpr ResourceTypeID(std::size_t id);
	constexpr explicit operator std::size_t() const;

public:
	constexpr friend bool operator==(ResourceTypeID const& lhs, ResourceTypeID const& rhs);
	constexpr friend bool operator<(ResourceTypeID const& lhs, ResourceTypeID const& rhs);
	friend struct std::hash<ResourceTypeID>;

private:
	Underlying_ID id;
};

struct FolderID {
	using Underlying_ID = std::size_t;

	constexpr FolderID();
	constexpr FolderID(std::size_t id);
	constexpr explicit operator std::size_t() const;

public:
	constexpr friend bool operator==(FolderID const& lhs, FolderID const& rhs);
	constexpr friend bool operator<(FolderID const& lhs, FolderID const& rhs);
	friend struct std::hash<FolderID>;

private:
	Underlying_ID id;
};

struct AudioInstanceID {
	using Underlying_ID = std::size_t;

	constexpr AudioInstanceID();
	constexpr AudioInstanceID(std::size_t id);
	constexpr explicit operator std::size_t() const;

public:
	constexpr friend bool operator==(AudioInstanceID const& lhs, AudioInstanceID const& rhs);
	constexpr friend bool operator<(AudioInstanceID const& lhs, AudioInstanceID const& rhs);
	friend struct std::hash<AudioInstanceID>;

private:
	Underlying_ID id;
};

struct ControllerNodeID {
	using Underlying_ID = std::size_t;

	constexpr ControllerNodeID();
	constexpr ControllerNodeID(std::size_t id);
	ControllerNodeID(std::string string);

	constexpr explicit operator std::size_t() const;
	inline explicit operator std::string() const;

public:
	constexpr friend bool operator==(ControllerNodeID const& lhs, ControllerNodeID const& rhs);
	constexpr friend bool operator<(ControllerNodeID const& lhs, ControllerNodeID const& rhs);
	friend struct std::hash<ControllerNodeID>;

private:
	Underlying_ID id;
};

// ========================================
// Euler angles (in radians)
// ========================================
struct EulerAngles {
	constexpr EulerAngles(glm::vec3 eulerAngles);
	EulerAngles(glm::quat quartenion);

	constexpr operator glm::quat() const;
	constexpr operator glm::vec3() const;
	constexpr friend bool operator==(EulerAngles const& lhs, EulerAngles const& rhs);

public:
	glm::vec3 angles;
};


// ========================================
// Filepaths
// (The different filepaths were way too confusing to keep track)
// 
// AssetFilePath		-> represents the filepath to the intermediary asset file.
// DescriptorFilePath	-> represents the filepath to the descriptor file
// ResourceFilePath		-> represents the filepath to the resource file
// ========================================
struct AssetFilePath {
	constexpr AssetFilePath() = default;
			  AssetFilePath(std::filesystem::path path);
	constexpr AssetFilePath(std::string path);

			  operator std::filesystem::path() const;
	constexpr operator std::string() const;

	constexpr friend bool operator==(AssetFilePath const& lhs, AssetFilePath const& rhs);
	friend struct std::hash<AssetFilePath>;

public:
	std::string string;
};

struct AssetCacheFilePath {
	constexpr AssetCacheFilePath() = default;
	AssetCacheFilePath(std::filesystem::path path);
	constexpr AssetCacheFilePath(std::string path);

	operator std::filesystem::path() const;
	constexpr operator std::string() const;

	constexpr friend bool operator==(AssetCacheFilePath const& lhs, AssetCacheFilePath const& rhs);
	friend struct std::hash<AssetCacheFilePath>;

public:
	std::string string;
};

struct DescriptorFilePath {
	constexpr DescriptorFilePath() = default;
			  DescriptorFilePath(std::filesystem::path path);
	constexpr DescriptorFilePath(std::string path);

			  operator std::filesystem::path() const;
	constexpr operator std::string() const;

	constexpr friend bool operator==(DescriptorFilePath const& lhs, DescriptorFilePath const& rhs);
	friend struct std::hash<DescriptorFilePath>;

public:
	std::string string;
};

struct ResourceFilePath {
	constexpr ResourceFilePath() = default;
			  ResourceFilePath(std::filesystem::path path);
	constexpr ResourceFilePath(std::string path);

			  operator std::filesystem::path() const;
	constexpr operator std::string() const;

	constexpr friend bool operator==(ResourceFilePath const& lhs, ResourceFilePath const& rhs);
	friend struct std::hash<ResourceFilePath>;

public:
	std::string string;
};

#include "type_alias.ipp"
