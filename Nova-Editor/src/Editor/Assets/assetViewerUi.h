#pragma once

#include "AssetManager/assetManager.h"
#include "Material.h"

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
	void displayMaterialInfo(AssetInfo<Material>& descriptor);
	void displayShaderInfo(AssetInfo<CustomShader>& descriptor);
	void displayTextureInfo(AssetInfo<Texture>& descriptor);
	void displayModelInfo(AssetInfo<Model>& descriptor);
	void displayAnimationInfo(AssetInfo<Model>& descriptor);
	void displayFontInfo(AssetInfo<Font>& descriptor);

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

	unsigned int copyOfSelectedFontSize = 1;

	bool toSerialiseSelectedDescriptor;
};

#include "assetViewerUi.ipp"
