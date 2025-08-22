#pragma once

#include <memory>
#include <functional>

#include "Libraries/type_alias.h"

class ECS;
class Editor;
class AssetManager;

class ComponentInspector {
public:
	ComponentInspector(Editor& editor);

public:
	void update();

public:
	template <typename T>
	void displayAssetDropDownList(AssetID id, const char* labelName, std::function<void(AssetID)> onClickCallback);

public:
	ECS& ecs;
	Editor& editor;
	AssetManager& assetManager;

	int imguiCounter = 0;
};

#include "assetManager.h"
#include "imgui.h"

template<typename T>
void ComponentInspector::displayAssetDropDownList(AssetID id, const char* labelName, std::function<void(AssetID)> onClickCallback) {
	ImGui::PushID(imguiCounter);
	++imguiCounter;

	Asset* selectedAsset = assetManager.getAssetInfo(id);
	auto allAssets = assetManager.getAllAssets<T>();

	char const* selectedAssetName;

	if (!selectedAsset) {
		selectedAssetName = "Invalid asset.";
	}
	else {
		selectedAssetName = selectedAsset->name.c_str();
	}

	if (ImGui::BeginCombo(labelName, selectedAssetName)) {
		for (auto&& asset : allAssets) {
			if (ImGui::Selectable(asset.get().name.c_str(), id == asset.get().id)) {
				onClickCallback(asset.get().id);
			}
		}
		
		ImGui::EndCombo();
	}

	ImGui::PopID();
}
