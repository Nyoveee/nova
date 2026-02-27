#include "reflection.h"

#include "PropertyDisplay/displayProperties.h"

void displayLightComponent(Editor& editor, Light& dataMember);
void displayReflectionProbeComponent(Editor& editor, ReflectionProbe& dataMember, Transform const& transform);

template <typename T> // T can be either MeshRenderer or SkinnedMeshRenderer.
void displayRendererComponent(Editor& editor, T& rendererComponent, entt::entity entity);

namespace {
	// https://stackoverflow.com/questions/54182239/c-concepts-checking-for-template-instantiation

	template<typename Component>
	void displayComponent([[maybe_unused]] Editor& editor, [[maybe_unused]] entt::entity entity, Component& component, entt::registry& registry, bool disable) {
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

			ImGui::PushID(static_cast<int>(entity));
			ImGui::PushID(static_cast<int>(typeid(Component).hash_code()));

			// show the close button and active checkbox only for components that are not transform.
			if constexpr (!std::same_as<Component, Transform>) {
				toDisplay = ImGui::CollapsingHeader(name, &toShowHeader);
				if constexpr(!NonComponentDisablingTypes<Component>) {
					// Active State
					EntityData* const entityData{ registry.try_get<EntityData>(entity) };

					if (entityData) {
						bool b_Active{ editor.engine.ecs.isComponentActive<Component>(entity)};
						// Display Checkbox
						ImGui::SameLine();
						ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6.5f);
						ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.24f, 0.24f, 0.24f, 1.0f));
						ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.28f, 0.28f, 0.28f, 1.0f));
						ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.32f, 0.32f, 0.32f, 1.0f));
						ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.1f, 0.1f));
						if (ImGui::Checkbox("##", &b_Active))
							editor.engine.ecs.setComponentActive<Component>(entity, b_Active);
						ImGui::PopStyleVar();
						ImGui::PopStyleColor();
						ImGui::PopStyleColor();
						ImGui::PopStyleColor();
					}
				}
				
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

			
			if (disable) {
				EntityData* entityData = registry.try_get<EntityData>(entity);
				if (entityData->prefabID == INVALID_RESOURCE_ID) {
					disable = false;
				}
				else {
					ImGui::Checkbox("Override?", &entityData->overridenComponents[Family::id<Component>()]);
					disable = !entityData->overridenComponents[Family::id<Component>()];
				}
			}

			ImGui::BeginDisabled(disable);
			
			ImGui::PushID(static_cast<int>(entity));
			ImGui::PushID(static_cast<int>(typeid(Component).hash_code()));

			// light has it's own display light logic..
			if constexpr (std::same_as<Component, Light>) {
				displayLightComponent(editor, component);
			}
			// reflection probe has an extra bake button.. and some constraints..
			else if constexpr (std::same_as<Component, ReflectionProbe>) {
				displayReflectionProbeComponent(editor, component, registry.get<Transform>(entity));
			}
			// same for renderers..
			else if constexpr (std::same_as<Component, MeshRenderer> || std::same_as<Component, SkinnedMeshRenderer>) {
				displayRendererComponent<Component>(editor, component, entity);
			}
			else {
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
					},
				component);
			}

			ImGui::PopID();
			ImGui::PopID();
		

			ImGui::EndDisabled();


		end:
			// prompted to delete component.
			if (!toShowHeader) 
				registry.erase<Component>(entity);
		
			ImGui::PopID();
			ImGui::PopID();
		}

	}
}

#include "displayRendererComponent.ipp"