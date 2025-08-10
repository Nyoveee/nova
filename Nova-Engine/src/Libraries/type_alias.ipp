#include <numbers>
#include <string>
#include "type_alias.h"
#include "family.h"

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

constexpr ColorA::ColorA()										: color	{} {}
constexpr ColorA::ColorA(glm::vec4 color)						: color	{ color } {}
constexpr ColorA::ColorA(float r, float g, float b, float a)	: color	{ r, g, b, a } {}
constexpr ColorA::operator glm::vec4() const							{ return color; }
constexpr ColorA::operator glm::vec3() const							{ return Color{ color.r, color.g, color.g }; }
constexpr ColorA::operator Color() const								{ return Color{ color.r, color.g, color.g }; }

// ========================================
// ID!
// ========================================

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

constexpr AssetID::AssetID() : id{} {}
constexpr AssetID::AssetID(std::size_t id) : id{ id } {}

constexpr bool operator==(AssetID const& lhs, AssetID const& rhs) {
	return lhs.id == rhs.id;
}

constexpr bool operator<(AssetID const& lhs, AssetID const& rhs) {
	return lhs.id < rhs.id;
}

template<>
struct std::hash<AssetID> {
	std::size_t operator()(AssetID const& assetId) const noexcept {
		return std::hash<std::size_t>{}(assetId.id);
	}
};