#include "asset.h"
#include "model.h"
#include "texture.h"

template<ValidAsset T>
void ResourceManager::parseResourceFile(std::filesystem::path const& filepath) {
	if constexpr (std::same_as<T, Texture>) {
		
	}
	else if constexpr (std::same_as<T, Model>) {

	}
	else {
		[] <bool flag = true>() {
			static_assert(flag, "Attempting to parse a unsupported resource file type.\n");
		}();
	}
}

template<ValidAsset T>
void ResourceManager::loadAllResources(std::filesystem::path const& directory) {
	// recursively iterate through a directory and parse all resource files.
	for (const auto& entry : std::filesystem::recursive_directory_iterator{ directory }) {
		std::filesystem::path currentPath = entry.path();

		if (entry.is_regular_file()) {
			parseResourceFile<T>(currentPath);
		}
	}
}
