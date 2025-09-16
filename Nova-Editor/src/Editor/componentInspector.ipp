#include "ResourceManager/resourceManager.h"
#include "imgui.h"
#include "Component/ECS.h"

template<typename T>
void ComponentInspector::displayAssetDropDownList(std::optional<ResourceID> id, const char* labelName, std::function<void(ResourceID)> onClickCallback) {
	ImGui::PushID(imguiCounter);
	++imguiCounter;

	char const* selectedAssetName = "";

	if (id) {
		Asset* selectedAsset = resourceManager.getResourceInfo(id.value());

		if (!selectedAsset) {
			selectedAssetName = "Invalid asset.";
		}
		else {
			selectedAssetName = selectedAsset->name.c_str();
		}
	}

	auto allResources = resourceManager.getAllResources<T>();

	if (ImGui::BeginCombo(labelName, selectedAssetName)) {
		for (auto&& resourceId : allResources) {
			Asset* asset = resourceManager.getResourceInfo(resourceId);

			if (ImGui::Selectable(asset->name.c_str(), id ? id.value() == asset->id : false)) {
				onClickCallback(asset->id);
			}
		}

		ImGui::EndCombo();
	}

	ImGui::PopID();
}

template<typename ...Components>
void ComponentInspector::displayComponentDropDownList(entt::entity entity) {
	if (ImGui::BeginCombo("##", "Add Component")) {

		// using fold expression, comma operator and lambda expression.
		([&]() {
			// ignore EntityData or Transform.. this component should be hidden and not be allowed to add directly.
			if constexpr (!std::same_as<Components, EntityData> && !std::same_as<Components, Transform>) {
				// if entity already has this component, don't display option..
				if (ecs.registry.all_of<Components>(entity)) {
					return;
				}

#if defined(_MSC_VER)
				// let's hope msvc doesnt change implementation haha
				// removes the `struct ` infront of type name.

				// 2 local variable of string because of lifetime :c
				std::string originalTypeName = typeid(Components).name();
				std::string typeName = originalTypeName.substr(6);

				char const* name = typeName.c_str();
#else
				constexpr char const* name = typeid(Components).name();
#endif
				if (ImGui::Selectable(name)) {
					ecs.registry.emplace_or_replace<Components>(entity, Components{});
				}
			}
		}(), ...);

		ImGui::EndCombo();
	}
}