#pragma once

#include "AssetManager/assetManager.h"

class Editor;

class AssetViewerUI {
public:
	AssetViewerUI(Editor& editor, AssetManager& assetManager, ResourceManager& resourceManager);
	
	void update();
	void updateScriptFileName(AssetFilePath const& filepath, std::string const& newName, ResourceID id);
	
	template <ValidResource T>
	void displayAssetUI(BasicAssetInfo& descriptor);

public:
	void selectNewResourceId(ResourceID id);

private:
	void displayMaterialInfo();
	void displayShaderInfo();
	void displayTextureInfo(AssetInfo<Texture>& descriptor);
	void displayModelInfo(AssetInfo<Model>& descriptor);
	void displayAnimationInfo(AssetInfo<Model>& descriptor);

	void displayBoneHierarchy(BoneIndex boneIndex, Skeleton const& skeleton);
	void displayNodeHierarchy(ModelNodeIndex nodeIndex, Skeleton const& skeleton);

private:
	ResourceID selectedResourceId;
	Editor& editor;
	AssetManager& assetManager;
	ResourceManager& resourceManager;

	std::string selectedResourceName;
	std::string selectedResourceStemCopy;
	std::string selectedResourceExtension;

	bool toSerialiseSelectedDescriptor;
};

#include "assetViewerUi.ipp"
