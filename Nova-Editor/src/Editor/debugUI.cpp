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

#include "cubemap.h"

#include "API/assetSerializer.h"

DebugUI::DebugUI(Editor& editor) :
	editor			{ editor },
	engine			{ editor.engine },
	renderer		{ editor.engine.renderer },
	resourceManager	{ editor.engine.resourceManager },
	window			{ editor.engine.window }
{}

void DebugUI::update() {
	ImGui::Begin(ICON_FA_MOBILE_SCREEN " Statistics");

	editor.displayEnumDropDownList<EditingMode>(editor.editorViewPort.gizmo.editingMode, "Gizmo Editing Mode", [&](EditingMode editingMode) {
		editor.editorViewPort.gizmo.editingMode = editingMode;
	});

	renderTransformShift();

	if (ImGui::Button("Bake skybox")) {
		AssetSerializer::serialiseCubeMap(renderer.bakeDiffuseIrradianceMap([&] {
			renderer.renderSkyBox();
		}));

		AssetSerializer::serialiseCubeMap(renderer.bakeSpecularIrradianceMap([&] {
			renderer.renderSkyBox();
		}));
	}

	ImGui::Text("Hovering entity: %u", static_cast<unsigned int>(editor.hoveringEntity));

	renderPerformanceSection();
	renderPhysicsSection();
	renderHDRSection();
	renderGammaCorrectionSection();

	if (ImGui::Button("Profiler")) {
		editor.launchProfiler();
	}

	if (ImGui::Button("recompile shaders")) {
		engine.renderer.recompileShaders();
	}

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
	ImGui::Checkbox("Physics debug render", &engine.toDebugRenderPhysics);
	ImGui::Checkbox("NavMesh debug render", &engine.toDebugRenderNavMesh);
	ImGui::Checkbox("Particle Emission Shape debug render", &engine.toDebugRenderParticleEmissionShape);
	ImGui::Checkbox("(Frustum Culling) Bounding Volume debug render", &renderer.toDebugRenderBoundingVolume);
	ImGui::Checkbox("Clusters debug render", &renderer.toDebugClusters);

	ImGui::SeparatorText("Mouse positions");

	ImGui::Text("Camera Speed: %.2f", engine.cameraSystem.getCameraSpeed());

#if 0
	glm::vec3 farClipPos = { window.getClipSpacePos(), 1.f };
	glm::vec3 nearClipPos = { farClipPos.x, farClipPos.y, -1.f };

	glm::vec3 farWorldPos = renderer.getEditorCamera().clipToWorldSpace(farClipPos);
	glm::vec3 nearWorldPos = renderer.getEditorCamera().clipToWorldSpace(nearClipPos);

	ImGui::Text("Clip space mouse coords, (%2f, %2f, %2f)", farClipPos.x, farClipPos.y, farClipPos.z);
	ImGui::Text("World space far plane mouse coords, (%2f, %2f, %2f)", farWorldPos.x, farWorldPos.y, farWorldPos.z);
	ImGui::Text("World space near plane mouse coords, (%2f, %2f, %2f)", nearWorldPos.x, nearWorldPos.y, nearWorldPos.z);

	auto const& levelEditorCamera = engine.cameraSystem.getLevelEditorCamera();
	glm::vec3 raycastDirection = glm::normalize(farWorldPos - nearWorldPos);

	ImGui::Text("Level Editor camera pos, (%2f, %2f, %2f)", levelEditorCamera.position.x, levelEditorCamera.position.y, levelEditorCamera.position.z);
	ImGui::Text("Mouse raycast direction, (%2f, %2f, %2f)", raycastDirection.x, raycastDirection.y, raycastDirection.z);
#endif
}

void DebugUI::renderHDRSection() {
	ImGui::SeparatorText("Rendering..");

	ImGui::SliderFloat("Gamma", &engine.dataManager.renderConfig.gamma, 1.f, 3.0f, "%.2f");

	ImGui::Checkbox("Chromatic Aberration", &renderer.enableChromaticAberration);
	ImGui::Checkbox("Blur", &renderer.isBlurEnabled);

	float iblMultiplier = engine.renderer.iblMultiplier;
	ImGui::SliderFloat("IBL Multiplier", &iblMultiplier, 0.f, 1.0f, "%.2f");
	engine.renderer.iblMultiplier = iblMultiplier;

	if (ImGui::Button("Randomize Offset")) {
		renderer.randomiseChromaticAberrationoffset();
	}

	float exposure = renderer.getHDRExposure();
	if (ImGui::SliderFloat("Exposure", &exposure, 0.1f, 5.0f, "%.2f")) {
		renderer.setHDRExposure(exposure);
	}

	ImGui::SliderFloat("Vignette", &renderer.vignette, 0.f, 1.0f, "%.2f");

	std::string label = std::string{ "##" } + "Vignette Color";
	ImGui::ColorEdit3(label.c_str(), glm::value_ptr(renderer.vignetteColor));



#if 0
	// Tone mapping method selection
	auto currentMethod = renderer.getToneMappingMethod();

	editor.displayEnumDropDownList<Renderer::ToneMappingMethod>(currentMethod, "Tone Mapping Method", [&](auto enumValue) {
		renderer.setToneMappingMethod(enumValue);
	});
#endif

	ImGui::SliderFloat("Bloom Filter Radius", &renderer.bloomFilterRadius, 0.0001f, 0.1f);
	ImGui::SliderFloat("Bloom Composite Percentage", &renderer.bloomCompositePercentage, 0.f, 1.0f);
}

void DebugUI::renderGammaCorrectionSection() {
	ImGui::SeparatorText("Gamma Correction");

	ImGui::Checkbox("Gamma correction", &renderer.toGammaCorrect);
}

void DebugUI::renderTransformShift() {
	auto&& entities = editor.getSelectedEntities();

	if (entities.empty()) {
		return;
	}

	// shift game object by half of transform scale..
	ImGui::SeparatorText("Shift game object by half it's scale, useful for UI.");

	glm::vec3 offset = {};

	if (ImGui::Button("X+")) {
		offset.x = editor.engine.ecs.registry.get<Transform>(entities[0]).scale.x / 2.f;
	}

	ImGui::SameLine();

	if (ImGui::Button("X-")) {
		offset.x = -editor.engine.ecs.registry.get<Transform>(entities[0]).scale.x / 2.f;
	}

	ImGui::SameLine();

	if (ImGui::Button("Y+")) {
		offset.y = editor.engine.ecs.registry.get<Transform>(entities[0]).scale.y / 2.f;
	}

	ImGui::SameLine();

	if (ImGui::Button("Y-")) {
		offset.y = -editor.engine.ecs.registry.get<Transform>(entities[0]).scale.y / 2.f;
	}

	if (offset != glm::vec3{}) {
		editor.engine.ecs.registry.get<Transform>(entities[0]).position += offset;
	}
}
