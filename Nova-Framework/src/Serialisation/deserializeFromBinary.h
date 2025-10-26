#pragma once

#include <glm/gtc/type_ptr.hpp>
#include <magic_enum.hpp>

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
inline void deserializeFromBinary(std::ifstream& inputFile, T& dataMember) {
	[] <bool flag = false, typename Ty = T, bool test = reflection::conceptReflectable<T>>() {
		static_assert(flag, "Did not account for all data members. " __FUNCSIG__);
	}();
}

// ----------------------------------------------------
// fundemental types are easy to handle..
template <typename T> requires std::is_fundamental_v<T>
inline void deserializeFromBinary(std::ifstream& inputFile, T& dataMember) {
	readFromFile(inputFile, dataMember);
}

// ----------------------------------------------------
// std::variant..

template <typename... Ts>
void initialiseVariant(std::ifstream& inputFile, std::size_t i, std::variant<Ts...>& variant) {
	assert(i < sizeof...(Ts));
	static std::variant<Ts...> table[] = { Ts{ }... };

	// we now retrieve the variant with the type belonging to this specific index.
	variant = table[i];

	// we then initialise the underlying variant value..
	std::visit([&](auto&& value) {
		deserializeFromBinary(inputFile, value);
	}, variant);
}

template <typename T> requires is_variant<T>::value
inline void deserializeFromBinary(std::ifstream& inputFile, T& variant) {
	// retrieve the index in runtime..
	std::size_t index;
	readFromFile(inputFile, index);

	// now comes the hardest part.
	// we need to assign the correct type to our variant based on our index.
	// but our index is a runtime value, so how can we construct a type (compile time constant) based on a runtime value?
	// we "promote" our runtime integer to compile time integral constants.
	// here, reddit saves the day. https://www.reddit.com/r/cpp/comments/f8cbzs/creating_stdvariant_based_on_index_at_runtime/
	initialiseVariant(inputFile, index, variant);
}

// ----------------------------------------------------
// normalised float
template <>
inline void deserializeFromBinary<NormalizedFloat>(std::ifstream& inputFile, NormalizedFloat& dataMember) {
	float value;
	readFromFile(inputFile, value);
	dataMember = value;
}

// vector 4
template <>
inline void deserializeFromBinary<glm::vec4>(std::ifstream& inputFile, glm::vec4& dataMember) {
	readFromFile(inputFile, dataMember.x);
	readFromFile(inputFile, dataMember.y);
	readFromFile(inputFile, dataMember.z);
	readFromFile(inputFile, dataMember.w);
}

// vector 3
template <>
inline void deserializeFromBinary<glm::vec3>(std::ifstream& inputFile, glm::vec3& dataMember) {
	readFromFile(inputFile, dataMember.x);
	readFromFile(inputFile, dataMember.y);
	readFromFile(inputFile, dataMember.z);
}

// vector 2
template <>
inline void deserializeFromBinary<glm::vec2>(std::ifstream& inputFile, glm::vec2& dataMember) {
	readFromFile(inputFile, dataMember.x);
	readFromFile(inputFile, dataMember.y);
}

// ivector 2
template <>
inline void deserializeFromBinary<glm::ivec2>(std::ifstream& inputFile, glm::ivec2& dataMember) {
	readFromFile(inputFile, dataMember.x);
	readFromFile(inputFile, dataMember.y);
}

// ivector 3
template <>
inline void deserializeFromBinary<glm::ivec3>(std::ifstream& inputFile, glm::ivec3& dataMember) {
	readFromFile(inputFile, dataMember.x);
	readFromFile(inputFile, dataMember.y);
	readFromFile(inputFile, dataMember.z);
}

// ivector 4
template <>
inline void deserializeFromBinary<glm::ivec4>(std::ifstream& inputFile, glm::ivec4& dataMember) {
	readFromFile(inputFile, dataMember.x);
	readFromFile(inputFile, dataMember.y);
	readFromFile(inputFile, dataMember.z);
	readFromFile(inputFile, dataMember.w);
}

// mat 3
template <>
inline void deserializeFromBinary<glm::mat3x3>(std::ifstream& inputFile, glm::mat3x3& dataMember) {
	static_assert(sizeof(glm::mat3x3) == 36);
	inputFile.read(reinterpret_cast<char*>(glm::value_ptr(dataMember)), sizeof(glm::mat3x3));
}

