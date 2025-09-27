#include <numbers>
#include <string>
#include "family.h"
#include "type_alias.h"

// ========================================
// DEGREE AND ANGLES.
// ========================================
namespace {
	constexpr Radian toRadian(Degree angle) {
		return angle * (std::numbers::pi_v<float> / 180.f);
	}
	constexpr Degree toDegree(Radian angle) {
		return angle * (180.f / std::numbers::pi_v<float>);
	}
}

constexpr Degree::Degree()					: angle	{}							{}
constexpr Degree::Degree(float angle)		: angle	{ angle }					{}
constexpr Degree::Degree(Radian radian)		: angle	{ toDegree(radian) }		{}
constexpr Degree::operator Radian() const			{ return toRadian(angle);	}
constexpr Degree::operator float() const			{ return angle;				}
constexpr Degree& Degree::operator+=(float f)		{ angle += f; return *this; }

constexpr Radian::Radian()					: angle	{}							{}
constexpr Radian::Radian(float angle)		: angle	{ angle }					{}
constexpr Radian::Radian(Degree radian)		: angle	{ toRadian(radian) }		{}
constexpr Radian::operator Degree() const			{ return toDegree(angle);	}
constexpr Radian::operator float() const			{ return angle;				}
constexpr Radian& Radian::operator+=(float f)		{ angle += f; return *this; }

// ========================================
// COLORS!
// ========================================

constexpr Color::Color()										: color	{}				{}
constexpr Color::Color(glm::vec3 color)							: color	{ color }		{}
constexpr Color::Color(float r, float g, float b)				: color	{ r, g, b }		{}
constexpr Color::operator glm::vec3() const								{ return color; }
constexpr float Color::r() const										{ return color.x; };
constexpr float Color::g() const										{ return color.y; };
constexpr float Color::b() const										{ return color.z; };

constexpr ColorA::ColorA()										: color	{} {}
constexpr ColorA::ColorA(glm::vec4 color)						: color	{ color } {}
constexpr float ColorA::r() const										{ return color.x; };
constexpr float ColorA::g() const										{ return color.y; };
constexpr float ColorA::b() const										{ return color.z; };
constexpr float ColorA::a() const										{ return color.w; };
constexpr ColorA::ColorA(float r, float g, float b, float a)	: color	{ r, g, b, a } {}
constexpr ColorA::operator glm::vec4() const							{ return color; }
constexpr ColorA::operator glm::vec3() const							{ return Color{ color.r, color.g, color.g }; }
constexpr ColorA::operator Color() const								{ return Color{ color.r, color.g, color.g }; }

// ========================================
// ID!
// ========================================

// ================ EventID ==================
constexpr EventID::EventID(std::size_t id) : id{ id } {}

// For hashing, we need to implement these member functions..
constexpr bool operator==(EventID const& lhs, EventID const& rhs) {
	return lhs.id == rhs.id;
}

constexpr bool operator<(EventID const& lhs, EventID const& rhs) {
	return lhs.id < rhs.id;
}

template<>
struct std::hash<EventID> {
	std::size_t operator()(EventID const& eventId) const noexcept {
		return std::hash<std::size_t>{}(eventId.id);
	}
};

// ================ AssetID ==================
constexpr ResourceID::ResourceID() : id{} {}
constexpr ResourceID::ResourceID(std::size_t id) : id{ id } {}

constexpr bool operator==(ResourceID const& lhs, ResourceID const& rhs) {
	return lhs.id == rhs.id;
}

constexpr bool operator<(ResourceID const& lhs, ResourceID const& rhs) {
	return lhs.id < rhs.id;
}

template<>
struct std::hash<ResourceID> {
	std::size_t operator()(ResourceID const& assetId) const noexcept {
		return std::hash<std::size_t>{}(assetId.id);
	}
};

constexpr ResourceID::operator std::size_t() const {
	return id;
}

#undef max
constexpr inline ResourceID INVALID_RESOURCE_ID{ std::numeric_limits<std::size_t>::max() };

// ================ AssetTypeID ==================
constexpr ResourceTypeID::ResourceTypeID() : id{} {}
constexpr ResourceTypeID::ResourceTypeID(std::size_t id) : id{ id } {}

constexpr bool operator==(ResourceTypeID const& lhs, ResourceTypeID const& rhs) {
	return lhs.id == rhs.id;
}

constexpr bool operator<(ResourceTypeID const& lhs, ResourceTypeID const& rhs) {
	return lhs.id < rhs.id;
}

template<>
struct std::hash<ResourceTypeID> {
	std::size_t operator()(ResourceTypeID const& assetTypeId) const noexcept {
		return std::hash<std::size_t>{}(assetTypeId.id);
	}
};

