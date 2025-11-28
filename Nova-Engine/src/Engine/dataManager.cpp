#include <fstream>
#include "dataManager.h"

constexpr const char * playerPreferenceFileName = "PlayerPref.json";

template<typename T>
std::optional<T> loadData(Json const& playerPreferenceData, std::string const& name) {
	auto iterator = playerPreferenceData.find(name);

	if (iterator == playerPreferenceData.end()) {
		return std::nullopt;
	}

	try {
		T data = iterator->get<T>();
		return data;
	}
	catch (nlohmann::json::exception const&) {
		return std::nullopt;
	}
}

DataManager::DataManager(Engine& engine) : 
	engine	{ engine } 
{
	// The data manager is responsible for loading a player preferences file.
	std::ifstream playerPreferenceFile{ playerPreferenceFileName };
	
	if (!playerPreferenceFile) {
		return;
	}

	try {
		playerPreferenceFile >> playerPreferenceData;
	}
	catch (std::exception const& ex) {
		Logger::error("Failed to load player preference data. {}", ex.what());
	}
}

DataManager::~DataManager() {
	savePlayerPreference();
}

void DataManager::savePlayerPreference() {
	// @TODO: Use a dedicated location meant for storing player preference..
	std::ofstream playerPreferenceFile{ playerPreferenceFileName };
	playerPreferenceFile << std::setw(4) << playerPreferenceData << std::endl;
}

std::optional<int> DataManager::loadIntData(std::string const& name) {
	return loadData<int>(playerPreferenceData, name);
}

std::optional<float> DataManager::loadFloatData(std::string const& name) {
	return loadData<float>(playerPreferenceData, name);
}

std::optional<std::string> DataManager::loadStringData(std::string const& name) {
	return loadData<std::string>(playerPreferenceData, name);
}

void DataManager::removeKey(std::string const& name) {
	auto iterator = playerPreferenceData.find(name);

	if (iterator == playerPreferenceData.end()) {
		return;
	}

	playerPreferenceData.erase(iterator);
}

void DataManager::clear() {
	playerPreferenceData.clear();
}