#include "AssetDirectoryWatcher.h"
#include "scriptAsset.h"
#include "model.h"
#include "cubemap.h"
#include "texture.h"
#include "assetManager.h"

#include "ResourceManager/resourceManager.h"
#include "Engine/engine.h"

AssetDirectoryWatcher::AssetDirectoryWatcher(AssetManager& assetManager, ResourceManager& resourceManager, Engine& engine, std::filesystem::path rootDirectory) :
	assetManager	{ assetManager },
	resourceManager	{ resourceManager },
	engine			{ engine },
	rootDirectory	{ rootDirectory },

	watch			{
						rootDirectory.wstring(),
						[&](const std::wstring& path, const filewatch::Event change_type) {
							HandleFileChangeCallback(path, change_type);
						}
					}
{}

#if 0
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
#endif

void AssetDirectoryWatcher::HandleFileChangeCallback(const std::wstring& path, filewatch::Event change_type) {
	// the app is destructing, don't do anything.
	if (engineIsDestructing)
		return;

	std::filesystem::path absPath{ rootDirectory.string() + "\\" + std::filesystem::path(path).string()};

	if (IsPathHidden(absPath))
		return;
	
	// New File added
	if (change_type == filewatch::Event::added) {
		// Thread safe callback submission
		assetManager.submitCallback([&, absPath]() {
			engine.scriptingAPIManager.OnAssetContentAddedCallback(absPath.string());
		});

		return;
	}

	// Attempts to finds the appropriate asset id.
	ResourceID resourceId = resourceManager.getResourceID(absPath);

	if (resourceId == INVALID_ASSET_ID)
		return;

	if (change_type != filewatch::Event::removed) {
		// There is a known bug in filewatch(Since 2021...) where modified is called twice, including when it's added
		// Therefore this will check the file with the existing time before updating it(Works except copy pasting from existing files)
		std::filesystem::file_time_type time{ std::filesystem::last_write_time(absPath) };
		
		if (lastWriteTimes[absPath.string()] != time) {
			lastWriteTimes[absPath.string()] = time;

			// Thread safe callback submission
			assetManager.submitCallback([&, resourceId]() {
				engine.scriptingAPIManager.OnAssetContentModifiedCallback(resourceId);
			});
		}
	}
	else{
		assetManager.submitCallback([&, resourceId]() {
			engine.scriptingAPIManager.OnAssetContentDeletedCallback(resourceId);
		});
	}
}

bool AssetDirectoryWatcher::IsPathHidden(std::filesystem::path const& path) const
{
	if (GetFileAttributesA(path.string().c_str()) & FILE_ATTRIBUTE_HIDDEN)
		return true;
	if (path == std::filesystem::current_path())
		return false;
	return IsPathHidden(path.parent_path());
}