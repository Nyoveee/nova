#pragma once

#include "AssetManager/assetManager.h"
#include "Material.h"

class Editor;

class AssetViewerUI {
public:
	AssetViewerUI(Editor& editor, AssetManager& assetManager, ResourceManager& resourceManager);
	
	void update();
	void updateScriptFileName(AssetFilePath const& filepath, ResourceID id);
	//void broadcast();
	void updateScriptFilePath(AssetFilePath const& filepath, ResourceID id);
	
	template <ValidResource T>
	void displayAssetUI(BasicAssetInfo& descriptor);

	//template<typename ...Components>
	//void updateComponents(entt::registry& ecsRegistry, entt::registry& prefabRegistry, entt::entity entity, entt::entity prefabEntity);

public:
	// this is the public facing api when attempting to select a new resource id..
	// it populates the resource navigation history
	void selectNewResourceId(ResourceID id);

private:
	void displayMaterialInfo(AssetInfo<Material>& descriptor);
	void displayShaderInfo(AssetInfo<CustomShader>& descriptor);
	void displayTextureInfo(AssetInfo<Texture>& descriptor);
	void displayModelInfo(AssetInfo<Model>& descriptor);
	void displayAnimationInfo(AssetInfo<Model>& descriptor);
	void displayFontInfo(AssetInfo<Font>& descriptor);
	void displayPrefabInfo(AssetInfo<Prefab>& descriptor);

	void displayBoneHierarchy(BoneIndex boneIndex, Skeleton& skeleton);
	void displayNodeHierarchy(ModelNodeIndex nodeIndex, Skeleton const& skeleton);

	template <typename T>
	void recompileResourceWithUpdatedDescriptor(AssetInfo<T> const& assetInfo);
	
	// this function is responsible for actually switching resource id
	void selectResourceID(ResourceID id);

	// selects the previous / next resource id based on history
	void selectPreviousResourceID();
	void selectNextResourceID();

	void displayNavigationHistory();

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
	std::string selectedResourceExtension;

	// Font
	unsigned int copyOfSelectedFontSize = 1;

	// Model
	float copyOfScale = 1.f;

	// Selected prefab entity
	entt::entity rootPrefabEntity;
	entt::entity selectedPrefabEntity;

	// ---------------------------------------------------
	bool toSerialiseSelectedDescriptor;

	// --- We store history of navigated resources for easy traversal --- (like undo, redo)
	std::list<ResourceID> previousResourceIds;
	std::list<ResourceID> nextResourceIds;

	bool toOverrideEditSystemResource;
	bool recentlyChangedResource;
};

#include "assetViewerUi.ipp"
