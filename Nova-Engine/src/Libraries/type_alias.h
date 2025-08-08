#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

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

#include "type_alias.ipp"