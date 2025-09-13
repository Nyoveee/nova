#include "AssetDirectoryWatcher.h"
#include "scriptAsset.h"
#include "model.h"
#include "cubemap.h"
#include "texture.h"
#include "assetManager.h"


AssetDirectoryWatcher::AssetDirectoryWatcher(AssetManager& assetManager, std::filesystem::path rootDirectory) : 
	assetManager	{ assetManager },
	rootDirectory	{ rootDirectory },

	watch			{
						rootDirectory.wstring(),
						[&](const std::wstring& path, const filewatch::Event change_type) {
							HandleFileChangeCallback(path, change_type);
						}
					}
{}

void AssetDirectoryWatcher::RegisterCallbackAssetContentAdded(std::function<void(std::string)> callback){
	std::lock_guard<std::mutex> lock{ contentAddCallbackMutex };
	assetContentAddCallbacks.push_back(callback);
}

void AssetDirectoryWatcher::RegisterCallbackAssetContentModified(std::function<void(ResourceID)> callback){
	std::lock_guard<std::mutex> lock{ contentModifiedCallbackMutex };
	assetContentModifiedCallbacks.push_back(callback);
}

void AssetDirectoryWatcher::RegisterCallbackAssetContentDeleted(std::function<void(ResourceID)> callback){
	std::lock_guard<std::mutex> lock{ contentDeleteCallbackMutex };
	assetContentDeletedCallbacks.push_back(callback);
}

void AssetDirectoryWatcher::HandleFileChangeCallback(const std::wstring& path, filewatch::Event change_type) {
#if 0
	// the app is destructing, don't do anything.
	if (engineIsDestructing)
		return;
	std::filesystem::path absPath{ rootDirectory.string() + "\\" + std::filesystem::path(path).string()};
	if (IsPathHidden(absPath))
		return;
	// New File added
	if (change_type == filewatch::Event::added) {
		std::lock_guard<std::mutex> lock{ contentAddCallbackMutex };
		// Thread safe callback submission
		for (std::function<void(std::string)> callback : assetContentAddCallbacks) {
			assetManager.submitCallback([callback,absPath]() {
				callback(absPath.string());
			});
		}
	}
	// Attempts to finds the appropriate asset id.
	std::optional<AssetID> assetIdOptional = assetManager.getAssetId(absPath.string());
	if (!assetIdOptional)
		return;

	AssetID assetId = assetIdOptional.value();
	if (change_type != filewatch::Event::removed) {
		// There is a known bug in filewatch(Since 2021...) where modified is called twice, including when it's added
		// Therefore this will check the file with the existing time before updating it(Works except copy pasting from existing files)
		// jq: sadge :(
		std::filesystem::file_time_type time{ std::filesystem::last_write_time(absPath) };
		
		if (lastWriteTimes[absPath.string()] != time) {
			lastWriteTimes[absPath.string()] = time;
		
			std::lock_guard<std::mutex> lock{ contentModifiedCallbackMutex };

			// Thread safe callback submission
			for (std::function<void(AssetID)> callback : assetContentModifiedCallbacks) {
				assetManager.submitCallback([callback, assetId]() {
					callback(assetId);
				});
			}
		}
	}
	else{
		std::lock_guard<std::mutex> lock{ contentDeleteCallbackMutex };
		for (std::function<void(AssetID)> callback : assetContentDeletedCallbacks) {
			assetManager.submitCallback([callback, assetId]() {
				callback(assetId);
			});
		}
	}
#endif
}

bool AssetDirectoryWatcher::IsPathHidden(std::filesystem::path const& path) const
{
	if (GetFileAttributesA(path.string().c_str()) & FILE_ATTRIBUTE_HIDDEN)
		return true;
	if (path == std::filesystem::current_path())
		return false;
	return IsPathHidden(path.parent_path());
}