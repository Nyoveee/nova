#include "assetManager.h"
#include <sstream>

template <ValidAsset T>
Asset& AssetManager::addAsset(AssetInfo<T> const& assetInfo) {
	std::unique_ptr<T> newAsset = std::make_unique<T>(
		createAsset<T>(assetInfo)
	);

	auto [iterator, success] = assets.insert({ assetInfo.id, std::move(newAsset) });

	if (!success) {
		// asset id collision occur! this shouldn't happen though.
		std::cerr << "Asset ID collision occur for: " << assetInfo.filepath << "!\n";
	}

	auto&& [assetId, asset] = *iterator;

	// associates a specific typed instance of serialise functor to a given id.
	// this will be used for serialisation. this is how we obtain the original type associated with the asset id.
	serialiseAssetFunctors[assetInfo.id] = std::make_unique<SerialiseAssetFunctor<T>>(SerialiseAssetFunctor<T>{});

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

	if (asset->isLoaded()) {
		T* typedAsset = dynamic_cast<T*>(asset.get());
		return AssetQuery<T>{typedAsset, typedAsset ? QueryResult::Success : QueryResult::WrongType };
	}
	else {
		asset->load();

		// Is the load operation asynchronous? Does it run on seperate thread?
		if (asset->isLoaded()) {
			T* typedAsset = dynamic_cast<T*>(asset.get());
			return AssetQuery<T>{typedAsset, typedAsset ? QueryResult::Success : QueryResult::WrongType };
		}
		else {
			return AssetQuery<T>{ nullptr, QueryResult::Loading };
		}
	}
}

template <ValidAsset T>
void AssetManager::recordAssetFile(std::filesystem::path const& path) {
	AssetInfo<T> assetInfo = parseMetaDataFile<T>(path);

	// Save asset entry in the parent folder.
	std::filesystem::path parentPath = path.parent_path();

	if (std::filesystem::is_directory(parentPath)) {
		auto iterator = folderNameToId.find(parentPath.string());

		if (iterator == std::end(folderNameToId)) {
			std::cerr << "Attempting to add asset to a non existing parent folder?\n";
		}
		else {
			auto&& [_, parentFolderId] = *iterator;
			directories[parentFolderId].assets.push_back(assetInfo.id);
		}
	}
	else {
		// it should be a directory but just in case
		std::cerr << "strange.\n";
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
			std::cerr << "Failure when trying to parse toFlip boolean for file: " << path << "\n";
			return createMetaDataFile<T>(path);
		}
		else {
			assetInfo.isFlipped = toFlip;
		}
	}

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
		// default value of false.
		metaDataFile << false << "\n";
	}

	std::cout << "Successfully created metadata file for " << path << '\n';
	return assetInfo;
}

template<ValidAsset T>
void AssetManager::serialiseAssetMetaData(T const& asset) {
	std::string metaDataFilename = asset.getFilePath() + ".nova_meta";
	std::ofstream metaDataFile{ metaDataFilename };
	
	if (!metaDataFile) {
		std::cerr << "Failure to serialise asset metadata file of: " << metaDataFilename << "\n";
	}

	serialiseAssetMetaData(asset, metaDataFile);

	// ============================
	// Filestream is now pointing at the 3rd line.
	// Do any serialisation for specifc metadata of asset type here!!
	// ============================

	if constexpr (std::same_as<T, Texture>) {
		metaDataFile << asset.isFlipped() << "\n";
	}

	std::cout << "Successfully serialised metadata file for " << asset.getFilePath() << '\n';
}
