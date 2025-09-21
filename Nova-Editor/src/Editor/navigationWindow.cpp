#include "imgui.h"

#include "editor.h"

#include "navigationWindow.h"
#include "Navigation/Navigation.h"
#include "Navigation/navMeshGeneration.h"

#include "misc/cpp/imgui_stdlib.h"

NavigationWindow::NavigationWindow(Editor& editor, NavigationSystem& navigationSystem, NavMeshGeneration& navMeshGenerator) :
	editor				{ editor },
	navigationSystem	{ navigationSystem },
	navMeshGenerator	{ navMeshGenerator },
	filename			{ "mySkinIsGleaming "}
{}

void NavigationWindow::update() {
	BuildSettings& buildSettings = navMeshGenerator.GetBuildSettings();

	ImGui::Begin("Navigation");

	//One block of nice GUI  = 6 lines GG
	ImGui::Dummy(ImVec2(10.0f, 0.0f));  // 10px horizontal padding
	ImGui::SameLine();
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Agent Name:");
	ImGui::SameLine();
	ImGui::InputText("##Agent Name:", &buildSettings.agentName);


	//--
	ImGui::Dummy(ImVec2(10.0f, 0.0f));
	ImGui::SameLine();
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Agent Radius:");
	ImGui::SameLine();
	ImGui::InputFloat("##Agent Radius", &buildSettings.agentRadius, 0.0f, 0.0f, "%.2f");

	//---
	ImGui::Dummy(ImVec2(10.0f, 0.0f));
	ImGui::SameLine();
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Agent Height:");
	ImGui::SameLine();
	ImGui::InputFloat("##Agent Height", &buildSettings.agentHeight, 0.0f, 0.0f, "%.2f");

	//--
	ImGui::Dummy(ImVec2(10.0f, 0.0f));
	ImGui::SameLine();
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Agent Max Climb:");
	ImGui::SameLine();
	ImGui::InputFloat("##Agent Max Climb:", &buildSettings.agentMaxClimb, 0.0f, 0.0f, "%.2f");


	//--
	ImGui::Dummy(ImVec2(10.0f, 0.0f));
	ImGui::SameLine();
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Agent Max Slope:");
	ImGui::SameLine();
	ImGui::SliderFloat("##Agent Max Slope:", &buildSettings.agentMaxSlope, 0.0f, 89.9f, "%.2f");

	// Specify filename when baking (TEMP)
	ImGui::InputText("Filename:", &filename);

	// TEMPORARY..
	editor.displayAssetDropDownList<NavMesh>(navigationSystem.getNavMeshId(), "Select nav mesh..", [&](ResourceID newNavMeshId) {
		navigationSystem.setNewNavMesh(newNavMeshId);
	});

	if (ImGui::Button("Unselect")) {
		navigationSystem.setNewNavMesh(INVALID_RESOURCE_ID);
	}

	//-- imgui buttons Reset and Set for now
	ImGui::Dummy(ImVec2(00.0f, 20.0f));

	float windowWidth = ImGui::GetWindowContentRegionMax().x;
	ImGui::SetCursorPosX(windowWidth - 220.0f);
	if (ImGui::Button("Reset", ImVec2(100, 40))) { navMeshGenerator.ResetBuildSetting(); };
	ImGui::SameLine();
	if (ImGui::Button("Bake", ImVec2(100, 40))) { navMeshGenerator.BuildNavMesh(filename); };

	//TO DO --- WORK ON DROP OFF AND JUMP HEIGHT IN M2

	ImGui::End();
}
