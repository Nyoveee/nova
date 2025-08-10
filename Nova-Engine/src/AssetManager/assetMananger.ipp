#include "assetManager.h"
template <typename T, typename ...Args> requires std::derived_from<T, Asset>
void AssetManager::addAsset(std::string filepath, Args... args) {
	std::unique_ptr<T> newAsset = std::make_unique<T>(
		T{ filepath, args... }
	);

	AssetID id{ assets.size() };
	assets[id] = std::move(newAsset);
}

template<typename T> requires std::derived_from<T, Asset>
AssetManager::AssetQuery<T> AssetManager::getAsset(AssetID id) {
	auto iterator = assets.find(id);

	if (iterator == assets.end()) {
		return AssetQuery<T>{ nullptr, AssetQuery<T>::Invalid };
	}

	auto&& [_, asset] = *iterator;

	if (asset->isLoaded()) {
		T* typedAsset = dynamic_cast<T*>(asset.get());
		return AssetQuery<T>{typedAsset, typedAsset ? AssetQuery<T>::Success : AssetQuery<T>::WrongType };
	}
	else {
		asset->load();

		// Is the load operation asynchronous? Does it run on seperate thread?
		if (asset->isLoaded()) {
			T* typedAsset = dynamic_cast<T*>(asset.get());
			return AssetQuery<T>{typedAsset, typedAsset ? AssetQuery<T>::Success : AssetQuery<T>::WrongType };
		}
		else {
			return AssetQuery<T>{ nullptr, AssetQuery<T>::Loading };
		}
	}
}
