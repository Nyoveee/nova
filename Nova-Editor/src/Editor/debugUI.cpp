#include "Engine/engine.h"

#include "debugUI.h"
#include "editor.h"
#include "ResourceManager/resourceManager.h"
#include "Engine/window.h"
#include "Graphics/renderer.h"

#include "IconsFontAwesome6.h"
#include "magic_enum.hpp"

#include <format>
#include <GLFW/glfw3.h>


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
	ImGui::Text("Drawcall: %d", renderer.drawCalls());
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

	ImGui::Checkbox("NavMesh debug render", &engine.toDebugRenderNavMesh);

	ImGui::SeparatorText("Mouse positions");

	glm::vec3 farClipPos = { window.getClipSpacePos(), 1.f };
	glm::vec3 nearClipPos = { farClipPos.x, farClipPos.y, -1.f };

	glm::vec3 farWorldPos = renderer.getCamera().clipToWorldSpace(farClipPos);
	glm::vec3 nearWorldPos = renderer.getCamera().clipToWorldSpace(nearClipPos);

	ImGui::Text("Camera Speed: %.2f", engine.cameraSystem.getCameraSpeed());

	ImGui::Text("Clip space mouse coords, (%2f, %2f, %2f)", farClipPos.x, farClipPos.y, farClipPos.z);
	ImGui::Text("World space far plane mouse coords, (%2f, %2f, %2f)", farWorldPos.x, farWorldPos.y, farWorldPos.z);
	ImGui::Text("World space near plane mouse coords, (%2f, %2f, %2f)", nearWorldPos.x, nearWorldPos.y, nearWorldPos.z);

	auto const& levelEditorCamera = engine.cameraSystem.getLevelEditorCamera();
	glm::vec3 raycastDirection = glm::normalize(farWorldPos - nearWorldPos);

	ImGui::Text("Level Editor camera pos, (%2f, %2f, %2f)", levelEditorCamera.position.x, levelEditorCamera.position.y, levelEditorCamera.position.z);
	ImGui::Text("Mouse raycast direction, (%2f, %2f, %2f)", raycastDirection.x, raycastDirection.y, raycastDirection.z);
}

void DebugUI::renderHDRSection() {
	ImGui::SeparatorText("HDR");

	float exposure = renderer.getHDRExposure();
	if (ImGui::SliderFloat("Exposure", &exposure, 0.1f, 5.0f, "%.2f")) {
		renderer.setHDRExposure(exposure);
	}

	// Tone mapping method selection
	auto currentMethod = renderer.getToneMappingMethod();

	// get the list of all possible enum values
	constexpr auto listOfEnumValues = magic_enum::enum_entries<Renderer::ToneMappingMethod>();

	if (ImGui::BeginCombo("Tone Mapping Method", std::string{ magic_enum::enum_name<Renderer::ToneMappingMethod>(currentMethod) }.c_str())) {
		for (auto&& [enumValue, enumInString] : listOfEnumValues) {
			if (ImGui::Selectable(std::string{ enumInString }.c_str(), enumValue == currentMethod)) {
				renderer.setToneMappingMethod(enumValue);
			}
		}

		ImGui::EndCombo();
	}
}

void DebugUI::renderGammaCorrectionSection() {
	ImGui::SeparatorText("Gamma Correction");

	ImGui::Checkbox("Gamma correction", &renderer.toGammaCorrect);
}
