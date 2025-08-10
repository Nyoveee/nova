#pragma once

#include "export.h"
#include <cstddef>

// Generates a unique number for every new template type argument it encounters.
// Returns the same generated unique number for template type argument it already encountered.
// Useful to give my component types a ID.
namespace Family {
	template<typename T>
	std::size_t id() noexcept {
		return typeid(T).hash_code();
	}
};