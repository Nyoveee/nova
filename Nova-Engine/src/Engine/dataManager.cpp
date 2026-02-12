#include <fstream>
#include "dataManager.h"

#include <windows.h>
#include <shlobj.h>

constexpr const char * playerPreferenceFileName = "PlayerPref.json";
constexpr const char * renderConfigFileName = "RenderConfig.json";
constexpr const char * audioConfigFileName = "AudioConfig.json";

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

DataManager::DataManager(Engine& engine, GameConfig gameConfig) :
	engine			{ engine },
	configDirectory {}
{
	// We get a suitable runtime location..
	// https://stackoverflow.com/questions/2812760/print-tchar-on-console
	TCHAR my_documents[MAX_PATH];
	HRESULT result = SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, my_documents);

	if (result != S_OK) {
		Logger::error("Error attempting to retrieve documents directory.. Error code: {}");
		return;
	}

	configDirectory = std::filesystem::path{ my_documents } / gameConfig.gameName;

	if (!std::filesystem::exists(configDirectory)) {
		std::filesystem::create_directory(configDirectory);
	}

	// ----------
	// Loading player preference file..
	try {
		// Get documents file directory
		std::filesystem::path playerPreferenceFilePath = configDirectory / std::filesystem::path{ playerPreferenceFileName };
		
		Logger::debug("Player preference file location: {}", playerPreferenceFilePath.string());

		// The data manager is responsible for loading a player preferences file.
		std::ifstream playerPreferenceFile{ playerPreferenceFilePath };
	}
	catch (std::exception const& ex) {
		Logger::error("Failed to load player preference data. {}", ex.what());
	}
	
	// ----------
	// Loading render config file..
	try {
		// Get documents file directory
		std::filesystem::path renderConfigFilePath = configDirectory / std::filesystem::path{ renderConfigFileName };

		Logger::debug("Render config file location: {}", renderConfigFilePath.string());

		// The data manager is responsible for loading a player preferences file.
		renderConfig = Serialiser::deserialiseConfig<RenderConfig>(renderConfigFilePath.string().c_str());
	}
	catch (std::exception const& ex) {
		Logger::error("Failed to load render config data.. {}", ex.what());
	}

	// ----------
	// Loading audio config file..
	try {
		// Get documents file directory
		std::filesystem::path audioConfigFilePath = configDirectory / std::filesystem::path{ audioConfigFileName };

		Logger::debug("Audio config file location: {}", audioConfigFilePath.string());

		// The data manager is responsible for loading a player preferences file.
		audioConfig = Serialiser::deserialiseConfig<AudioConfig>(audioConfigFilePath.string().c_str());
	}
	catch (std::exception const& ex) {
		Logger::error("Failed to load render config data.. {}", ex.what());
	}
}

DataManager::~DataManager() {
	savePlayerPreference();

	std::filesystem::path renderConfigFilePath = configDirectory / std::filesystem::path{ renderConfigFileName };
	std::filesystem::path audioConfigFilePath = configDirectory / std::filesystem::path{ audioConfigFileName };

	Serialiser::serialiseConfig<RenderConfig>(renderConfigFilePath.string().c_str(), renderConfig);
	Serialiser::serialiseConfig<AudioConfig>(audioConfigFilePath.string().c_str(), audioConfig);
}

void DataManager::savePlayerPreference() {
	std::ofstream playerPreferenceFile{ configDirectory / playerPreferenceFileName };
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