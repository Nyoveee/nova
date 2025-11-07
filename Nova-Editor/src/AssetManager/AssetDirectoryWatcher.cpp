#include "AssetDirectoryWatcher.h"
#include "scriptAsset.h"
#include "model.h"
#include "cubemap.h"
#include "texture.h"
#include "assetManager.h"

#include "ResourceManager/resourceManager.h"
#include "Engine/engine.h"
#include "Editor/editor.h"

AssetDirectoryWatcher::AssetDirectoryWatcher(AssetManager& assetManager, ResourceManager& resourceManager, Engine& engine) :
	assetManager	{ assetManager },
	resourceManager	{ resourceManager },
	engine			{ engine },
	watch			{
						AssetIO::assetDirectory.wstring(),
						[&](const std::wstring& path, const filewatch::Event change_type) {
							// the app is destructing or the asset manager hasn't finish initialising.., don't do anything 
							if (engineIsDestructing || !assetManager.hasInitialised)
								return;

							assetManager.submitCallback([&, path, change_type]() { 
								HandleFileChangeCallback(path, change_type);
							});
						}
					}
{}

void AssetDirectoryWatcher::HandleFileChangeCallback(const std::wstring& path, filewatch::Event change_type) {
	std::filesystem::path absPath{ AssetIO::assetDirectory / std::filesystem::path(path) };

	if (IsPathHidden(absPath))
		return;
	
	// Attempts to finds the appropriate asset id.
	ResourceID resourceId = assetManager.getResourceID(absPath);

	switch (change_type)
	{
	case filewatch::Event::added:
		engine.scriptingAPIManager.OnAssetContentAddedCallback(absPath.string());
		assetManager.onAssetAddition(absPath);
		break;
	case filewatch::Event::removed:
		engine.scriptingAPIManager.OnAssetContentDeletedCallback(resourceId);
		assetManager.onAssetDeletion(resourceId);
		
		break;
	case filewatch::Event::modified: {
		if (resourceId == INVALID_RESOURCE_ID)
			break;

		// There is a known bug in filewatch(Since 2021...) where modified is called twice, including when it's added
		// Therefore this will check the file with the existing time before updating it(Works except copy pasting from existing files)

		// + renaming events sometimes cause the modified event to be invoked.
		// we combat this by comparing the previous last write modified.

		//editor.navMeshGenerator.AddNavMeshSurface(resourceId);

		std::filesystem::file_time_type lastWriteTime{ std::filesystem::last_write_time(absPath) };

		// get the last write of this invocation..
		auto iterator = lastWriteTimes.find(resourceId);

		if (iterator != lastWriteTimes.end()) {
			// check if this invocation is a repeated one.
			if (iterator->second == lastWriteTime) {
				// this is a repeated invocation.
				break;
			}

			// update map..
			iterator->second = lastWriteTime;
		}
		else {
			// new invocation, let's record on our map..
			lastWriteTimes.insert({ resourceId, lastWriteTime });
		}


		BasicAssetInfo* descriptor = assetManager.getDescriptor(resourceId);

		if (!descriptor) {
			return;
		}

		auto epoch = std::chrono::duration_cast<std::chrono::milliseconds>(lastWriteTime.time_since_epoch());

		Logger::debug("Handling file change..");
		engine.scriptingAPIManager.OnAssetContentModifiedCallback(resourceId);
		assetManager.onAssetModification(resourceId, absPath);
		break;
	}
	default:
		break;
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