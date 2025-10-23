#include "ResourceManager/resourceManager.h"
#include "Editor/ComponentInspection/PropertyDisplay/displayProperties.h"
#include "displayProperties.h"
#include "Editor/editor.h"
#include "Engine/engine.h"
#include <type_traits>
#include <unordered_map>

void displayScriptFields(ScriptData& scriptData, Editor& editor) {
	entt::registry& registry = editor.engine.ecs.registry;

	for (FieldData& fieldData : scriptData.fields) {
	
		// Set the field data
		std::visit([&](auto&& dataMember) {
			using FieldType = std::decay_t<decltype(dataMember)>;
			const char* dataMemberName = fieldData.name.c_str();
		
			// Specializations
			if constexpr (std::is_same_v<FieldType, entt::entity>) {
				entt::entity& entityReference{ dataMember };

				ImGui::Text((fieldData.name + ":").c_str());
				ImGui::SameLine();

				if (entityReference != entt::null && !registry.valid(entityReference))
					ImGui::Text("Invalid");
				else
					ImGui::Text((entityReference == entt::null) ? "Null" : registry.get<EntityData>(entityReference).name.c_str());

				if (ImGui::BeginDragDropTarget()) {
					if (ImGuiPayload const* payload = ImGui::AcceptDragDropPayload("HIERARCHY_ITEM")) {
						entityReference = *((entt::entity*)payload->Data);
					}
					ImGui::EndDragDropTarget();
				}

				return;
			}
			else
			{
				// Generalization
				DisplayProperty<FieldType>(editor, dataMemberName, dataMember);
			}
		}, fieldData.data);
	}
}