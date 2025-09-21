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

			// We attempt to parse the descriptor file, to find out the original asset it is referencing...
			std::optional<AssetInfo<T>> opt = AssetIO::parseDescriptorFile<T>(descriptorFilepath);
			AssetInfo<T> descriptor;

			if (!opt) {
				Logger::info("Parsing failed, removing invalid descriptor file..\n");
				std::filesystem::remove(descriptorFilepath);
				continue;
			}
			// We check the validity of descriptor file, whether the asset path is pointing to a valid file.
			else if (!std::filesystem::is_regular_file(std::filesystem::path{ opt.value().filepath })) {
				Logger::info("Descriptor file is pointing to an invalid asset filepath, removing descriptor file..\n");
				std::filesystem::remove(descriptorFilepath);
				continue;
			}
			else {
				Logger::info("Sucessfully parsed descriptor file.");
				descriptor = opt.value();
			}

			// We first check if the last write time of the asset file matches the one recorded in descriptor.
			// If it doesn't, we need to recompile this asset file into it's resource form, and load it to the resource manager.
			auto assetLastWriteTime = std::filesystem::last_write_time(std::filesystem::path{ descriptor.filepath });
			auto assetEpoch = std::chrono::duration_cast<std::chrono::milliseconds>(assetLastWriteTime.time_since_epoch());

			// doesnt match, asset has changed..
			if (assetEpoch != descriptor.timeLastWrite) {
				Logger::info("Asset has changed, recompiling it's corresponding resource file..\n");
				resourceManager.removeResource(descriptor.id);
				createResourceFile<T>(descriptor);
			}
			// If it does, we check if the resource manager already have this particular resource loaded.
			// If it doesn't exist, it means that this resource file is missing / invalid.
			// Let's compile the corresponding intermediary asset file and load it to the resource manager.
			else if (!resourceManager.doesResourceExist(descriptor.id)) {
				Logger::info("Corresponding resource file does not exist, compiling intermediary asset {}\n", descriptor.filepath.string);
				createResourceFile<T>(descriptor);
			}
			else {
				Logger::info("A valid resource file exist for this descriptor and asset.\n");
				// we record all encountered intermediary assets with corresponding filepaths.
				intermediaryAssetsToDescriptor.insert({ descriptor.filepath, { descriptorFilepath, descriptor.id } });

				// we associate resource id with a name.
				assetToDescriptor.insert({ descriptor.id, std::make_unique<AssetInfo<T>>(descriptor) });

				// for each asset, we associate it with a descriptor functor.
				serialiseDescriptorFunctors.insert({ descriptor.id, std::make_unique<SerialiseDescriptorFunctor<T>>() });

				// map each resource id to the original type.
				resourceToType.insert({ descriptor.id, Family::id<T>() });
			}
		}

		Logger::info("===========================\n");
	}(), ...);
}

template<ValidResource T>
ResourceID AssetManager::createResourceFile(AssetInfo<T> descriptor) {
	// this compiles a resource file corresponding to the intermediary asset.
	compileIntermediaryFile<T>(descriptor);

	DescriptorFilePath descriptorFilePath = AssetIO::getDescriptorFilename<T>(descriptor.id);
	ResourceFilePath resourceFilePath = AssetIO::getResourceFilename<T>(descriptor.id);

	// we can now add this resource to the resource manager.
	ResourceID id = resourceManager.addResourceFile<T>(resourceFilePath);

	if (id != INVALID_RESOURCE_ID) {
		// we record all encountered intermediary assets with corresponding filepaths.
		intermediaryAssetsToDescriptor.insert({ descriptor.filepath, { descriptorFilePath, descriptor.id } });

		// we associate resource id with a name.
		assetToDescriptor.insert({ id, std::make_unique<AssetInfo<T>>(descriptor) });

		// for each asset, we associate it with a descriptor functor.
		serialiseDescriptorFunctors.insert({ id, std::make_unique<SerialiseDescriptorFunctor<T>>() });

		// map each resource id to the original type.
		resourceToType.insert({ id, Family::id<T>() });
	}

	return id;
}

template<ValidResource T>
void AssetManager::serialiseDescriptor(ResourceID id) {
	std::ofstream descriptorFile{ AssetIO::getDescriptorFilename<T>(id) };

	if (!descriptorFile) {
		Logger::error("Failure to serialise descriptor file of: {}", AssetIO::getDescriptorFilename<T>(id).string);
		return;
	}

	// retrieve asset info..
	std::unique_ptr<BasicAssetInfo> const& basicAssetInfoPtr = assetToDescriptor[id];
	assert(basicAssetInfoPtr && "Should never be nullptr");
	
	AssetInfo<T>* assetInfo = static_cast<AssetInfo<T>*>(basicAssetInfoPtr.get());
	assert(assetInfo && "Should also never be nullptr");

	// serialise basic asset info..
	// 1st line -> asset id, 2nd line -> name, 3rd line -> asset filepath, 4th epoch time / last write time.

	// calculate relative path to the Assets directory.
	std::filesystem::path relativePath = std::filesystem::relative(std::filesystem::path{ assetInfo->filepath }, AssetIO::assetDirectory);

	// get most recently write time..
	auto lastWriteTime = std::filesystem::last_write_time(std::filesystem::path{ assetInfo->filepath });
	auto epoch = lastWriteTime.time_since_epoch();
	auto value = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);

	descriptorFile << static_cast<std::size_t>(assetInfo->id) << '\n' << assetInfo->name << '\n' << relativePath.string() << '\n' << value.count() << '\n';

	// ============================
	// Filestream is now pointing at the 5th line.
	// Do any serialisation for specifc metadata of asset type here!!
	// ============================

	// ============================
	Logger::info("Successfully serialised descriptor file for {}", assetInfo->filepath.string);
}