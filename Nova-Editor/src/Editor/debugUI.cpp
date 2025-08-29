#include "engine.h"

#include "debugUI.h"
#include "editor.h"
#include "assetManager.h"
#include "window.h"
#include "Graphics/renderer.h"

#include "IconsFontAwesome6.h"

#include <format>

DebugUI::DebugUI(Editor& editor) :
	engine{ editor.engine },
	renderer{ editor.engine.renderer },
	assetManager{ editor.engine.assetManager },
	window{ editor.engine.window }
{
}

void DebugUI::update() {
	ImGui::Begin(ICON_FA_MOBILE_SCREEN " Statistics");

	ImGui::SeparatorText("Performance");

	ImGui::Text("FPS: %.2f", window.fps());

	ImGui::SeparatorText("Asset Manager");
	std::string threadPoolStats = std::format("{} tasks total, {} tasks running, {} tasks queued.",
		assetManager.threadPool.get_tasks_total(),
		assetManager.threadPool.get_tasks_running(),
		assetManager.threadPool.get_tasks_queued()
	);

	ImGui::Text(threadPoolStats.c_str());
	ImGui::End();
}