// mat 4
template <>
inline void deserializeFromBinary<glm::mat4x4>(std::ifstream& inputFile, glm::mat4x4& dataMember) {
	static_assert(sizeof(glm::mat4x4) == 64);
	inputFile.read(reinterpret_cast<char*>(glm::value_ptr(dataMember)), sizeof(glm::mat4x4));
}

// string
template <>
inline void deserializeFromBinary<std::string>(std::ifstream& inputFile, std::string& string) {
	std::size_t length;
	readFromFile(inputFile, length);
	string.resize(length);

	inputFile.read(string.data(), length);
}

// quat.
template <>
inline void deserializeFromBinary<glm::quat>(std::ifstream& inputFile, glm::quat& dataMember) {
	readFromFile(inputFile, dataMember.w);
	readFromFile(inputFile, dataMember.x);
	readFromFile(inputFile, dataMember.y);
	readFromFile(inputFile, dataMember.z);
}

// resource id
template <>
inline void deserializeFromBinary<ResourceID>(std::ifstream& inputFile, ResourceID& dataMember) {
	std::size_t id;
	readFromFile(inputFile, id);
	dataMember = ResourceID{ id };
}

// ----------------------------------------------------
// unordered_map entry.. handling pair with function overloading..
template <isPair T>
inline void deserializeFromBinary(std::ifstream& inputFile, T& pair) {
	deserializeFromBinary(inputFile, pair.first);
	deserializeFromBinary(inputFile, pair.second);
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
// optional..
template <isOptional T>
inline void deserializeFromBinary(std::ifstream& inputFile, T& optional) {
	char firstByte;
	inputFile.read(&firstByte, 1);

	if (firstByte == 0) {
		// optional was null, nothing is serialised.
		return;
	}
	else if (firstByte == 1) {
		optional = typename T::value_type{}; // default construct the underlying type.
		deserializeFromBinary<typename T::value_type>(inputFile, optional.value());
	}
	else {
		assert(false && "Faulty first byte");
	}
}

// ----------------------------------------------------
template<IsTypedResourceID T>
inline void deserializeFromBinary(std::ifstream& inputFile, T& typedResourceID) {
	deserializeFromBinary<ResourceID>(inputFile, typedResourceID);
}

// ----------------------------------------------------
// our strongly typed ids..
template<isTypedID T>
inline void deserializeFromBinary(std::ifstream& inputFile, T& id) {
	typename T::Underlying_ID underlyingId;
	deserializeFromBinary(inputFile, underlyingId);
	id = underlyingId;
}

// ----------------------------------------------------
// enum..
template <IsEnum T>
inline void deserializeFromBinary(std::ifstream& inputFile, T& enumDataMember) {
	std::string parsedEnumString;
	deserializeFromBinary<std::string>(inputFile, parsedEnumString);

	// lets convert string to enum..
	std::optional<T> enumValueOpt = magic_enum::enum_cast<T>(parsedEnumString);

	if (enumValueOpt) {
		enumDataMember = enumValueOpt.value();
	}
	else {
		Logger::error("Failed to deserialise enum..");
	}
}

// ----------------------------------------------------
// handling containers with function overloading..
template <typename T> requires isVector<T> || isUnorderedSet<T> || isUnorderedMap<T> || is_std_array<T>::value
inline void deserializeFromBinary(std::ifstream& inputFile, T& container) {
	std::size_t length;
	readFromFile(inputFile, length);
	
	// we reserve space for vector..
	if constexpr (isVector<T>) {
		container.reserve(length);
	}

	// we iterate through the container..
	for (int i = 0; i < length; ++i) {
		typename Element<T>::Type element;

		deserializeFromBinary<typename Element<T>::Type>(inputFile, element);

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
inline void deserializeFromBinary(std::ifstream& inputFile, T& dataMember) {
	reflection::visit([&](auto&& fieldData) {
		auto&& innerDataMember = fieldData.get();
		using DataMemberType = std::decay_t<decltype(innerDataMember)>;

		deserializeFromBinary<DataMemberType>(inputFile, innerDataMember);
	}, dataMember);
}