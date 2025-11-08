#pragma once

#include "AssetManager/assetManager.h"
#include "Material.h"

class Editor;

class AssetViewerUI {
public:
	AssetViewerUI(Editor& editor, AssetManager& assetManager, ResourceManager& resourceManager);
	
	void update();
	void updateScriptFileName(AssetFilePath const& filepath, ResourceID id);
	
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

	template <typename T>
	void recompileResourceWithUpdatedDescriptor(AssetInfo<T> const& assetInfo);

private:
	ResourceID selectedResourceId;
	Editor& editor;
	AssetManager& assetManager;
	ResourceManager& resourceManager;

private:
	// We make a copy of the following data members for temporarily editing..
	// Once these fields are edited, we verify the validity of these fields are recompile asset when needed.
	// ---------------------------------------------------
	// Name applies for all resource
	std::string selectedResourceName;
	std::string selectedResourceStemCopy;
	std::string selectedResourceExtension;

	// Font
	unsigned int copyOfSelectedFontSize = 1;

	// Model
	float copyOfScale = 1.f;

	// ---------------------------------------------------
	bool toSerialiseSelectedDescriptor;
};

#include "assetViewerUi.ipp"
