#pragma once

#include <memory>
#include <functional>
#include <optional>

#include "type_alias.h"

class ECS;
class Editor;
class AssetManager;

class ComponentInspector {
public:
	ComponentInspector(Editor& editor);

public:
	void update();

public:
	// displays a ImGui combo drop down box of all the assets related to type T.
	// first parameter is used to specific which asset id is selected.
	template <typename T>
	void displayAssetDropDownList(std::optional<AssetID> id, const char* labelName, std::function<void(AssetID)> onClickCallback);

	template <typename ...Components>
	void displayComponentDropDownList(entt::entity entity);

public:
	ECS& ecs;
	Editor& editor;
	AssetManager& assetManager;

	int imguiCounter = 0;
};

#include "componentInspector.ipp"
