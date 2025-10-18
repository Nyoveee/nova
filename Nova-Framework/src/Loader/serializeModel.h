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
	[] <bool flag = false, typename Ty = T, bool test = isOptional<T>>() {
		static_assert(flag, "Did not account for all data members. " __FUNCSIG__);
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

// quat.
template <>
inline void serializeModel<glm::quat>(std::ofstream& outputFile, glm::quat const& dataMember) {
	writeBytesToFile(outputFile, dataMember.w);
	writeBytesToFile(outputFile, dataMember.x);
	writeBytesToFile(outputFile, dataMember.y);
	writeBytesToFile(outputFile, dataMember.z);
}

// ----------------------------------------------------
// unordered_map entry.. handling pair with function overloading..
template <isPair T>
inline void serializeModel(std::ofstream& outputFile, T const& pair) {
	serializeModel(outputFile, pair.first);
	serializeModel(outputFile, pair.second);
}

// ----------------------------------------------------
// optional..
template <isOptional T>
inline void serializeModel(std::ofstream& outputFile, T const& optional) {
	char firstByte = optional ? 1 : 0;
	outputFile.write(&firstByte, 1);

	if (optional) {
		serializeModel<typename T::value_type>(outputFile, optional.value());
	}
}

// ----------------------------------------------------
// handling containers with function overloading..
template <typename T> requires isVector<T> || isUnorderedSet<T> || is_std_array<T>::value
inline void serializeModel(std::ofstream& outputFile, T const& container) {
	writeBytesToFile(outputFile, container.size());
	
	for (auto&& element : container) {
		serializeModel(outputFile, element);
	}
}

template <isUnorderedMap T>
inline void serializeModel(std::ofstream& outputFile, T const& unordered_map) {
	writeBytesToFile(outputFile, unordered_map.size());

	for (auto&& pair : unordered_map) {
		serializeModel(outputFile, pair);
	}
}

// ----------------------------------------------------
// serialising anything that is reflectable.
template <typename T> requires reflection::conceptReflectable<T>
inline void serializeModel(std::ofstream& outputFile, T const& dataMember) {
	reflection::visit([&](auto&& fieldData) {
		auto&& innerDataMember = fieldData.get();
		using DataMemberType = std::decay_t<decltype(innerDataMember)>;

		serializeModel<DataMemberType>(outputFile, innerDataMember);
		}, dataMember);
}