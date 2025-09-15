#include "ResourceManager/resourceManager.h"
#include "componentInspector.h"
#include "Editor/editor.h"
#include "Engine/engine.h"
#include "Engine/ScriptingAPIManager.h"
#include <type_traits>
void displayScriptFields(ScriptData& scriptData, ComponentInspector& componentInspector) {

	for (const FieldData& fieldData : scriptData.fields) {
		ImGui::Text(fieldData.first.c_str());
		std::visit([](auto&& arg) {
			using FieldType = std::decay_t<decltype(arg)>;
			FieldType value{ arg };
			(void)value; // To Do replace this with editable fields
		}, fieldData.second);
	}
}