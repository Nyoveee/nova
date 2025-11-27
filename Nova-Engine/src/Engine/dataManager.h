#pragma once

#include <optional>

#include "export.h"
#include "Serialisation/serialisation.h"

class Engine;

class DataManager {
public:
	ENGINE_DLL_API DataManager(Engine& engine);
	ENGINE_DLL_API ~DataManager();

public:
	ENGINE_DLL_API void savePlayerPreference();

public:
	template <typename T>
	void saveData(std::string const& name, T const& data);

	ENGINE_DLL_API std::optional<int>			loadIntData(std::string const& name);
	ENGINE_DLL_API std::optional<float>			loadFloatData(std::string const& name);
	ENGINE_DLL_API std::optional<std::string>	loadStringData(std::string const& name);

	ENGINE_DLL_API void removeKey(std::string const& name);
	ENGINE_DLL_API void clear();

private:
	Engine& engine;
	Json playerPreferenceData;
};

#include "dataManager.ipp"