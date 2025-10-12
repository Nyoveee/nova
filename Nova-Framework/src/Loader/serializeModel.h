#pragma once

#include "type_concepts.h"

// === Helper function to read a specific type from file === ..
template <typename T>
void writeBytesToFile(std::ofstream& outputFile, T const& data) {
	outputFile.write(reinterpret_cast<const char*>(&data), sizeof(data));
}

// ==========================================================================
// Serialization..
// ==========================================================================

// ----------------------------------------------------
// default implementation, if not explicitly implemented we flag an compile time error.
template <typename T>
inline void serializeModel(std::ofstream& outputFile, T const& dataMember) {
	[] <bool flag = false>() {
		static_assert(false, "Did not account for all data members. " __FUNCSIG__);
	}();
}

// ----------------------------------------------------
// fundemental types are easy to handle..
template <typename T> requires std::is_fundamental_v<T>
inline void serializeModel(std::ofstream& outputFile, T const& dataMember) {
	writeBytesToFile(outputFile, dataMember);
}

// vector 3
template <>
inline void serializeModel<glm::vec3>(std::ofstream& outputFile, glm::vec3 const& dataMember) {
	writeBytesToFile(outputFile, dataMember.x);
	writeBytesToFile(outputFile, dataMember.y);
	writeBytesToFile(outputFile, dataMember.z);

}

// vector 2
template <>
inline void serializeModel<glm::vec2>(std::ofstream& outputFile, glm::vec2 const& dataMember) {
	writeBytesToFile(outputFile, dataMember.x);
	writeBytesToFile(outputFile, dataMember.y);
}

// mat 4
template <>
inline void serializeModel<glm::mat4x4>(std::ofstream& outputFile, glm::mat4x4 const& dataMember) {
	static_assert(sizeof(glm::mat4x4) == 64);
	outputFile.write(reinterpret_cast<char const*>(glm::value_ptr(dataMember)), sizeof(glm::mat4x4));
}

// string
template <>
inline void serializeModel<std::string>(std::ofstream& outputFile, std::string const& string) {
	writeBytesToFile(outputFile, string.size());
	outputFile.write(string.data(), string.size());
}

// ----------------------------------------------------
// unordered_map entry.. handling pair with function overloading..
template <isPair T>
inline void serializeModel(std::ofstream& outputFile, T const& pair) {
	serializeModel(outputFile, pair.first);
	serializeModel(outputFile, pair.second);
}

// ----------------------------------------------------
// handling containers with function overloading..
template <typename T> requires isVector<T> || isUnorderedSet<T> || is_std_array<T>::value
inline void serializeModel(std::ofstream& outputFile, T const& container) {
	writeBytesToFile(outputFile, container.size());
	
	for (auto&& element : container) {
		if constexpr (reflection::isReflectable<std::decay_t<decltype(element)>>()) {
			reflection::visit([&](auto&& fieldData) {
				auto&& dataMember = fieldData.get();
				using DataMemberType = std::decay_t<decltype(dataMember)>;
				serializeModel<DataMemberType>(outputFile, dataMember);
			}, element);
		}
		else {
			serializeModel(outputFile, element);
		}
	}
}

template <isUnorderedMap T>
inline void serializeModel(std::ofstream& outputFile, T const& unordered_map) {
	writeBytesToFile(outputFile, unordered_map.size());

	for (auto&& pair : unordered_map) {
		serializeModel(outputFile, pair);
	}
}