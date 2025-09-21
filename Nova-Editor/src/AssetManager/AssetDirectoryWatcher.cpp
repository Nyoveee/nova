#include "AssetDirectoryWatcher.h"
#include "scriptAsset.h"
#include "model.h"
#include "cubemap.h"
#include "texture.h"
#include "assetManager.h"

#include "ResourceManager/resourceManager.h"
#include "Engine/engine.h"

AssetDirectoryWatcher::AssetDirectoryWatcher(AssetManager& assetManager, ResourceManager& resourceManager, Engine& engine) :
	assetManager	{ assetManager },
	resourceManager	{ resourceManager },
	engine			{ engine },

	watch			{
						AssetIO::assetDirectory.wstring(),
						[&](const std::wstring& path, const filewatch::Event change_type) {
							HandleFileChangeCallback(path, change_type);
						}
					}
{}

void AssetDirectoryWatcher::HandleFileChangeCallback(const std::wstring& path, filewatch::Event change_type) {
	// the app is destructing, don't do anything.
	if (engineIsDestructing)
		return;

	std::filesystem::path absPath{ AssetIO::assetDirectory / std::filesystem::path(path) };

	if (IsPathHidden(absPath))
		return;
	
	// Attempts to finds the appropriate asset id.
	ResourceID resourceId = assetManager.getDescriptor(absPath).descriptor.id;

	switch (change_type)
	{
	case filewatch::Event::added:
		// Thread safe callback submission
		assetManager.submitCallback([&, absPath]() {
			engine.scriptingAPIManager.OnAssetContentAddedCallback(absPath.string());
			assetManager.onAssetAddition(absPath);
		});

		break;
	case filewatch::Event::removed:
		assetManager.submitCallback([&, resourceId]() {
			engine.scriptingAPIManager.OnAssetContentDeletedCallback(resourceId);
			assetManager.onAssetDeletion(resourceId);
		});
		
		break;
	case filewatch::Event::modified: {
		if (resourceId == INVALID_RESOURCE_ID)
			break;

		// There is a known bug in filewatch(Since 2021...) where modified is called twice, including when it's added
		// Therefore this will check the file with the existing time before updating it(Works except copy pasting from existing files)
		std::filesystem::file_time_type time{ std::filesystem::last_write_time(absPath) };

		if (lastWriteTimes[absPath.string()] != time) {
			lastWriteTimes[absPath.string()] = time;

			// Thread safe callback submission
			assetManager.submitCallback([&, resourceId]() {
				engine.scriptingAPIManager.OnAssetContentModifiedCallback(resourceId);
				assetManager.onAssetModification(resourceId, absPath);
			});
		}

		break;
	}
	case filewatch::Event::renamed_old:
		break;
	case filewatch::Event::renamed_new:
		break;
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