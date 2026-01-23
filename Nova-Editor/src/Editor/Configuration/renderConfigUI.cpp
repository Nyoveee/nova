#include "renderConfigUI.h"

#include "Editor/editor.h"
#include "Engine/engine.h"

#include "Editor/ComponentInspection/PropertyDisplay/displayProperties.h"

RenderConfigUI::RenderConfigUI(Editor& editor) :
	editor			{ editor },
	renderConfig	{ editor.engine.renderer.renderConfig }
{}

void RenderConfigUI::update() {
	ImGui::Begin("Render Configuration");
	
	reflection::visit([&](auto&& fieldData) {
		auto& dataMember = fieldData.get();
		const char* dataMemberName = fieldData.name();
		using DataMemberType = std::decay_t<decltype(dataMember)>;

		// Generalization
		DisplayProperty<DataMemberType>(editor, dataMemberName, dataMember);
	}, renderConfig);

	ImGui::End();
}