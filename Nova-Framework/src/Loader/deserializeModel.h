#pragma once

#include "type_concepts.h"

// === Helper function to read a specific type from file === ..
template <typename T>
void readFromFile(std::ifstream& inputFile, T& data) {
	inputFile.read(reinterpret_cast<char*>(&data), sizeof(data));
}

// ==========================================================================
// Deserialization..
// ==========================================================================

// ----------------------------------------------------
// default implementation, if not explicitly implemented we flag an compile time error.
template <typename T>
inline void deserializeModel(std::ifstream& inputFile, T& dataMember) {
	[] <bool flag = false, typename Ty = T, bool test = reflection::conceptReflectable<T>>() {
		static_assert(flag, "Did not account for all data members. " __FUNCSIG__);
	}();
}

// ----------------------------------------------------
// fundemental types are easy to handle..
template <typename T> requires std::is_fundamental_v<T>
inline void deserializeModel(std::ifstream& inputFile, T& dataMember) {
	readFromFile(inputFile, dataMember);
}

// vector 3
template <>
inline void deserializeModel<glm::vec3>(std::ifstream& inputFile, glm::vec3& dataMember) {
	readFromFile(inputFile, dataMember.x);
	readFromFile(inputFile, dataMember.y);
	readFromFile(inputFile, dataMember.z);

}

// vector 2
template <>
inline void deserializeModel<glm::vec2>(std::ifstream& inputFile, glm::vec2& dataMember) {
	readFromFile(inputFile, dataMember.x);
	readFromFile(inputFile, dataMember.y);
}

// mat 4
template <>
inline void deserializeModel<glm::mat4x4>(std::ifstream& inputFile, glm::mat4x4& dataMember) {
	static_assert(sizeof(glm::mat4x4) == 64);
	inputFile.read(reinterpret_cast<char*>(glm::value_ptr(dataMember)), sizeof(glm::mat4x4));
}

// string
template <>
inline void deserializeModel<std::string>(std::ifstream& inputFile, std::string& string) {
	std::size_t length;
	readFromFile(inputFile, length);
	string.resize(length);

	inputFile.read(string.data(), length);
}

// quat.
template <>
inline void deserializeModel<glm::quat>(std::ifstream& inputFile, glm::quat& dataMember) {
	readFromFile(inputFile, dataMember.w);
	readFromFile(inputFile, dataMember.x);
	readFromFile(inputFile, dataMember.y);
	readFromFile(inputFile, dataMember.z);
}

// ----------------------------------------------------
// unordered_map entry.. handling pair with function overloading..
template <isPair T>
inline void deserializeModel(std::ifstream& inputFile, T& pair) {
	deserializeModel(inputFile, pair.first);
	deserializeModel(inputFile, pair.second);
}

// we use partial template specialisation to redefine our element type..
template <typename T>
struct Element {
	using Type = typename T::value_type;
};

template <isUnorderedMap T>
struct Element<T> {
	using Type = std::pair<typename T::key_type, typename T::mapped_type>;
};

// ----------------------------------------------------
// handling containers with function overloading..
template <typename T> requires isVector<T> || isUnorderedSet<T> || isUnorderedMap<T> || is_std_array<T>::value
inline void deserializeModel(std::ifstream& inputFile, T& container) {
	std::size_t length;
	readFromFile(inputFile, length);
	
	// we reserve space for vector..
	if constexpr (isVector<T>) {
		container.reserve(length);
	}

	// we iterate through the container..
	for (int i = 0; i < length; ++i) {
		typename Element<T>::Type element;

		deserializeModel<typename Element<T>::Type>(inputFile, element);

		// we deserialised each element, time to add them to the respective containers..
		if constexpr (isVector<T>) {
			container.push_back(std::move(element));
		}
		else if constexpr (isUnorderedSet<T> || isUnorderedMap<T>) {
			container.insert(std::move(element));
		}
		else if constexpr (is_std_array<T>::value) {
			container[i] = std::move(element);
		}
		else {
			[] <bool flag = false>() {
				// serves as a guard rail.
				static_assert(false, "Did not account for all containers. " __FUNCSIG__);
			}();
		}
	}
}

// ----------------------------------------------------
// deserialising anything that is reflectable.
template <typename T> requires reflection::conceptReflectable<T>
inline void deserializeModel(std::ifstream& inputFile, T& dataMember) {
	reflection::visit([&](auto&& fieldData) {
		auto&& innerDataMember = fieldData.get();
		using DataMemberType = std::decay_t<decltype(innerDataMember)>;

		deserializeModel<DataMemberType>(inputFile, innerDataMember);
	}, dataMember);
}