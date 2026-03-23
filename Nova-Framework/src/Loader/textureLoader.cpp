#include "loader.h"

#include <glad/glad.h>
#include <gli/gli.hpp>

#include "texture.h"

#include "Serialisation/serialisation.h"
#include "Serialisation/deserializeFromBinary.h"

// Documentation
// https://github.com/g-truc/gli/blob/master/manual.md

std::optional<ResourceConstructor> ResourceLoader<Texture>::load(ResourceID id, ResourceFilePath const& resourceFilePath) {
	std::ifstream ifs;
	// Load Sampling Data
	std::string wrappingStr{};
	ifs.open(resourceFilePath.string, std::ios::binary);
	deserializeFromBinary(ifs, wrappingStr);
	
	auto wrappingPtr = magic_enum::enum_cast<Texture::Wrapping>(wrappingStr);
	// Main Texture Data
	Texture::Wrapping wrapping = wrappingPtr.value();
	std::size_t start = ifs.tellg();
	ifs.seekg(0, std::ios::end);
	std::size_t end = ifs.tellg();
	ifs.seekg(start);
	std::size_t fileSize{ end - start };
	std::vector<char> data(fileSize);
	ifs.read(data.data(), fileSize);
	gli::texture  texture = gli::load(data.data(), data.size());
	
	gli::gl GL{ gli::gl::PROFILE_GL33 };

	gli::gl::format format = GL.translate(texture.format(), texture.swizzles());
	
	
	return { ResourceConstructor{[id, resourceFilePath, texture = std::move(texture), format = std::move(format), wrapping = std::move(wrapping)]() {
		return std::make_unique<Texture>(id, std::move(resourceFilePath), std::move(texture), std::move(format), std::move(wrapping));
	}} };
}

std::optional<ResourceConstructor> ResourceLoader<Texture>::loadWithoutDescriptor(ResourceFilePath const& resourceFilePath) {
	gli::texture texture = gli::load(resourceFilePath.string);

	gli::gl GL{ gli::gl::PROFILE_GL33 };

	gli::gl::format format = GL.translate(texture.format(), texture.swizzles());


	return { ResourceConstructor{[resourceFilePath, texture = std::move(texture), format = std::move(format)]() {
		return std::make_unique<Texture>(INVALID_RESOURCE_ID, std::move(resourceFilePath), std::move(texture), std::move(format), Texture::Wrapping::Repeat);
	}} };
}