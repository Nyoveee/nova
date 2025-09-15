#include <sstream>
#include <cstdlib>

#include "assetManager.h"
#include "Profiling.h"
#include "ResourceManager/resourceManager.h"
#include "descriptor.h"

#if 0
template <ValidAsset T>
Asset& AssetManager::addAsset(AssetInfo<T> const& assetInfo) {
	std::unique_ptr<T> newAsset = std::make_unique<T>(
		createAsset<T>(assetInfo)
	);
	auto [iterator, success] = assets.insert({ assetInfo.id, std::move(newAsset) });

	if (!success) {
		// asset id collision occur! this shouldn't happen though.
		Logger::error("Asset ID collision occured for: {}", assetInfo.filepath);
	}

	auto&& [assetId, asset] = *iterator;

	// associates a specific typed instance of serialise functor to a given id.
	// this will be used for serialisation. this is how we obtain the original type associated with the asset id.
	serialiseMetaDataFunctors[assetInfo.id] = std::make_unique<SerialiseMetaDataFunctor<T>>(SerialiseMetaDataFunctor<T>{});

	// record this asset to the corresponding asset type.
	assetsByType[Family::id<T>()].push_back(assetInfo.id);

	// associate this asset id with this asset type.
	assetIdToType[assetInfo.id] = Family::id<T>();

	// associate absolute filepath to this asset id.
	filepathToAssetId[assetInfo.filepath] = assetInfo.id;

	return *asset.get();
}

template <ValidAsset T>
AssetManager::AssetQuery<T> AssetManager::getAsset(AssetID id) {
	// i love template programming
	
	auto iterator = assets.find(id);

	if (iterator == assets.end()) {
		return AssetQuery<T>{ nullptr, QueryResult::Invalid };
	}
	
	auto&& [_, asset] = *iterator;

	switch (asset->getLoadStatus())
	{
	case Asset::LoadStatus::NotLoaded:
		asset->toLoad();
		break;
	case Asset::LoadStatus::Loaded:
		break;
	case Asset::LoadStatus::Loading:
		return AssetQuery<T>{ nullptr, QueryResult::Loading };
	case Asset::LoadStatus::LoadingFailed:
		Logger::error("Loading operator for asset id of {} has failed. Retrying..", static_cast<std::size_t>(id));
		asset->toLoad();
		break;
	}

	if (asset->isLoaded()) {
		T* typedAsset = dynamic_cast<T*>(asset.get());
		return AssetQuery<T>{ typedAsset, typedAsset ? QueryResult::Success : QueryResult::WrongType };
	}

	if (asset->getLoadStatus() == Asset::LoadStatus::LoadingFailed) {
		return AssetQuery<T>{ nullptr, QueryResult::LoadingFailed };
	}
	else {
		return AssetQuery<T>{ nullptr, QueryResult::Loading };
	}
}

template<ValidAsset T>
std::vector<AssetID> const& AssetManager::getAllAssets() const {
	auto iterator = assetsByType.find(Family::id<T>());

	if (iterator == assetsByType.end()) {
		Logger::error("Attempt to retrieve all assets of an invalid type?");
		static std::vector<AssetID> empty;
		return empty;
	}

	auto&& [_, allAssets] = *iterator;
	return allAssets;
}

template<ValidAsset T>
AssetID AssetManager::getSomeAssetID() const {
	auto iterator = assetsByType.find(Family::id<T>());

	if (iterator == assetsByType.end()) {
		Logger::error("Attempt to get an asset id of an invalid type?");
		return INVALID_ASSET_ID;
	}

	auto&& [_, allAssets] = *iterator;
	
	if (allAssets.empty()) {
		Logger::error("This asset type has no asset?");
		return INVALID_ASSET_ID;
	}

	return allAssets[0];
}

template<ValidAsset T>
bool AssetManager::isAsset(AssetID id) const {
	auto iterator = assetIdToType.find(id);

	assert(iterator != assetIdToType.end() && "asset id doesn't exist / have no associated asset type id?");

	auto [_, assetTypeId] = *iterator;
	return assetTypeId == Family::id<T>();
}

template <ValidAsset T>
void AssetManager::recordAssetFile(std::filesystem::path const& path) {
	AssetInfo<T> assetInfo = parseMetaDataFile<T>(path);

	// Save asset entry in the parent folder.
	std::filesystem::path parentPath = path.parent_path();

	auto iterator = folderNameToId.find(parentPath.string());

	if (iterator == std::end(folderNameToId)) {
		Logger::error("Attempting to add asset to a non existing parent folder?");
	}
	else {
		auto&& [_, parentFolderId] = *iterator;
		directories[parentFolderId].assets.push_back(assetInfo.id);
	}
	
	// Record the asset into the asset manager's database.
	Asset& asset = addAsset<T>(assetInfo);
	asset.name = assetInfo.name;
	asset.id = assetInfo.id;
}



