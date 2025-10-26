#include "assetIO.h"

#include "xresource_guid.h"

#include <filesystem>
#include <fstream>

#include "resource.h"

#define DescriptorSubDirectory(AssetType) \
	std::pair{ Family::id<AssetType>(), std::filesystem::current_path() / "Descriptors" / #AssetType }

#define ResourceSubDirectory(AssetType) \
	std::pair{ Family::id<AssetType>(), std::filesystem::current_path() / "Resources" / #AssetType }

std::unordered_map<ResourceTypeID, std::filesystem::path> const AssetIO::subDescriptorDirectories {
	DescriptorSubDirectory(Texture),
	DescriptorSubDirectory(Model),
	DescriptorSubDirectory(CubeMap),
	DescriptorSubDirectory(ScriptAsset),
	DescriptorSubDirectory(Audio),
	DescriptorSubDirectory(Scene),
	DescriptorSubDirectory(NavMesh),
	DescriptorSubDirectory(Prefab)
};

std::unordered_map<ResourceTypeID, std::filesystem::path> const AssetIO::subResourceDirectories{
	ResourceSubDirectory(Texture),
	ResourceSubDirectory(Model),
	ResourceSubDirectory(CubeMap),
	ResourceSubDirectory(ScriptAsset),
	ResourceSubDirectory(Audio),
	ResourceSubDirectory(Scene),
	ResourceSubDirectory(NavMesh),
	ResourceSubDirectory(Prefab)

};

std::filesystem::path const AssetIO::assetDirectory = std::filesystem::current_path() / "Assets";
std::filesystem::path const AssetIO::resourceDirectory = std::filesystem::current_path() / "Resources";
std::filesystem::path const AssetIO::descriptorDirectory = std::filesystem::current_path() / "Descriptors";

std::optional<BasicAssetInfo> AssetIO::parseDescriptorFile(std::ifstream& descriptorFile) {
	try {
		std::string line;
		ResourceID resourceId;

		// reads the 1st line.
		std::getline(descriptorFile, line);
		resourceId = std::stoull(line);

		// reads the 2nd line, name.
		std::string name;
		std::getline(descriptorFile, name);

		// reads the 3rd line, relative filepath.
		std::string relativeFilepath;
		std::getline(descriptorFile, relativeFilepath);
		std::string fullFilepath = std::filesystem::path{ assetDirectory / relativeFilepath }.string();

		// reads the 4th line, last write time..
		long long duration;
		descriptorFile >> duration;
		
		// convert to filesystem last write..
		std::chrono::milliseconds value{ duration };

		return { { resourceId, std::move(name), std::move(fullFilepath), std::move(value) }};
	}
	catch (std::exception const& ex) {
		Logger::error("Failed to parse descriptor file, reason: {}", ex.what());
		return std::nullopt;
	}
}

BasicAssetInfo AssetIO::createDescriptorFile(ResourceID id, AssetFilePath const& path, std::ofstream& metaDataFile) {
	BasicAssetInfo assetInfo = { id, std::filesystem::path{ path }.stem().string(), path };

	try {
		// calculate relative path to the Assets directory.
		std::filesystem::path relativePath = std::filesystem::relative(std::filesystem::path{ path }, assetDirectory);

		if (!metaDataFile) {
			Logger::error("Error creating metadata file for {}", path.string);
			return assetInfo;
		}

		// cast from time point to a primitive type.
		// https://stackoverflow.com/questions/31255486/how-do-i-convert-a-stdchronotime-point-to-long-and-back
		auto lastWriteTime = std::filesystem::last_write_time(std::filesystem::path{ path });
		auto epoch = lastWriteTime.time_since_epoch();
		auto value = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);

		// write to file
		metaDataFile << static_cast<std::size_t>(assetInfo.id) << '\n' << assetInfo.name << '\n' << relativePath.string() << '\n' << value.count() << '\n';

		assetInfo.timeLastWrite = value;
		return assetInfo;
	}
	catch (std::exception const& ex) {
		Logger::error("Failed to parse descriptor file, reason: {}", ex.what());
		return assetInfo;
	}
}

ResourceID AssetIO::generateResourceID() {
	xresource::instance_guid guid = xresource::instance_guid::GenerateGUIDCopy();
	return guid.m_Value;
}