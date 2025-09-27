#include "Engine/engine.h"

#include "debugUI.h"
#include "editor.h"
#include "ResourceManager/resourceManager.h"
#include "Engine/window.h"
#include "Graphics/renderer.h"

#include "IconsFontAwesome6.h"

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

	ImGui::End();

	if (glfwGetKey(engine.window.getGLFWwindow(), GLFW_KEY_0)) {
		entt::registry& registry = engine.ecs.registry;
		entt::entity entity = registry.create();

		Transform transform = {
			nearWorldPos,
			{1.f, 1.f, 1.f}
		};

		registry.emplace<Transform>(entity, std::move(transform));
		registry.emplace<EntityData>(entity, EntityData{ "Bullet" });

		std::unordered_map<MaterialName, Material> materials;

		ResourceID modelAsset{ 1245008178564268053 };
		materials["Material"] = { Material::Pipeline::PBR, Color{0.5f, 0.5f, 0.5f} };

		registry.emplace<MeshRenderer>(entity, MeshRenderer{ modelAsset, materials });

		registry.emplace<Rigidbody>(entity, Rigidbody{ JPH::EMotionType::Kinematic, Rigidbody::Layer::Moving, raycastDirection * 100.f, 1.f });
		registry.emplace<BoxCollider>(entity, BoxCollider{});
	}
}