constexpr ResourceTypeID::operator std::size_t() const {
	return id;
}

// ================ FolderID ==================
constexpr FolderID::FolderID() : id{} {}
constexpr FolderID::FolderID(std::size_t id) : id{ id } {}

constexpr bool operator==(FolderID const& lhs, FolderID const& rhs) {
	return lhs.id == rhs.id;
}

constexpr bool operator<(FolderID const& lhs, FolderID const& rhs) {
	return lhs.id < rhs.id;
}

template<>
struct std::hash<FolderID> {
	std::size_t operator()(FolderID const& folderId) const noexcept {
		return std::hash<std::size_t>{}(folderId.id);
	}
};

constexpr FolderID::operator std::size_t() const {
	return id;
}

constexpr inline FolderID ASSET_FOLDER{ std::numeric_limits<std::size_t>::max() };

// ================ AudioInstanceID ==================
constexpr AudioInstanceID::AudioInstanceID() : id{} {}
constexpr AudioInstanceID::AudioInstanceID(std::size_t id) : id{ id } {}

constexpr bool operator==(AudioInstanceID const& lhs, AudioInstanceID const& rhs) {
	return lhs.id == rhs.id;
}

constexpr bool operator<(AudioInstanceID const& lhs, AudioInstanceID const& rhs) {
	return lhs.id < rhs.id;
}

template<>
struct std::hash<AudioInstanceID> {
	std::size_t operator()(AudioInstanceID const& folderId) const noexcept {
		return std::hash<std::size_t>{}(folderId.id);
	}
};

constexpr AudioInstanceID::operator std::size_t() const {
	return id;
}

// ========================================
// Euler angles
// ========================================

constexpr EulerAngles::EulerAngles(glm::vec3 eulerAngles) : 
	angles{ eulerAngles }
{}

inline EulerAngles::EulerAngles(glm::quat quartenion) : 
	angles{ glm::eulerAngles(glm::normalize(quartenion)) }
{};

constexpr EulerAngles::operator glm::quat() const {
	return glm::quat{ angles };
}

constexpr EulerAngles::operator glm::vec3() const {
	return angles;
}

constexpr bool operator==(EulerAngles const& lhs, EulerAngles const& rhs) {
	return lhs.angles == rhs.angles;
}

// ========================================
// Filepaths
// ========================================

// ================ AssetFilePath ==================
inline AssetFilePath::AssetFilePath(std::filesystem::path path) : string{ std::move(path).string() } {}
constexpr AssetFilePath::AssetFilePath(std::string path) : string{ std::move(path) } {}

inline AssetFilePath::operator std::filesystem::path() const {
	return { string };
}

constexpr AssetFilePath::operator std::string() const {
	return string;
}

constexpr bool operator==(AssetFilePath const& lhs, AssetFilePath const& rhs) {
	return lhs.string == rhs.string;
}

template<>
struct std::hash<AssetFilePath> {
	std::size_t operator()(AssetFilePath const& assetFilePath) const noexcept {
		return std::hash<std::string>{}(assetFilePath.string);
	}
};

// ================ DescriptorFilePath ==================
inline DescriptorFilePath::DescriptorFilePath(std::filesystem::path path) : string{ std::move(path).string() } {}
constexpr DescriptorFilePath::DescriptorFilePath(std::string path) : string{ std::move(path) } {}

inline DescriptorFilePath::operator std::filesystem::path() const {
	return { string };
}

constexpr DescriptorFilePath::operator std::string() const {
	return string;
}

constexpr bool operator==(DescriptorFilePath const& lhs, DescriptorFilePath const& rhs) {
	return lhs.string == rhs.string;
}

template<>
struct std::hash<DescriptorFilePath> {
	std::size_t operator()(DescriptorFilePath const& descriptorFilePath) const noexcept {
		return std::hash<std::string>{}(descriptorFilePath.string);
	}
};

// ================ ResourceFilePath ==================
inline ResourceFilePath::ResourceFilePath(std::filesystem::path path) : string{ std::move(path).string() } {}
constexpr ResourceFilePath::ResourceFilePath(std::string path) : string{ std::move(path) } {}

inline ResourceFilePath::operator std::filesystem::path() const {
	return { string };
}

constexpr ResourceFilePath::operator std::string() const {
	return string;
}

constexpr bool operator==(ResourceFilePath const& lhs, ResourceFilePath const& rhs) {
	return lhs.string == rhs.string;
}

template<>
struct std::hash<ResourceFilePath> {
	std::size_t operator()(ResourceFilePath const& resourceFilePath) const noexcept {
		return std::hash<std::string>{}(resourceFilePath.string);
	}
};
