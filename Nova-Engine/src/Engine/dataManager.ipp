template<typename T>
void DataManager::saveData(std::string const& name, T const& data) {
	playerPreferenceData[name] = serializeToJson(data);
}