#include <sstream>

#include "texture.h"
#include "audio.h"
#include "Logger.h"

#include "magic_enum.hpp"
#include "assetIO.h"

template <ValidResource T>
std::optional<AssetInfo<T>> AssetIO::parseDescriptorFile(DescriptorFilePath const& descriptorFilepath) {
	try {
		std::ifstream descriptorFile{ descriptorFilepath };
		descriptorFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		// Attempt to read corresponding metafile.
		if (!descriptorFile) {
			return std::nullopt;
		}

		// parse the generic asset metadata info first.
		std::optional<BasicAssetInfo> parsedAssetInfo = parseDescriptorFile(descriptorFile);

		// parsing failed, time to create a new metadata file.
		if (!parsedAssetInfo) {
			return std::nullopt;
		}

		AssetInfo<T> assetInfo{ parsedAssetInfo.value() };

		// ============================
		// Filestream is now pointing at the 4th line.
		// Do any metadata specific to any type parsing here!!
		// ============================
		if constexpr (std::same_as<T, Texture>) {
			std::string compressionFormat;
			std::getline(descriptorFile, compressionFormat);

			auto compressionValueOpt = magic_enum::enum_cast<AssetInfo<Texture>::Compression>(compressionFormat);

			if (!compressionValueOpt) {
				// parsing this failed, let's give some default compression value.
				assetInfo.compression = AssetInfo<Texture>::Compression::BC1_SRGB;
			}
			else {
				assetInfo.compression = compressionValueOpt.value();
			}
		}
		// ============================
		return assetInfo;
	}
	catch (std::exception const& ex) {
		Logger::error("Error parsing.. {}", ex.what());
		return std::nullopt;
	}
}

template <ValidResource T>
AssetInfo<T> AssetIO::createDescriptorFile(AssetFilePath const& path) {
	ResourceID id = generateResourceID();
	DescriptorFilePath descriptorFileName = getDescriptorFilename<T>(id);
	std::ofstream descriptorFile{ descriptorFileName };
	descriptorFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	AssetInfo<T> assetInfo{ createDescriptorFile(id, path, descriptorFile) };

	// ============================
	// Filestream is now pointing at the 4th line.
	// Do any metadata specific to any type default creation here!!
	// ============================
	if constexpr (std::same_as<T, Texture>) {
		descriptorFile << magic_enum::enum_name(AssetInfo<Texture>::Compression::BC1_SRGB) << '\n';
	}

	// ============================
	return assetInfo;
}

template<ValidResource T>
DescriptorFilePath AssetIO::getDescriptorFilename(ResourceID id) {
	auto iterator = subDescriptorDirectories.find(Family::id<T>());
	assert(iterator != subDescriptorDirectories.end() && "Sub asset directory not recorded.");

	auto&& [_, subDescriptorDirectory] = *iterator;
	return std::filesystem::path{ subDescriptorDirectory / std::filesystem::path{ std::to_string(static_cast<std::size_t>(id)) }.stem() }.string() + ".desc";
}

template<ValidResource T>
ResourceFilePath AssetIO::getResourceFilename(ResourceID id) {
	auto iterator = subResourceDirectories.find(Family::id<T>());
	assert(iterator != subResourceDirectories.end() && "Sub asset directory not recorded.");

	auto&& [_, subResourceDirectory] = *iterator;
	return std::filesystem::path{ subResourceDirectory / std::filesystem::path{ std::to_string(static_cast<std::size_t>(id)) }.stem() }.string();
}

template<ValidResource T>
AssetCacheFilePath AssetIO::getAssetCacheFilename(ResourceID id) {
	auto iterator = subAssetCacheDirectories.find(Family::id<T>());
	assert(iterator != subAssetCacheDirectories.end() && "Sub asset directory not recorded.");

	auto&& [_, subAssetCacheDirectory] = *iterator;
	return std::filesystem::path{ subAssetCacheDirectory / std::filesystem::path{ std::to_string(static_cast<std::size_t>(id)) }.stem() }.string();
}
