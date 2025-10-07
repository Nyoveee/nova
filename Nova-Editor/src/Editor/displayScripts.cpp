#include "ResourceManager/resourceManager.h"
#include "componentInspector.h"
#include "Editor/editor.h"
#include "Engine/engine.h"
#include <type_traits>
#include <unordered_map>

void displayScriptFields(entt::entity entity,ScriptData& scriptData, ScriptingAPIManager& scriptingAPIManager,Engine& engine) {
	
	for (FieldData& fieldData : scriptData.fields) {
		// Set the field data
		std::visit([&](auto&& arg) {
			using FieldType = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<FieldType, float>) {
				float& value{ arg };
				if (ImGui::InputFloat(fieldData.name.c_str(), &value));
				return;
			}
			if constexpr (std::is_same_v<FieldType, entt::entity>) {
				entt::entity& entityReference{ arg };
				ImGui::Text((fieldData.name + ":").c_str());
				ImGui::SameLine();
				if (entityReference != entt::null && !engine.ecs.registry.valid(entityReference))
					ImGui::Text("Invalid");
				else
					ImGui::Text((entityReference == entt::null) ? "Null" : engine.ecs.registry.get<EntityData>(entityReference).name.c_str());
				if (ImGui::BeginDragDropTarget()) {
					if (ImGuiPayload const* payload = ImGui::AcceptDragDropPayload("HIERARCHY_ITEM")) {
						entityReference = *((entt::entity*)payload->Data);
					}
					ImGui::EndDragDropTarget();
				}
				return;

				
			}
			if constexpr (std::is_same_v<FieldType, glm::vec2>) {
				if (ImGui::BeginTable("MyTable", 3, ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_NoPadInnerX)) {
					glm::vec2& value{ arg };

					ImGui::TableSetupColumn("Fixed Column", ImGuiTableColumnFlags_WidthFixed, 100.0f);

					ImGui::TableNextColumn();
					ImGui::AlignTextToFramePadding();
					ImGui::Text(fieldData.name.c_str());

					ImGui::TableNextColumn();
					if (ImGui::InputFloat("x", &value.x));
		
					ImGui::TableNextColumn();
					if (ImGui::InputFloat("y", &value.y));

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
					if (ImGui::InputFloat("x", &value.x));

					ImGui::TableNextColumn();

					if (ImGui::InputFloat("y", &value.y));

					ImGui::TableNextColumn();
					if (ImGui::InputFloat("z", &value.z));

					ImGui::EndTable();
				}
				return;
			}
		}, fieldData.data);

	}
}