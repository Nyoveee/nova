#include "renderConfigUI.h"

#include "Editor/editor.h"
#include "Engine/engine.h"

#include "Editor/ComponentInspection/PropertyDisplay/displayProperties.h"

RenderConfigUI::RenderConfigUI(Editor& editor) :
	editor			{ editor },
	renderConfig	{ editor.engine.dataManager.renderConfig }
{}

void RenderConfigUI::update() {
	ImGui::Begin("Render Configuration");
	
	DisplayProperty<bool>(editor, "Enable SSAO?",		renderConfig.toEnableSSAO);
	DisplayProperty<bool>(editor, "Enable Fog?",		renderConfig.toEnableFog);
	DisplayProperty<bool>(editor, "Enable Vsync?",		renderConfig.toEnableVsync);
	DisplayProperty<bool>(editor, "Enable TAA?",		renderConfig.toEnableAntiAliasing);
	DisplayProperty<bool>(editor, "Enable Shadows?",	renderConfig.toEnableShadows);
	DisplayProperty<bool>(editor, "Enable IBL?",		renderConfig.toEnableIBL);
	DisplayProperty<bool>(editor, "Fullscreen?",		renderConfig.fullScreen);
	DisplayProperty<float>(editor, "Gamma",				renderConfig.gamma);

	//reflection::visit([&](auto&& fieldData) {
	//	auto& dataMember = fieldData.get();
	//	const char* dataMemberName = fieldData.name();
	//	using DataMemberType = std::decay_t<decltype(dataMember)>;

	//	// Generalization
	//	DisplayProperty<DataMemberType>(editor, dataMemberName, dataMember);
	//}, renderConfig.postProcessingOptions);

	reflection::visit([&](auto&& fieldData) {
		auto& dataMember = fieldData.get();
		const char* dataMemberName = fieldData.name();
		using DataMemberType = std::decay_t<decltype(dataMember)>;

		// Generalization
		DisplayProperty<DataMemberType>(editor, dataMemberName, dataMember);
	}, editor.engine.dataManager.audioConfig);

	ImGui::End();
}