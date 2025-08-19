#include "assetManager.h"
template <typename T, typename ...Args> requires std::derived_from<T, Asset>
Asset& AssetManager::addAsset(AssetID id, std::string filepath, Args... args) {
	std::unique_ptr<T> newAsset = std::make_unique<T>(
		T{ filepath, args... }
	);

	auto [iterator, success] = assets.insert({ id, std::move(newAsset) });

	if (!success) {
		// asset id collision occur! this shouldn't happen though.
		std::cerr << "Asset ID collision occur for: " << filepath << "!\n";
	}

	auto&& [assetId, asset] = *iterator;

	return *asset.get();
}

// i love template programming
template<typename T> requires std::derived_from<T, Asset>
AssetManager::AssetQuery<T> AssetManager::getAsset(AssetID id) {
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

template<typename T> requires std::derived_from<T, Asset>
void AssetManager::recordAssetFile(std::filesystem::path const& path) {
	auto [assetId, name] = parseMetaDataFile(path);

	// Save asset entry in the parent folder.
	std::filesystem::path parentPath = path.parent_path();

	if (std::filesystem::is_directory(parentPath)) {
		auto iterator = folderNameToId.find(parentPath.string());

		if (iterator == std::end(folderNameToId)) {
			std::cerr << "Attempting to add asset to a non existing parent folder?";
		}
		else {
			auto&& [_, parentFolderId] = *iterator;
			directories[parentFolderId].assets.push_back(assetId);
		}
	}
	else {
		// it should be a directory but just in case
		std::cerr << "strange.\n";
	}

	addAsset<T>(assetId, path.string()).name = name;
}
