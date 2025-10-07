#pragma once

#include <memory>
#include <functional>
#include <optional>
#include <entt/entt.hpp>

#include "type_alias.h"

class ECS;
class Editor;
class ResourceManager;
class AssetManager;
class AudioSystem;

struct ScriptData;

class ComponentInspector {
public:
	ComponentInspector(Editor& editor);

public:
	void update();
	void displayAvailableScriptDropDownList(std::vector<ScriptData> const& ownedScripts, std::function<void(ResourceID)> onClickCallback);

public:
	template <typename ...Components>
	void displayComponentDropDownList(entt::entity entity);

public:
	ECS& ecs;
	Editor& editor;
	ResourceManager& resourceManager;
	AssetManager& assetManager;
	AudioSystem& audioSystem;
};

#include "componentInspector.ipp"
