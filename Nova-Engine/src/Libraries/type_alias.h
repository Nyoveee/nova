#pragma once

// We provide strong type alias here. Type strongly describes the value and can catch errors in compile time. 
// Very useful for not using values unintended. For an example, you won't want to add GUIDs together, and 
// you don't want any arbitrary int to act as id.

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

#include "type_alias.ipp"