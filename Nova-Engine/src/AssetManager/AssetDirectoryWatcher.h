#pragma once

#include "Libraries/type_alias.h"
#include "Libraries/FileWatch.hpp"
#include <filesystem>
#include <unordered_map>
#include <vector>

#include <mutex>

extern std::atomic<bool> engineIsDestructing;

class AssetManager;

class AssetDirectoryWatcher
{
public:
	AssetDirectoryWatcher(AssetManager& assetManager, std::filesystem::path rootDirectory);
	
	~AssetDirectoryWatcher() = default;
	AssetDirectoryWatcher(AssetDirectoryWatcher const&)				= delete;
	AssetDirectoryWatcher(AssetDirectoryWatcher&&)					= delete;
	AssetDirectoryWatcher& operator=(AssetDirectoryWatcher const&)	= delete;
	AssetDirectoryWatcher& operator=(AssetDirectoryWatcher&&)		= delete;

public:
	void RegisterCallbackAssetContentAdded(std::function<void(std::string)> callback);
	void RegisterCallbackAssetContentModified(std::function<void(AssetID)> callback);
	void RegisterCallbackAssetContentDeleted(std::function<void(AssetID)> callback);

private:
	void HandleFileChangeCallback(const std::wstring& path, filewatch::Event change_type);

private:
	AssetManager& assetManager;

	std::filesystem::path rootDirectory;
	filewatch::FileWatch<std::wstring> watch;

	//std::unordered_map<std::string, AssetTypeID> extensionToAssetType;
	
	std::unordered_map<std::string, std::filesystem::file_time_type> lastWriteTimes;

	std::vector<std::function<void(std::string)>> assetContentAddCallbacks;
	std::vector<std::function<void(AssetID)>> assetContentModifiedCallbacks;
	std::vector<std::function<void(AssetID)>> assetContentDeletedCallbacks;

	std::mutex contentAddCallbackMutex;
	std::mutex contentModifiedCallbackMutex;
	std::mutex contentDeleteCallbackMutex;
};

