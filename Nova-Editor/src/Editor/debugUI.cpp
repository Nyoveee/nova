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

	ImGui::SeparatorText("Physics");

	ImGui::Checkbox("Physics debug render", &engine.toDebugRenderPhysics);

	ImGui::SeparatorText("HDR");

	float exposure = renderer.getHDRExposure();
	if (ImGui::SliderFloat("Exposure", &exposure, 0.1f, 5.0f, "%.2f")) {
		renderer.setHDRExposure(exposure);
	}

	ImGui::End();
}
