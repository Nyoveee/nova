#include "descriptor.h"
#include <filesystem>
#include <fstream>

#include "texture.h"
#include "model.h"
#include "cubemap.h"
#include "scriptAsset.h"
#include "audio.h"

#define DescriptorSubDirectory(AssetType) \
	std::pair{ Family::id<AssetType>(), std::filesystem::current_path() / "Descriptors" / #AssetType }

#define ResourceSubDirectory(AssetType) \
	std::pair{ Family::id<AssetType>(), std::filesystem::current_path() / "Resources" / #AssetType }

std::unordered_map<ResourceTypeID, std::filesystem::path> const Descriptor::subDescriptorDirectories {
	DescriptorSubDirectory(Texture),
	DescriptorSubDirectory(Model),
	DescriptorSubDirectory(CubeMap),
	DescriptorSubDirectory(ScriptAsset),
	DescriptorSubDirectory(Audio)
};

std::unordered_map<ResourceTypeID, std::filesystem::path> const Descriptor::subResourceDirectories{
	ResourceSubDirectory(Texture),
	ResourceSubDirectory(Model),
	ResourceSubDirectory(CubeMap),
	ResourceSubDirectory(ScriptAsset),
	ResourceSubDirectory(Audio)
};

std::filesystem::path const Descriptor::assetDirectory = std::filesystem::current_path() / "Assets";
std::filesystem::path const Descriptor::resourceDirectory = std::filesystem::current_path() / "Resources";
std::filesystem::path const Descriptor::descriptorDirectory = std::filesystem::current_path() / "Descriptors";

std::optional<BasicAssetInfo> Descriptor::parseDescriptorFile(std::ifstream& descriptorFile) {
	std::string line;
	ResourceID resourceId;

	// reads the 1st line.
	std::getline(descriptorFile, line);

	try {
		resourceId = std::stoull(line);
	}
	catch (std::exception const&) {
		return std::nullopt;
	}

	// reads the 2nd line, name.
	std::string name;
	std::getline(descriptorFile, name);

	// reads the 3rd line, relative filepath.
	std::string relativeFilepath;
	std::getline(descriptorFile, relativeFilepath);
	std::string fullFilepath = std::filesystem::path{ assetDirectory / relativeFilepath }.string();

	return { { resourceId, std::move(name), std::move(fullFilepath) } };
}

BasicAssetInfo Descriptor::createDescriptorFile(std::filesystem::path const& path, std::ofstream& metaDataFile) {
	// calculate relative path to the Assets directory.
	std::filesystem::path relativePath = std::filesystem::relative(path, assetDirectory);

	BasicAssetInfo assetInfo = { generateResourceID(path), path.filename().string(), path.string() };

	if (!metaDataFile) {
		Logger::error("Error creating metadata file for {}", path.string());
		return assetInfo;
	}

	// write to file
	metaDataFile << static_cast<std::size_t>(assetInfo.id) << '\n' << assetInfo.name << '\n' << relativePath.string() << '\n';

	return assetInfo;
}

ResourceID Descriptor::generateResourceID(std::filesystem::path const& path) {
	// We don't need complex id generation code.
	// We just need to make sure our id is unique everytime a metadata file is generated in the context of our system
	// The easiest way is to utilise the current time + the filepath.

	// average c++ attempting to get time line of code..
	std::size_t time = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	std::string combined = path.string() + "_" + std::to_string(time);
	ResourceID id{ std::hash<std::string>{}(combined) };

	return id;
}