#include <sstream>
#include <cstdlib>

#include "assetManager.h"
#include "Profiling.h"
#include "ResourceManager/resourceManager.h"
#include "assetIO.h"

#if 0
template <ValidResource T>
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

template <ValidResource T>
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

template<ValidResource T>
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

template<ValidResource T>
AssetID AssetManager::getSomeAssetID() const {
	auto iterator = assetsByType.find(Family::id<T>());

	if (iterator == assetsByType.end()) {
		Logger::error("Attempt to get an asset id of an invalid type?");
		return INVALID_RESOURCE_ID;
	}

	auto&& [_, allAssets] = *iterator;
	
	if (allAssets.empty()) {
		Logger::error("This asset type has no asset?");
		return INVALID_RESOURCE_ID;
	}

	return allAssets[0];
}

template<ValidResource T>
bool AssetManager::isAsset(AssetID id) const {
	auto iterator = assetIdToType.find(id);

	assert(iterator != assetIdToType.end() && "asset id doesn't exist / have no associated asset type id?");

	auto [_, assetTypeId] = *iterator;
	return assetTypeId == Family::id<T>();
}

template <ValidResource T>
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



template<ValidResource T>
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

template<ValidResource T>
void AssetManager::compileIntermediaryFile(AssetInfo<T> descriptor) {
#if _DEBUG
		const char* executableConfiguration = "Debug";
#else
		const char* executableConfiguration = "Release";
#endif
		constexpr const char* executableName = "Nova-ResourceCompiler.exe";

		std::filesystem::path compilerPath = std::filesystem::current_path() / "ExternalApplication" / executableConfiguration / executableName;

		DescriptorFilePath descriptorFilePath = AssetIO::getDescriptorFilename<T>(descriptor.id);

		// https://stackoverflow.com/questions/27975969/how-to-run-an-executable-with-spaces-using-stdsystem-on-windows/27976653#27976653
		std::string command = "\"\"" + compilerPath.string() + "\" \"" + descriptorFilePath.string + "\"\"";

		Logger::info("Running command: {}", command);

		if (std::system(command.c_str())) {
			Logger::error("Error compiling {}", descriptorFilePath.string);
		}
		else {
			Logger::info("Successful compiling {}", descriptorFilePath.string);
			Logger::info("Resource file created: {}", AssetIO::getResourceFilename<T>(descriptor.id).string);
		}
}

template<ValidResource ...T>
void AssetManager::loadAllDescriptorFiles() {
	([&] {
		auto iterator = AssetIO::subDescriptorDirectories.find(Family::id<T>());
		assert(iterator != AssetIO::subDescriptorDirectories.end() && "This descriptor sub directory is not recorded.");
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
			std::optional<AssetInfo<T>> opt = AssetIO::parseDescriptorFile<T>(descriptorFilepath);
			AssetInfo<T> descriptor;

			if (!opt) {
				Logger::info("Failed parsing, recreating a new descriptor file..");
				descriptor = AssetIO::createDescriptorFile<T>(descriptorFilepath);
			}
			else {
				Logger::info("Sucessfully parsed descriptor file.");
				descriptor = opt.value();
			}

			// check if resource manager has this particular resources already loaded.
			// if resource doesn't exist, let's compile the corresponding intermediary asset file and load it to		
			// the resource manager.

			if (!resourceManager.doesResourceExist(descriptor.id)) {
				Logger::info("Corresponding resource file does not exist, compiling intermediary asset {}", descriptor.filepath.string);
				createResourceFile<T>(descriptor);
			}
			else {
				Logger::info("Resource file exist.");
			}

			Logger::info("");

			// we record all encountered intermediary assets with corresponding filepaths.
			intermediaryAssetsToDescriptor.insert({ descriptor.filepath, { descriptorFilepath, descriptor.id } });

			// we associate resource id with a name.
			assetToDescriptor.insert({ descriptor.id, descriptor });
		}

		Logger::info("===========================\n");
	}(), ...);
}

template<ValidResource T>
void AssetManager::createResourceFile(AssetInfo<T> descriptor) {
	// this compiles a resource file corresponding to the intermediary asset.
	compileIntermediaryFile<T>(descriptor);

	DescriptorFilePath descriptorFilePath = AssetIO::getDescriptorFilename<T>(descriptor.id);
	ResourceFilePath resourceFilePath = AssetIO::getResourceFilename<T>(descriptor.id);

	// we can now add this resource to the resource manager.
	ResourceID id = resourceManager.addResourceFile<T>(resourceFilePath);

	if (id != INVALID_RESOURCE_ID) {
		// we associate resource id with a name.
		assetToDescriptor.insert({ id, descriptor });
	}
}
