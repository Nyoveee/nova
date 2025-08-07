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
// DEGREE AND ANGLES.
// ========================================