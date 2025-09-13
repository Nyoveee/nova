// dont worry about the intellisense error, VS2022 doesnt know when it is getting included.

template <ValidAsset T>
void SerialiseMetaDataFunctor<T>::operator()(Asset& asset, AssetManager& assetManager) const {
	 T& specificAsset = static_cast<T&>(asset);
	 assetManager.serialiseAssetMetaData(specificAsset);
}

template <ValidAsset T>
void SerialiseAssetFunctor<T>::operator()(Asset& asset, AssetManager& assetManager) const {
	//T& specificAsset = static_cast<T&>(asset);
	//assetManager.serialiseAsset(specificAsset);
}