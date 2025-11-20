template <ValidResource T>
void SerialiseDescriptorFunctor<T>::operator()(ResourceID id, AssetManager& assetManager) const {
	 assetManager.serializeDescriptor<T>(id);
}