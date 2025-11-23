#include "ResourceManager/resourceManager.h"
#include "Editor/ComponentInspection/PropertyDisplay/displayProperties.h"
#include "displayProperties.h"
#include "Editor/editor.h"
#include "Engine/engine.h"
#include <type_traits>
#include <unordered_map>

void displayScriptFields(ScriptData& scriptData, Editor& editor) {
	for (FieldData& fieldData : scriptData.fields) {
		DisplayProperty<serialized_field_type>(editor, fieldData.name.c_str(), fieldData.data);
	}
}