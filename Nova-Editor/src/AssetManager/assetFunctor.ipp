template <ValidResource T>
void SerialiseDescriptorFunctor<T>::operator()(ResourceID id, AssetManager& assetManager) const {
	 assetManager.serialiseDescriptor<T>(id);
}