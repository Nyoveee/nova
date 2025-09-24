#include "ResourceManager/resourceManager.h"
#include "componentInspector.h"
#include "Editor/editor.h"
#include "Engine/engine.h"
#include "Engine/ScriptingAPIManager.h"
#include <type_traits>
#include <unordered_map>
void displayScriptFields(entt::entity entity,ScriptData& scriptData, ScriptingAPIManager& scriptingAPIManager,Engine& engine) {
	
	for (FieldData& fieldData : scriptData.fields) {
		bool modified{ false };
		// Set the field data
		std::visit([&](auto&& arg) {
			using FieldType = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<FieldType, float>) {
				float& value{ arg };
				if(ImGui::InputFloat(fieldData.name.c_str(), &value)) modified = true;
				return;
			}
			if constexpr (std::is_same_v<FieldType, entt::entity>) {
				entt::entity& entityReference{ arg };
				ImGui::Text((fieldData.name + ":").c_str());
				ImGui::SameLine();
				if (entityReference != entt::null && !engine.ecs.registry.valid(entityReference))
				{
					entityReference = entt::null;
					modified = true;
				}
				ImGui::Text((entityReference == entt::null) ? "Null" : engine.ecs.registry.get<EntityData>(entityReference).name.c_str());
				if (ImGui::BeginDragDropTarget()) {
					if (ImGuiPayload const* payload = ImGui::AcceptDragDropPayload("HIERARCHY_ITEM")) {
						entityReference = *((entt::entity*)payload->Data);
						modified = true;
					}
					ImGui::EndDragDropTarget();
				}
				
			}
			if constexpr (std::is_same_v<FieldType, glm::vec2>) {
				if (ImGui::BeginTable("MyTable", 3, ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_NoPadInnerX)) {
					glm::vec2& value{ arg };

					ImGui::TableSetupColumn("Fixed Column", ImGuiTableColumnFlags_WidthFixed, 100.0f);

					ImGui::TableNextColumn();
					ImGui::AlignTextToFramePadding();
					ImGui::Text(fieldData.name.c_str());

					ImGui::TableNextColumn();
					if (ImGui::InputFloat("x", &value.x)) modified = true;
		
					ImGui::TableNextColumn();
					if (ImGui::InputFloat("y", &value.y)) modified = true;

					ImGui::EndTable();
				}
				return;
			}
			if constexpr (std::is_same_v<FieldType, glm::vec3>) {
				if (ImGui::BeginTable("MyTable", 4, ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_NoPadInnerX)) {
					glm::vec3& value{ arg };

					ImGui::TableSetupColumn("Fixed Column", ImGuiTableColumnFlags_WidthFixed, 100.0f);

					ImGui::TableNextColumn();
					ImGui::AlignTextToFramePadding();
					ImGui::Text(fieldData.name.c_str());
					ImGui::TableNextColumn();
					if (ImGui::InputFloat("x", &value.x)) modified = true;

					ImGui::TableNextColumn();

					if (ImGui::InputFloat("y", &value.y)) modified = true;

					ImGui::TableNextColumn();
					if (ImGui::InputFloat("z", &value.z)) modified = true;

					ImGui::EndTable();
				}
				return;
			}
		}, fieldData.data);

#if 0
		// Update the script instance if a field is modified
		if (modified && !scriptingAPIManager.setScriptFieldData(entity, scriptData.scriptId, fieldData))
			scriptData.fields = scriptingAPIManager.getScriptFieldDatas(entity, scriptData.scriptId); // If we failed to set, like invalid component, reset to previous setting
	
#endif
	}
}