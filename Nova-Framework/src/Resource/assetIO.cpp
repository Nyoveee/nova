#include <filesystem>
#include <fstream>

#include "assetIO.h"
#include "resource.h"
#include "nova_math.h"

#include "Material.h"
#include "customShader.h"

#define DescriptorSubDirectory(AssetType) \
	std::pair{ Family::id<AssetType>(), std::filesystem::current_path() / "Descriptors" / #AssetType }

#define ResourceSubDirectory(AssetType) \
	std::pair{ Family::id<AssetType>(), std::filesystem::current_path() / "Resources" / #AssetType }

#define AssetCacheSubDirectory(AssetType) \
	std::pair{ Family::id<AssetType>(), std::filesystem::current_path() / ".asset_cache" / #AssetType }

#if 0
#define SystemResourceSubDirectory(AssetType) \
	std::pair{ Family::id<AssetType>(), std::filesystem::current_path() / "System" / #AssetType }
#endif

std::unordered_map<ResourceTypeID, std::filesystem::path> const AssetIO::subDescriptorDirectories {
	DescriptorSubDirectory(Texture),
	DescriptorSubDirectory(Model),
	DescriptorSubDirectory(CubeMap),
	DescriptorSubDirectory(ScriptAsset),
	DescriptorSubDirectory(Audio),
	DescriptorSubDirectory(Scene),
	DescriptorSubDirectory(NavMesh),
	DescriptorSubDirectory(Font),
	DescriptorSubDirectory(Controller),
	DescriptorSubDirectory(CustomShader),
	DescriptorSubDirectory(Material),
	DescriptorSubDirectory(Prefab),
	DescriptorSubDirectory(Sequencer)
};

std::unordered_map<ResourceTypeID, std::filesystem::path> const AssetIO::subResourceDirectories{
	ResourceSubDirectory(Texture),
	ResourceSubDirectory(Model),
	ResourceSubDirectory(CubeMap),
	ResourceSubDirectory(ScriptAsset),
	ResourceSubDirectory(Audio),
	ResourceSubDirectory(Scene),
	ResourceSubDirectory(NavMesh),
	ResourceSubDirectory(Font),
	ResourceSubDirectory(Controller),
	ResourceSubDirectory(CustomShader),
	ResourceSubDirectory(Material),
	ResourceSubDirectory(Prefab),
	ResourceSubDirectory(Sequencer)

};

std::unordered_map<ResourceTypeID, std::filesystem::path> const AssetIO::subAssetCacheDirectories{
	AssetCacheSubDirectory(Texture),
	AssetCacheSubDirectory(Model),
	AssetCacheSubDirectory(CubeMap),
	AssetCacheSubDirectory(ScriptAsset),
	AssetCacheSubDirectory(Audio),
	AssetCacheSubDirectory(Scene),
	AssetCacheSubDirectory(NavMesh),
	AssetCacheSubDirectory(Font),
	AssetCacheSubDirectory(Controller),
	AssetCacheSubDirectory(CustomShader),
	AssetCacheSubDirectory(Material),
	AssetCacheSubDirectory(Prefab),
	AssetCacheSubDirectory(Sequencer)
};

std::filesystem::path const AssetIO::assetDirectory				= std::filesystem::current_path() / "Assets";
std::filesystem::path const AssetIO::resourceDirectory			= std::filesystem::current_path() / "Resources";
std::filesystem::path const AssetIO::descriptorDirectory		= std::filesystem::current_path() / "Descriptors";
std::filesystem::path const AssetIO::assetCacheDirectory		= std::filesystem::current_path() / ".asset_cache";
std::filesystem::path const AssetIO::systemResourceDirectory	= std::filesystem::current_path() / "System";

std::optional<BasicAssetInfo> AssetIO::parseDescriptorFile(std::ifstream& descriptorFile, std::filesystem::path const& rootDirectory) {
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
		std::string fullFilepath = std::filesystem::path{ rootDirectory / relativeFilepath }.string();

		return { { resourceId, std::move(name), std::move(fullFilepath) }};
	}
	catch (std::exception const& ex) {
		Logger::error("Failed to parse descriptor file, reason: {}", ex.what());
		return std::nullopt;
	}
}

BasicAssetInfo AssetIO::createDescriptorFile(ResourceID id, std::filesystem::path const& path, std::ostream& descriptorFile, std::filesystem::path const& rootDirectory) {
	BasicAssetInfo assetInfo = { id, std::filesystem::path{ path }.stem().string(), path };

	try {
		// calculate relative path to the Assets directory.
		std::filesystem::path relativePath = std::filesystem::relative(std::filesystem::path{ path }, rootDirectory);

		if (!descriptorFile) {
			Logger::error("Error creating descriptor file for {}", path.string());
			return assetInfo;
		}

		// write to file
		descriptorFile << static_cast<std::size_t>(assetInfo.id) << '\n' << assetInfo.name << '\n' << relativePath.string() << '\n';
		return assetInfo;
	}
	catch (std::exception const& ex) {
		Logger::error("Failed to parse descriptor file, reason: {}", ex.what());
		return assetInfo;
	}
}

ResourceID AssetIO::generateResourceID() {
	return Math::getGUID();
}