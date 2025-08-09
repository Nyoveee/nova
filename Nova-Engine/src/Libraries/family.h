#pragma once

#include <cstddef>

// Generates a unique number for every new template type argument it encounters.
// Returns the same generated unique number for template type argument it already encountered.
// Useful to give my component types a ID.
class Family {
	static std::size_t identifier() noexcept {
		static std::size_t value = 0;

		return value++;
	}

public:
	template<typename>
	static std::size_t id() noexcept {
		static const std::size_t value = identifier();
		return value;
	}
};