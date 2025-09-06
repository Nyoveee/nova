#include "AssetDirectoryWatcher.h"
#include "Asset/scriptAsset.h"
#include "Asset/model.h"
#include "Asset/cubemap.h"
#include "Asset/texture.h"
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

void AssetDirectoryWatcher::RegisterCallbackAssetContentChanged(std::function<void(AssetID)> callback){
	std::lock_guard<std::mutex> lock{ contentChangeCallbackMutex };
	assetContentChangedCallbacks.push_back(callback);
}

void AssetDirectoryWatcher::RegisterCallbackAssetContentDeleted(std::function<void(void)> callback){
	std::lock_guard<std::mutex> lock{ contentDeleteCallbackMutex };
	assetContentDeletedCallbacks.push_back(callback);
}

void AssetDirectoryWatcher::HandleFileChangeCallback(const std::wstring& path, filewatch::Event change_type) {
	// the app is destructing, don't do anything.
	if (engineIsDestructing) {
		return;
	}

	std::filesystem::path currentPath{ path };

	if (std::filesystem::is_directory(currentPath)) return;
	
	if (change_type != filewatch::Event::removed) {
		std::string absPath{ rootDirectory.string() + "\\" + currentPath.string() };
		
		// Attempts to finds the appropriate asset id.
		std::optional<AssetID> assetIdOptional = assetManager.getAssetId(absPath);

		if (!assetIdOptional) {
			return;
		}

		AssetID assetId = assetIdOptional.value();

		// There is a known bug in filewatch(Since 2021...) where modified is called twice, including when it's added
		// Therefore this will check the file with the existing time before updating it(Works except copy pasting from existing files)
		// jq: sadge :(
		std::filesystem::file_time_type time{ std::filesystem::last_write_time(absPath) };
		
		if (lastWriteTimes[absPath] != time) {
			lastWriteTimes[absPath] = time;
		
			std::lock_guard<std::mutex> lock{ contentChangeCallbackMutex };

			// Thread safe callback submission
			for (std::function<void(AssetID)> callback : assetContentChangedCallbacks) {
				assetManager.submitCallback([callback, assetId]() {
					callback(assetId);
				});
			}
		}
	}
	// Thread safe callback submission
	else /*change_type == filewatch::Event::removed*/ {

		std::lock_guard<std::mutex> lock{ contentDeleteCallbackMutex };

		for (std::function<void(void)> callback : assetContentDeletedCallbacks) {
			assetManager.submitCallback(callback);
		}
	}
}
