#include "AssetDirectoryWatcher.h"
#include "Asset/scriptAsset.h"
#include "Asset/model.h"
#include "Asset/cubemap.h"
#include "Asset/texture.h"
#include "assetManager.h"
AssetDirectoryWatcher::AssetDirectoryWatcher(AssetManager& assetManager, std::filesystem::path rootDirectory)
: watch{
	rootDirectory.wstring(),
	[&](const std::wstring& path, const filewatch::Event change_type) {
		(void)change_type;
		std::filesystem::path currentPath{ path };
		if (std::filesystem::is_directory(currentPath)) return;
		if (!extensionToAssetType.contains(currentPath.extension().string())) return;
		if (change_type != filewatch::Event::removed) {
			std::string absPath{ rootDirectory.string() + "\\" + currentPath.string()};
			// There is a known bug in filewatch(Since 2021...) where modified is called twice, including when it's added
			// Therefore this will check the file with the existing time before updating it(Works except copy pasting from existing files)
			std::filesystem::file_time_type time{ std::filesystem::last_write_time(absPath)};
			if (lastWriteTimes[absPath] != time) {
				lastWriteTimes[absPath] = time;
				// Thread safe callback submission
				for (std::function<void(AssetTypeID)> callback : assetContentChangedCallbacks) 
					assetManager.submitCallback(std::bind(callback, extensionToAssetType[currentPath.extension().string()]));
			}
		}
		// Thread safe callback submission
		if (change_type == filewatch::Event::removed)
			for (std::function<void(void)> callback : assetContentDeletedCallbacks)
				assetManager.submitCallback(callback);
	}
}
{
	extensionToAssetType.emplace(".fbx", Family::id<Model>());
	extensionToAssetType.emplace(".png", Family::id<Texture>());
	extensionToAssetType.emplace(".jpg", Family::id<Texture>());
	extensionToAssetType.emplace(".exr", Family::id<CubeMap>());
	extensionToAssetType.emplace(".cs", Family::id<ScriptAsset>());
}

void AssetDirectoryWatcher::RegisterCallbackAssetContentChanged(std::function<void(AssetTypeID)> callback){
	assetContentChangedCallbacks.push_back(callback);
}

void AssetDirectoryWatcher::RegisterCallbackAssetContentDeleted(std::function<void(void)> callback){
	assetContentDeletedCallbacks.push_back(callback);
}
