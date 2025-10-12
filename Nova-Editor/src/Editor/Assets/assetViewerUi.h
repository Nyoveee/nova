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
	void displayTextureInfo(ResourceID id, AssetInfo<Texture>& descriptor);
	void displayModelInfo(ResourceID id, AssetInfo<Model>& descriptor);

	void displayBoneHierarchy(BoneIndex boneIndex, Model const& model);

private:
	ResourceID selectedResourceId;
	AssetManager& assetManager;
	ResourceManager& resourceManager;

	std::string selectedResourceName;
	std::string selectedResourceStemCopy;
	std::string selectedResourceExtension;
};

#include "assetViewerUi.ipp"
