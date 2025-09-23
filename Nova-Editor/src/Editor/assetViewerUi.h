#pragma once

#include "AssetManager/assetManager.h"

class AssetViewerUI {
public:
	AssetViewerUI(AssetManager& assetManager, ResourceManager& resourceManager);
	
	void update();

public:
	void selectNewResourceId(ResourceID id);

private:
	ResourceID selectedResourceId;
	AssetManager& assetManager;
	ResourceManager& resourceManager;

	std::string selectedResourceStemCopy;
	std::string selectedResourceExtension;
};