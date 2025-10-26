#pragma once

#include <glm/gtc/type_ptr.hpp>
#include <magic_enum.hpp>

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
inline void serializeToBinary(std::ofstream& outputFile, T const& dataMember) {
	[] <bool flag = false, typename Ty = T, bool test = is_variant<T>::value>() {
		static_assert(flag, "Did not account for all data members. " __FUNCSIG__);
	}();
}

// ----------------------------------------------------
// fundemental types are easy to handle..
template <typename T> requires std::is_fundamental_v<T>
inline void serializeToBinary(std::ofstream& outputFile, T const& dataMember) {
	writeBytesToFile(outputFile, dataMember);
}

// ----------------------------------------------------
// variant..
template <typename T> requires is_variant<T>::value
inline void serializeToBinary(std::ofstream& outputFile, T const& variant) {
	// serialise the index of this current variant.
	serializeToBinary(outputFile, variant.index());

	// serialise the current value, retrieving the underlying variant value & type.
	std::visit([&](auto&& value) {
		serializeToBinary(outputFile, value);
	}, variant);
}

// normalized float
template <>
inline void serializeToBinary<NormalizedFloat>(std::ofstream& outputFile, NormalizedFloat const& dataMember) {
	float value = dataMember;
	writeBytesToFile(outputFile, value);
	writeBytesToFile(outputFile, value);
	writeBytesToFile(outputFile, value);
	writeBytesToFile(outputFile, value);
}

// vector 4
template <>
inline void serializeToBinary<glm::vec4>(std::ofstream& outputFile, glm::vec4 const& dataMember) {
	writeBytesToFile(outputFile, dataMember.x);
	writeBytesToFile(outputFile, dataMember.y);
	writeBytesToFile(outputFile, dataMember.z);
	writeBytesToFile(outputFile, dataMember.w);
}

// vector 3
template <>
inline void serializeToBinary<glm::vec3>(std::ofstream& outputFile, glm::vec3 const& dataMember) {
	writeBytesToFile(outputFile, dataMember.x);
	writeBytesToFile(outputFile, dataMember.y);
	writeBytesToFile(outputFile, dataMember.z);
}

// vector 2
template <>
inline void serializeToBinary<glm::vec2>(std::ofstream& outputFile, glm::vec2 const& dataMember) {
	writeBytesToFile(outputFile, dataMember.x);
	writeBytesToFile(outputFile, dataMember.y);
}

// ivector 2
template <>
inline void serializeToBinary<glm::ivec2>(std::ofstream& outputFile, glm::ivec2 const& dataMember) {
	writeBytesToFile(outputFile, dataMember.x);
	writeBytesToFile(outputFile, dataMember.y);
}

// ivector 3
template <>
inline void serializeToBinary<glm::ivec3>(std::ofstream& outputFile, glm::ivec3 const& dataMember) {
	writeBytesToFile(outputFile, dataMember.x);
	writeBytesToFile(outputFile, dataMember.y);
	writeBytesToFile(outputFile, dataMember.z);
}

// ivector 4
template <>
inline void serializeToBinary<glm::ivec4>(std::ofstream& outputFile, glm::ivec4 const& dataMember) {
	writeBytesToFile(outputFile, dataMember.x);
	writeBytesToFile(outputFile, dataMember.y);
	writeBytesToFile(outputFile, dataMember.z);
	writeBytesToFile(outputFile, dataMember.w);
}

// mat 3
template <>
inline void serializeToBinary<glm::mat3x3>(std::ofstream& outputFile, glm::mat3x3 const& dataMember) {
	static_assert(sizeof(glm::mat3x3) == 36);
	outputFile.write(reinterpret_cast<char const*>(glm::value_ptr(dataMember)), sizeof(glm::mat3x3));
}

// mat 4
template <>
inline void serializeToBinary<glm::mat4x4>(std::ofstream& outputFile, glm::mat4x4 const& dataMember) {
	static_assert(sizeof(glm::mat4x4) == 64);
	outputFile.write(reinterpret_cast<char const*>(glm::value_ptr(dataMember)), sizeof(glm::mat4x4));
}

// string
template <>
inline void serializeToBinary<std::string>(std::ofstream& outputFile, std::string const& string) {
	writeBytesToFile(outputFile, string.size());
	outputFile.write(string.data(), string.size());
}

// quat.
template <>
inline void serializeToBinary<glm::quat>(std::ofstream& outputFile, glm::quat const& dataMember) {
	writeBytesToFile(outputFile, dataMember.w);
	writeBytesToFile(outputFile, dataMember.x);
	writeBytesToFile(outputFile, dataMember.y);
	writeBytesToFile(outputFile, dataMember.z);
}

// resource id
template <>
inline void serializeToBinary<ResourceID>(std::ofstream& outputFile, ResourceID const& id) {
	writeBytesToFile(outputFile, static_cast<std::size_t>(id));
}

// ----------------------------------------------------
// unordered_map entry.. handling pair with function overloading..
template <isPair T>
inline void serializeToBinary(std::ofstream& outputFile, T const& pair) {
	serializeToBinary(outputFile, pair.first);
	serializeToBinary(outputFile, pair.second);
}

// ----------------------------------------------------
// optional..
template <isOptional T>
inline void serializeToBinary(std::ofstream& outputFile, T const& optional) {
	char firstByte = optional ? 1 : 0;
	outputFile.write(&firstByte, 1);

	if (optional) {
		serializeToBinary<typename T::value_type>(outputFile, optional.value());
	}
}

// ----------------------------------------------------
// enum..
template <IsEnum T>
inline void serializeToBinary(std::ofstream& outputFile, T const& enumValue) {
	std::string enumName{ magic_enum::enum_name(enumValue) };
	serializeToBinary(outputFile, enumName);
}

// ----------------------------------------------------
// type resource id
template <IsTypedResourceID T>
inline void serializeToBinary(std::ofstream& outputFile, T const& id) {
	serializeToBinary<ResourceID>(outputFile, id);
}

// ----------------------------------------------------
// our strongly typed ids..
template <isTypedID T>
inline void serializeToBinary(std::ofstream& outputFile, T const& id) {
	serializeToBinary(outputFile, static_cast<std::size_t>(id));
}

// ----------------------------------------------------
// handling containers with function overloading..
template <typename T> requires isVector<T> || isUnorderedSet<T> || is_std_array<T>::value
inline void serializeToBinary(std::ofstream& outputFile, T const& container) {
	writeBytesToFile(outputFile, container.size());
	
	for (auto&& element : container) {
		serializeToBinary(outputFile, element);
	}
}

template <isUnorderedMap T>
inline void serializeToBinary(std::ofstream& outputFile, T const& unordered_map) {
	writeBytesToFile(outputFile, unordered_map.size());

	for (auto&& pair : unordered_map) {
		serializeToBinary(outputFile, pair);
	}
}

// ----------------------------------------------------
// serialising anything that is reflectable.
template <typename T> requires reflection::conceptReflectable<T>
inline void serializeToBinary(std::ofstream& outputFile, T const& dataMember) {
	reflection::visit([&](auto&& fieldData) {
		auto&& innerDataMember = fieldData.get();
		using DataMemberType = std::decay_t<decltype(innerDataMember)>;

		serializeToBinary<DataMemberType>(outputFile, innerDataMember);
	}, dataMember);
}