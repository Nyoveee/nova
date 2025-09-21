#include <sstream>

#include "texture.h"
#include "audio.h"
#include "Logger.h"

template <ValidResource T>
std::optional<AssetInfo<T>> AssetIO::parseDescriptorFile(DescriptorFilePath const& descriptorFilepath) {
	std::ifstream descriptorFile{ descriptorFilepath };

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

	if constexpr (std::same_as<T, Audio>) {
		std::string line;
		std::getline(descriptorFile, line);

		bool is3D;

		if (!(std::stringstream{ line } >> is3D)) {
			return std::nullopt;
		}
		else {
			assetInfo.is3D = is3D;
		}
	}

	// ============================
	return assetInfo;
}

template <ValidResource T>
AssetInfo<T> AssetIO::createDescriptorFile(AssetFilePath const& path) {
	ResourceID id = generateResourceID();
	DescriptorFilePath descriptorFileName = getDescriptorFilename<T>(id);
	std::ofstream metaDataFile{ descriptorFileName };

	AssetInfo<T> assetInfo{ createDescriptorFile(id, path, metaDataFile) };

	// ============================
	// Filestream is now pointing at the 4th line.
	// Do any metadata specific to any type default creation here!!
	// ============================

	if constexpr (std::same_as<T, Audio>) {
		metaDataFile << false << "\n";
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