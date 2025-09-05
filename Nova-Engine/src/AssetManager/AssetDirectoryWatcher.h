#pragma once
#include "Libraries/type_alias.h"
#include "Libraries/FileWatch.hpp"
#include <filesystem>
#include <unordered_map>
#include <vector>
class AssetManager;
class AssetDirectoryWatcher
{
public:
	AssetDirectoryWatcher(AssetManager& assetManager, std::filesystem::path rootDirectory);
	AssetDirectoryWatcher(AssetDirectoryWatcher const&) = delete;
	AssetDirectoryWatcher(AssetDirectoryWatcher&&) = delete;
	AssetDirectoryWatcher& operator=(AssetDirectoryWatcher const&) = delete;
	AssetDirectoryWatcher& operator=(AssetDirectoryWatcher&&) = delete;
public:
	void RegisterCallbackAssetContentChanged(std::function<void(AssetTypeID)> callback);
	void RegisterCallbackAssetContentDeleted(std::function<void(void)> callback);
private:
	filewatch::FileWatch<std::wstring> watch;
	std::unordered_map<std::string, AssetTypeID> extensionToAssetType;
	std::unordered_map<std::string, std::filesystem::file_time_type> lastWriteTimes;
	std::vector<std::function<void(AssetTypeID)>> assetContentChangedCallbacks;
	std::vector<std::function<void(void)>> assetContentDeletedCallbacks;

};

