#include "loader.h"

std::optional<ResourceConstructor> ResourceLoader<EquirectangularMap>::load(ResourceID id, ResourceFilePath const& resourceFilePath) {
	gli::texture texture = gli::load(resourceFilePath.string);

	if (texture.empty()) {
		return std::nullopt;
	}

	gli::gl GL{ gli::gl::PROFILE_GL33 };

	gli::gl::format format = GL.translate(texture.format(), texture.swizzles());

	return { ResourceConstructor{[id, resourceFilePath, texture = std::move(texture), format = std::move(format)]() {
		return std::make_unique<EquirectangularMap>(id, std::move(resourceFilePath), std::move(texture), std::move(format));
	}} };
}