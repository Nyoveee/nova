#pragma once

enum class KeyType {
	Keyboard,
	MouseClick,
	MouseMovement,
	Scroll
};

// A GLFWInput strongly types a GLFW input key (which using macros that have an underlying type of int) along with the associated key type.
struct GLFWInput {
	int key;
	KeyType type;
};

// For unordered_map.
constexpr bool operator==(GLFWInput const& lhs, GLFWInput const& rhs) {
	return lhs.key == rhs.key && lhs.type == lhs.type;
}

constexpr bool operator<(GLFWInput const& lhs, GLFWInput const& rhs) {
	if (lhs.type < rhs.type) {
		return true;
	}
	else if (lhs.type > rhs.type) {
		return false;
	}

	return lhs.key < rhs.key;
}

template<>
struct std::hash<GLFWInput> {
	std::size_t operator()(GLFWInput const& input) const noexcept {
		std::size_t h1 = std::hash<int>{}(input.key);
		std::size_t h2 = std::hash<int>{}(static_cast<int>(input.type));
		return h1 ^ (h2 << 1);;
	}
};