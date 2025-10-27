#include "imgui.h"
#include "ECS/ECS.h"
#include "component.h"
#include "componentInspector.h"

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