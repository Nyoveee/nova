#include "reflection.h"

#include "PropertyDisplay/displayProperties.h"
namespace {
	// https://stackoverflow.com/questions/54182239/c-concepts-checking-for-template-instantiation

	template<typename Component>
	void displayComponent(ComponentInspector& componentInspector, entt::entity entity, Component& component, bool b, entt::registry& registry) {
	//void displayComponent(ComponentInspector& componentInspector, entt::entity entity, Component& component) {
		(void) entity;

		[[maybe_unused]] Editor& editor = componentInspector.editor;

		if constexpr (!reflection::isReflectable<Component>()) {
			return;
		}
		else if constexpr (std::same_as<Component, EntityData>) {
			return;
		}
		else {

#if defined(_MSC_VER)
			// let's hope msvc doesnt change implementation haha
			// removes the `struct ` infront of type name.

			// 2 local variable of string because of lifetime :c
			std::string originalTypeName = typeid(Component).name();
			std::string typeName = originalTypeName.substr(6);

			char const* name = typeName.c_str();
#else
			constexpr char const* name = typeid(Component).name();
#endif
			bool toDisplay;
			bool toShowHeader = true;

			// show the close button only for components that are not transform.
			if constexpr (!std::same_as<Component, Transform>) {
				toDisplay = ImGui::CollapsingHeader(name, &toShowHeader);
			}
			else {
				(void) toShowHeader;
				toDisplay = ImGui::CollapsingHeader(name);
			}

			if (!toDisplay) {
				// don't bother rendering each data member's ui if the collapsing header
				// is not active, go straight to end.
				goto end;
			}

			if (b) {
				EntityData* entityData = registry.try_get<EntityData>(entity);
				if (entityData->prefabID == INVALID_RESOURCE_ID) {
					b = false;
				}
			}

			ImGui::BeginDisabled(b);
			
			ImGui::PushID(static_cast<int>(entity));
			ImGui::PushID(static_cast<int>(typeid(Component).hash_code()));
			// Visits each of the component's data member and renders them.
			reflection::visit(
				[&](auto fieldData) {
					auto& dataMember = fieldData.get();
					constexpr const char* dataMemberName = fieldData.name();
					using DataMemberType = std::decay_t<decltype(dataMember)>;
					ImGui::PushID(static_cast<int>(std::hash<std::string>{}(dataMemberName)));
					// Generalization
					DisplayProperty<DataMemberType>(editor, dataMemberName, dataMember);
					ImGui::PopID();

					//for (entt::entity en : registry.view<entt::entity>()) {
					//	EntityData* enData = registry.try_get<EntityData>(en);
					//	EntityData* entityData = registry.try_get<EntityData>(entity);
					//	if (enData->prefabID == entityData->prefabID) {
					//		auto* entityComponent = registry.try_get<Component>(entity);
					//		auto* enComponent = registry.try_get<Component>(en);
					//	}
					//}
				},
			component);
			
			ImGui::PopID();
			ImGui::PopID();

			ImGui::EndDisabled();


		end:
			// prompted to delete component.
			if (!toShowHeader) {
				registry.erase<Component>(entity);
			}
		}
	}
}