template<ValidAsset T>
void AssetManager::serialiseAssetMetaData(T const& asset) {
	std::string metaDataFilename = asset.getFilePath() + ".nova_meta";
	std::ofstream metaDataFile{ metaDataFilename };
	
	if (!metaDataFile) {
		Logger::error("Failure to serialise asset metadata file of: {}", metaDataFilename);
		return;
	}

	serialiseAssetMetaData(asset, metaDataFile);

	// ============================
	// Filestream is now pointing at the 3rd line.
	// Do any serialisation for specifc metadata of asset type here!!
	// ============================

	if constexpr (std::same_as<T, Texture>) {
		metaDataFile << asset.isFlipped() << "\n";
	}

	if constexpr (std::same_as<T, Audio>) {
		metaDataFile << asset.isAudio3D() << "\n";
	}

	// ============================
	Logger::info("Successfully serialised metadata file for {}", asset.getFilePath());
}
#endif

template<ValidAsset T>
void AssetManager::compileIntermediaryFile(AssetFilePath const& path) {
#if _DEBUG
	const char* executableConfiguration = "Debug";
#else
	const char* executableConfiguration = "Release";
#endif
	constexpr const char* executableName = "Nova-ResourceCompiler.exe";

	std::filesystem::path compilerPath = std::filesystem::current_path() / "ExternalApplication" / executableConfiguration / executableName;
	
	DescriptorFilePath descriptorFilePath = Descriptor::getDescriptorFilename<T>(path);
	std::string command = compilerPath.string() + " \"" + descriptorFilePath.string + "\"";

	if (std::system(command.c_str())) {
		Logger::error("Error compiling {}", descriptorFilePath.string);
	}
	else {
		Logger::info("Successful compiling {}", descriptorFilePath.string);
		Logger::info("Resource file created: {}", Descriptor::getResourceFilename<T>(descriptorFilePath).string);
	}
}

template<ValidAsset T>
void AssetManager::loadAllDescriptorFiles() {
	auto iterator = Descriptor::subDescriptorDirectories.find(Family::id<T>());
	assert(iterator != Descriptor::subDescriptorDirectories.end() && "This descriptor sub directory is not recorded.");
	std::filesystem::path const& directory = iterator->second;

	Logger::info("===========================");
	Logger::info("Loading all {} descriptors.", directory.stem().string());
	Logger::info("----");

	// recursively iterate through a directory and parse all descriptor files.
	for (const auto& entry : std::filesystem::recursive_directory_iterator{ directory }) {
		std::filesystem::path descriptorFilepath = entry.path();

		if (!entry.is_regular_file()) {
			continue;
		}

		if (descriptorFilepath.extension() != ".desc") {
			continue;
		}

		Logger::info("Parsing {}..", descriptorFilepath.string());

		// parse the descriptor file.
		std::optional<AssetInfo<T>> opt = Descriptor::parseDescriptorFile<T>(descriptorFilepath);
		AssetInfo<T> descriptor;

		if (!opt) {
			Logger::info("Failed parsing, recreating a new descriptor file..");
			descriptor = Descriptor::createDescriptorFile<T>(descriptorFilepath);
		}
		else {
			Logger::info("Sucessfully parsed descriptor file.");
			descriptor = opt.value();
		}

		// check if resource manager has this particular resources already loaded.
		// if resource doesn't exist, let's compile the corresponding intermediary asset file and load it to		
		// the resource manager.

		Asset* resource = resourceManager.getResourceInfo(descriptor.id);
		if (!resource) {
			Logger::info("Corresponding resource file does not exist, compiling intermediary asset {}", descriptor.filepath.string);
			createResourceFile<T>(descriptor);
		}
		else {
			Logger::info("Resource file exist.");
			resource->name = descriptor.name;
		}

		Logger::info("");

		// we record all encountered intermediary assets with corresponding filepaths.
		intermediaryAssetsToFilepaths.insert({ descriptor.filepath, { descriptorFilepath, Descriptor::getResourceFilename<T>(descriptorFilepath) } });
	}

	Logger::info("===========================\n");
}

template<ValidAsset T>
void AssetManager::createResourceFile(AssetInfo<T> descriptor) {
	// this compiles a resource file corresponding to the intermediary asset.
	compileIntermediaryFile<T>(descriptor.filepath);

	DescriptorFilePath descriptorFilePath = Descriptor::getDescriptorFilename<T>(descriptor.filepath);
	ResourceFilePath resourceFilePath = Descriptor::getResourceFilename<T>(descriptorFilePath);

	// we can now add this resource to the resource manager.
	T* resource = resourceManager.addResourceFile<T>(resourceFilePath);

	if (resource) {
		resource->name = descriptor.name;
	}
}
