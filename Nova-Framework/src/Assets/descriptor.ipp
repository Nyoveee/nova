#include <sstream>

#include "texture.h"
#include "audio.h"
#include "Logger.h"
#include "descriptor.h"

template <ValidAsset T>
std::optional<AssetInfo<T>> Descriptor::parseDescriptorFile(std::filesystem::path const& descriptorFilepath) {
	std::ifstream descriptorFile{ descriptorFilepath.string() };

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
		std::string line;
		std::getline(descriptorFile, line);

		bool toFlip;

		if (!(std::stringstream{ line } >> toFlip)) {
			return std::nullopt;
		}
		else {
			assetInfo.isFlipped = toFlip;
		}
	}

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

template <ValidAsset T>
AssetInfo<T> Descriptor::createDescriptorFile(std::filesystem::path const& path) {
	std::string metaDataFilename = getMetaDataFilename<T>(path);
	std::ofstream metaDataFile{ metaDataFilename };

	AssetInfo<T> assetInfo{ createDescriptorFile(path, metaDataFile) };

	// ============================
	// Filestream is now pointing at the 4th line.
	// Do any metadata specific to any type default creation here!!
	// ============================
	if constexpr (std::same_as<T, Texture>) {
		metaDataFile << false << "\n";
	}

	if constexpr (std::same_as<T, Audio>) {
		metaDataFile << false << "\n";
	}

	// ============================
	Logger::info("Successfully created metadata file for: {}", path.string());
	return assetInfo;
}

template<ValidAsset T>
std::string Descriptor::getMetaDataFilename(std::filesystem::path const& path) {
	auto iterator = subDescriptorDirectories.find(Family::id<T>());
	assert(iterator != subDescriptorDirectories.end() && "Sub asset directory not recorded.");

	auto&& [_, subDescriptorDirectory] = *iterator;
	return std::filesystem::path{ subDescriptorDirectory / path.stem() }.string() + ".desc";
}

template<ValidAsset T>
std::string Descriptor::getResourceFilename(std::filesystem::path const& descriptorPath) {
	auto iterator = subResourceDirectories.find(Family::id<T>());
	assert(iterator != subResourceDirectories.end() && "Sub asset directory not recorded.");

	auto&& [_, subResourceDirectory] = *iterator;
	return std::filesystem::path{ subResourceDirectory / descriptorPath.stem() }.string();
}