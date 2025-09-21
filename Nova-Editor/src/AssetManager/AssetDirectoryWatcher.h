#pragma once

#include "type_alias.h"
#include "FileWatch.hpp"
#include <filesystem>
#include <unordered_map>
#include <vector>

#include <mutex>

// variable is defined in Window class.
ENGINE_DLL_API extern std::atomic<bool> engineIsDestructing;

class AssetManager;
class ResourceManager;
class Engine;

class AssetDirectoryWatcher
{
public:
	AssetDirectoryWatcher(AssetManager& assetManager, ResourceManager& resourceManager, Engine& engine);
	
	~AssetDirectoryWatcher() = default;
	AssetDirectoryWatcher(AssetDirectoryWatcher const&)				= delete;
	AssetDirectoryWatcher(AssetDirectoryWatcher&&)					= delete;
	AssetDirectoryWatcher& operator=(AssetDirectoryWatcher const&)	= delete;
	AssetDirectoryWatcher& operator=(AssetDirectoryWatcher&&)		= delete;

public:
	bool IsPathHidden(std::filesystem::path const& path) const;

private:
	void HandleFileChangeCallback(const std::wstring& path, filewatch::Event change_type);

private:
	AssetManager& assetManager;
	ResourceManager& resourceManager;
	Engine& engine;

	filewatch::FileWatch<std::wstring> watch;

	std::unordered_map<ResourceID, std::filesystem::file_time_type> lastWriteTimes;
};

