#include <sstream>

#include "assetManager.h"
#include "Debugging/Profiling.h"

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
	serialiseAssetFunctors[assetInfo.id] = std::make_unique<SerialiseAssetFunctor<T>>(SerialiseAssetFunctor<T>{});

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
		asset->toLoad(*this);
		break;
	case Asset::LoadStatus::Loaded:
		break;
	case Asset::LoadStatus::Loading:
		return AssetQuery<T>{ nullptr, QueryResult::Loading };
	case Asset::LoadStatus::LoadingFailed:
		Logger::error("Loading operator for asset id of {} has failed. Retrying..", static_cast<std::size_t>(id));
		asset->toLoad(*this);
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

template <ValidAsset T>
AssetInfo<T> AssetManager::parseMetaDataFile(std::filesystem::path const& path) {
	std::string metaDataFilename = path.string() + ".nova_meta";
	std::ifstream metaDataFile{ metaDataFilename };

	// parse the generic asset metadata info first.
	std::optional<BasicAssetInfo> parsedAssetInfo = parseMetaDataFile(path, metaDataFile);

	// parsing failed, time to create a new metadata file.
	if (!parsedAssetInfo) {
		return createMetaDataFile<T>(path);
	}

	AssetInfo<T> assetInfo{ parsedAssetInfo.value() };

	// ============================
	// Filestream is now pointing at the 3rd line.
	// Do any metadata specific to any type parsing here!!
	// ============================
	if constexpr (std::same_as<T, Texture>) {
		std::string line;
		std::getline(metaDataFile, line);

		bool toFlip;

		if (!(std::stringstream{ line } >> toFlip)) {
			Logger::error("Failure when trying to parse toFlip boolean for file: {}", path.string());
			return createMetaDataFile<T>(path);
		}
		else {
			assetInfo.isFlipped = toFlip;
		}
	}

	if constexpr (std::same_as<T, Audio>) {
		std::string line;
		std::getline(metaDataFile, line);

		bool is3D;

		if (!(std::stringstream{ line } >> is3D)) {
			Logger::error("Failure when trying to parse is3D boolean for file: {}", path.string());
			return createMetaDataFile<T>(path);
		}
		else {
			assetInfo.is3D = is3D;
		}
	}

	// ============================
	return assetInfo;
}

template <ValidAsset T>
AssetInfo<T> AssetManager::createMetaDataFile(std::filesystem::path const& path) {
	std::string metaDataFilename = path.string() + ".nova_meta";
	std::ofstream metaDataFile{ metaDataFilename };

	AssetInfo<T> assetInfo{ createMetaDataFile(path, metaDataFile) };

	// ============================
	// Filestream is now pointing at the 3rd line.
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
