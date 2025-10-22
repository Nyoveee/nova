#pragma once

#include "AssetManager/assetManager.h"

class AssetViewerUI {
public:
	AssetViewerUI(AssetManager& assetManager, ResourceManager& resourceManager);
	
	void update();
	void updateScriptFileName(AssetFilePath const& filepath, std::string const& newName, ResourceID id);
	
	template <ValidResource T>
	void displayAssetUI(BasicAssetInfo& descriptor);

public:
	void selectNewResourceId(ResourceID id);

private:
	void displayMaterialInfo(AssetInfo<Material>& descriptor);
	void displayTextureInfo(AssetInfo<Texture>& descriptor);
	void displayModelInfo(AssetInfo<Model>& descriptor);
	void displayAnimationInfo(AssetInfo<Model>& descriptor);

	void displayBoneHierarchy(BoneIndex boneIndex, Skeleton const& skeleton);
	void displayNodeHierarchy(ModelNodeIndex nodeIndex, Skeleton const& skeleton);

private:
	ResourceID selectedResourceId;
	AssetManager& assetManager;
	ResourceManager& resourceManager;

	std::string selectedResourceName;
	std::string selectedResourceStemCopy;
	std::string selectedResourceExtension;

	bool toSerialiseSelectedDescriptor;
};

#include "assetViewerUi.ipp"
