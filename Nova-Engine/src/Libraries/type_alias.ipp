#include <numbers>
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

constexpr ColorA::ColorA()										: color	{} {}
constexpr ColorA::ColorA(glm::vec4 color)						: color	{ color } {}
constexpr ColorA::ColorA(float r, float g, float b, float a)	: color	{ r, g, b, a } {}
constexpr ColorA::operator glm::vec4() const							{ return color; }
constexpr ColorA::operator glm::vec3() const							{ return Color{ color.r, color.g, color.g }; }
constexpr ColorA::operator Color() const								{ return Color{ color.r, color.g, color.g }; }