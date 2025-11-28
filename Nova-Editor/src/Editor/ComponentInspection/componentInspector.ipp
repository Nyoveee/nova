#include "imgui.h"
#include "ECS/ECS.h"
#include "component.h"
#include "componentInspector.h"
#include "Editor/ImGui/misc/cpp/imgui_stdlib.h"

template<typename ...Components>
void ComponentInspector::displayComponentDropDownList(entt::entity entity, entt::registry& registry) {
	if (ImGui::BeginCombo("##", "Add Component")) {
		// Add a search bar..
		ImGui::InputText("Search", &componentSearchQuery);

		// Case insensitive searchQuery..
		uppercaseSearchQuery.clear();

		std::transform(componentSearchQuery.begin(), componentSearchQuery.end(), std::back_inserter(uppercaseSearchQuery), [](char c) { return static_cast<char>(std::toupper(c)); });

		// using fold expression, comma operator and lambda expression.
		([&]() {
			uppercaseTypeName.clear();

			// ignore EntityData or Transform.. this component should be hidden and not be allowed to add directly.
			if constexpr (!std::same_as<Components, EntityData> && !std::same_as<Components, Transform>) {
				// if entity already has this component, don't display option..
				if (registry.all_of<Components>(entity)) {
					return;
				}

#if defined(_MSC_VER)
				// let's hope msvc doesnt change implementation haha
				// removes the `struct ` infront of type name.

				// 2 local variable of string because of lifetime :c
				std::string originalTypeName = typeid(Components).name();
				std::string typeName = originalTypeName.substr(6);

				// Let's upper case our component name..
				std::transform(typeName.begin(), typeName.end(), std::back_inserter(uppercaseTypeName), [](char c) { return static_cast<char>(std::toupper(c)); });

				char const* name = typeName.c_str();
#else
				constexpr char const* name = typeid(Components).name();
#endif
				if (uppercaseTypeName.find(uppercaseSearchQuery) != std::string::npos) {
					if (ImGui::Selectable(name)) {
						registry.emplace_or_replace<Components>(entity, Components{});
					}
				}
			}
		}(), ...);

		ImGui::EndCombo();
	}
}