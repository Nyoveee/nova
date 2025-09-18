#include "Engine/engine.h"

#include "debugUI.h"
#include "editor.h"
#include "ResourceManager/resourceManager.h"
#include "Engine/window.h"
#include "Graphics/renderer.h"

#include "IconsFontAwesome6.h"

#include <format>

DebugUI::DebugUI(Editor& editor) :
	engine			{ editor.engine },
	renderer		{ editor.engine.renderer },
	resourceManager	{ editor.engine.resourceManager },
	window			{ editor.engine.window }
{}

void DebugUI::update() {
	ImGui::Begin(ICON_FA_MOBILE_SCREEN " Statistics");

	renderPerformanceSection();
	renderPhysicsSection();
	renderHDRSection();
	renderGammaCorrectionSection();

	ImGui::End();
}

void DebugUI::renderPerformanceSection() {
	ImGui::SeparatorText("Performance");
	ImGui::Text("FPS: %.2f", window.fps());

#if 0
	ImGui::SeparatorText("Asset Manager");
	std::string threadPoolStats = std::format("{} tasks total, {} tasks running, {} tasks queued.",
		assetManager.threadPool.get_tasks_total(),
		assetManager.threadPool.get_tasks_running(),
		assetManager.threadPool.get_tasks_queued()
	);

	ImGui::Text(threadPoolStats.c_str());
#endif
}

void DebugUI::renderPhysicsSection() {
	ImGui::SeparatorText("Physics");
	ImGui::Checkbox("Physics debug render", &engine.toDebugRenderPhysics);
}

void DebugUI::renderHDRSection() {
	ImGui::SeparatorText("HDR");

	float exposure = renderer.getHDRExposure();
	if (ImGui::SliderFloat("Exposure", &exposure, 0.1f, 5.0f, "%.2f")) {
		renderer.setHDRExposure(exposure);
	}

	// Tone mapping method selection
	auto currentMethod = renderer.getToneMappingMethod();

	bool isExposure = (currentMethod == decltype(currentMethod)::Exposure);
	bool isReinhard = (currentMethod == decltype(currentMethod)::Reinhard);
	bool isACES = (currentMethod == decltype(currentMethod)::ACES);

	if (ImGui::Checkbox("Exposure Tone Mapping", &isExposure) && isExposure) {
		renderer.setToneMappingMethod(decltype(currentMethod)::Exposure);
	}

	if (ImGui::Checkbox("Reinhard Tone Mapping", &isReinhard) && isReinhard) {
		renderer.setToneMappingMethod(decltype(currentMethod)::Reinhard);
	}

	if (ImGui::Checkbox("ACES Tone Mapping", &isACES) && isACES) {
		renderer.setToneMappingMethod(decltype(currentMethod)::ACES);
	}
}

void DebugUI::renderGammaCorrectionSection() {
	ImGui::SeparatorText("Gamma Correction");

	bool srgbEnabled = renderer.isSRGBFramebufferEnabled();
	if (ImGui::Checkbox("Use sRGB Framebuffer (Hardware Gamma)", &srgbEnabled)) {
		renderer.enableSRGBFramebuffer(srgbEnabled);
	}

	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("Enable hardware sRGB framebuffer for automatic gamma correction.");
	}
}
