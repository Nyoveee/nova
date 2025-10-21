#pragma once

#include "AssetManager/assetManager.h"

class AssetViewerUI {
public:
	AssetViewerUI(AssetManager& assetManager, ResourceManager& resourceManager);
	
	void update();
	void updateScriptFileName(AssetFilePath const& filepath, std::string const& newName, ResourceID id);
	
	template <ValidResource T>
	void displayAssetUI(ResourceID id, BasicAssetInfo& descriptor);

public:
	void selectNewResourceId(ResourceID id);

private:
	ResourceID selectedResourceId;
	AssetManager& assetManager;
	ResourceManager& resourceManager;

	std::string selectedResourceName;
	std::string selectedResourceStemCopy;
	std::string selectedResourceExtension;
};

#include "assetViewerUi.ipp"
