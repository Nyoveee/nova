#include "engine.h"

#include "debugUI.h"
#include "editor.h"
#include "assetManager.h"
#include "Graphics/renderer.h"

#include "IconsFontAwesome6.h"

#include <format>

DebugUI::DebugUI(Editor& editor) :
	engine			{ editor.engine },
	renderer		{ editor.engine.renderer },
	assetManager	{ editor.engine.assetManager }
{}

void DebugUI::update() {
	ImGui::Begin(ICON_FA_MOBILE_SCREEN " Statistics");

	ImGui::Text("Asset Manager loading task pool");
	std::string threadPoolStats = std::format("{} tasks total, {} tasks running, {} tasks queued.", 
		assetManager.threadPool.get_tasks_total(),
		assetManager.threadPool.get_tasks_running(),
		assetManager.threadPool.get_tasks_queued()
	);

	ImGui::Text(threadPoolStats.c_str());
	ImGui::End();
}
