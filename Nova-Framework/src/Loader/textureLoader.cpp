#include "loader.h"

#include <glad/glad.h>
#include <gli/gli.hpp>

#include "texture.h"

// Documentation
// https://github.com/g-truc/gli/blob/master/manual.md

std::optional<ResourceConstructor> ResourceLoader<Texture>::load(ResourceID id, ResourceFilePath const& resourceFilePath) {
	gli::texture texture = gli::load(resourceFilePath.string);
	
	if (texture.empty()) {
		return std::nullopt;
	}

	gli::gl GL{ gli::gl::PROFILE_GL33 };

	gli::gl::format format = GL.translate(texture.format(), texture.swizzles());

	return { ResourceConstructor{[id, texture = std::move(texture), format = std::move(format)]() {
		return std::make_unique<Texture>(id, std::move(texture), std::move(format));
	}} };
}