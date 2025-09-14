#include "ResourceManager/resourceManager.h"
#include "componentInspector.h"
#include "Editor/editor.h"
#include "Engine/engine.h"
#include "Engine/ScriptingAPIManager.h"
#include <type_traits>
namespace {
}
void displayScriptFields(TypedResourceID<ScriptAsset> scriptID, entt::entity entity, ComponentInspector& componentInspector) {
	ScriptingAPIManager& scriptingAPIManager{componentInspector.editor.engine.scriptingAPIManager};

	std::vector<FieldData> fieldDatas{ scriptingAPIManager.getScriptFieldDatas(static_cast<unsigned int>(entity),static_cast<std::size_t>(scriptID)) };
	for (const FieldData& fieldData : fieldDatas) {
		ImGui::Text(fieldData.first.c_str());
		ImGui::SameLine();
		std::visit([](auto&& arg) {
			using FieldType = std::decay_t<decltype(arg)>;
			FieldType value{ arg };
			// Component references entityID that has the component
			if constexpr (std::is_same_v<FieldType, entt::entity>)
				ImGui::Text((value == entt::null)?"Null": std::to_string(static_cast<unsigned int>(value)).c_str());
		}, fieldData.second);
	}
}