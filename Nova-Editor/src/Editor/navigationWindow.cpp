#include "imgui.h"
#include <fstream>
#include "editor.h"

#include "navigationWindow.h"
#include "Navigation/Navigation.h"
#include "Navigation/navMeshGeneration.h"

#include "misc/cpp/imgui_stdlib.h"
#include "Engine/engine.h"

//using json = nlohmann::ordered_json;

NavigationWindow::NavigationWindow(Editor& editor, NavigationSystem& navigationSystem, NavMeshGeneration& navMeshGenerator) :
	editor				{ editor },
	navigationSystem	{ navigationSystem },
	navMeshGenerator	{ navMeshGenerator },
	editorConfigUI      { editor.editorConfigUI },
	onFileCreate		{ false },
	step				{ 0 },
	filename			{ "Navmesh" }
{

#include <filesystem>
	//// ...
	//std::cout << "Current path: " << std::filesystem::current_path() << std::endl;
	//Logger::info("Looking for config at: {}", std::filesystem::absolute(editorConfigUI.getPath()).string());

	try {

		//Note: i ran into an issue where navigationw window is intialised before
		//editorUI config resulting in me unable to retrive file path so i am
		//doing this instead
		std::ifstream file(saveFilePath);
		if (file.is_open()) {
			config = json::parse(file);

			if (!config.contains("Agents") || !config["Agents"].is_array()) {
				config["Agents"] = json::array();
			}

			BuildSettings& buildSettings = navMeshGenerator.GetBuildSettings();
			if (config["Agents"].empty()) {
				json defaultAgent = {
					{"Agent Name", buildSettings.agentName},
					{"Radius",  buildSettings.agentRadius},
					{"Height", buildSettings.agentHeight},
					{"MaxClimb", buildSettings.agentMaxClimb},
					{"MaxSlope", buildSettings.agentMaxSlope}
				};

				config["Agents"].push_back(defaultAgent);

				std::ofstream outFile(saveFilePath);
				if (outFile.is_open()) {
					outFile << config.dump(4);
				}
			}


		}
	}
	catch (const json::parse_error& e) {
		// Handle syntax errors in the JSON file
		Logger::error("JSON Parse Error: {}", e.what());
	}


}

NavigationWindow::~NavigationWindow()
{
	std::ofstream outFile(saveFilePath);
	outFile << config.dump(4);

}

void NavigationWindow::update() {
	BuildSettings& buildSettings = navMeshGenerator.GetBuildSettings();

	//ImGui::Begin("Navigation");

	//if (editor.isInSimulationMode()) {
	//	ImGui::BeginDisabled();
	//}

	////One block of nice GUI  = 6 lines GG
	//ImGui::Dummy(ImVec2(10.0f, 0.0f));  // 10px horizontal padding
	//ImGui::SameLine();
	//ImGui::AlignTextToFramePadding();
	//ImGui::Text("Agent Name:");
	//ImGui::SameLine();
	//ImGui::InputText("##Agent Name:", &buildSettings.agentName);


	////--
	//ImGui::Dummy(ImVec2(10.0f, 0.0f));
	//ImGui::SameLine();
	//ImGui::AlignTextToFramePadding();
	//ImGui::Text("Agent Radius:");
	//ImGui::SameLine();
	//ImGui::InputFloat("##Agent Radius", &buildSettings.agentRadius, 0.0f, 0.0f, "%.2f");

	////---
	//ImGui::Dummy(ImVec2(10.0f, 0.0f));
	//ImGui::SameLine();
	//ImGui::AlignTextToFramePadding();
	//ImGui::Text("Agent Height:");
	//ImGui::SameLine();
	//ImGui::InputFloat("##Agent Height", &buildSettings.agentHeight, 0.0f, 0.0f, "%.2f");

	////--
	//ImGui::Dummy(ImVec2(10.0f, 0.0f));
	//ImGui::SameLine();
	//ImGui::AlignTextToFramePadding();
	//ImGui::Text("Agent Max Climb:");
	//ImGui::SameLine();
	//ImGui::InputFloat("##Agent Max Climb:", &buildSettings.agentMaxClimb, 0.0f, 0.0f, "%.2f");


	////--
	//ImGui::Dummy(ImVec2(10.0f, 0.0f));
	//ImGui::SameLine();
	//ImGui::AlignTextToFramePadding();
	//ImGui::Text("Agent Max Slope:");
	//ImGui::SameLine();
	//ImGui::SliderFloat("##Agent Max Slope:", &buildSettings.agentMaxSlope, 0.0f, 89.9f, "%.2f");


	//BuildSettings& buildSettings = navMeshGenerator.GetBuildSettings();

	ImGui::Begin("Navigation");

	if (editor.isInSimulationMode()) {
		ImGui::BeginDisabled();
	}

	ImGui::Dummy(ImVec2(10.0f, 0.0f));
	ImGui::SameLine();
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Select Agent:");
	ImGui::SameLine();

	// --- Agent Selector Combo ---
	std::vector<std::string> agentNames;
	for (auto& agent : config["Agents"]) {
		agentNames.push_back(agent.value("Agent Name", "Unnamed Agent"));
	}

	float comboWidth = ImGui::GetContentRegionAvail().x - 100.0f;
	ImGui::SetNextItemWidth(comboWidth);

	// Ensure selected index is valid
	if (selectedAgentIndex >= (int)agentNames.size()) selectedAgentIndex = 0;
	if (selectedAgentIndex < 0) selectedAgentIndex = 0;

	std::string currentAgentName = agentNames.empty() ? "None" : agentNames[selectedAgentIndex];
	if (ImGui::BeginCombo("##AgentSelector", currentAgentName.c_str())) {
		for (int i = 0; i < (int)agentNames.size(); i++) {
			bool isSelected = (selectedAgentIndex == i);
			if (ImGui::Selectable(agentNames[i].c_str(), isSelected)) {
				selectedAgentIndex = i;

				// Update BuildSettings immediately upon selection
				auto& agent = config["Agents"][i];
				buildSettings.agentName = agent.value("Agent Name", "");
				buildSettings.agentRadius = agent.value("Radius", 0.6f);
				buildSettings.agentHeight = agent.value("Height", 2.0f);
				buildSettings.agentMaxClimb = agent.value("MaxClimb", 0.9f);
				buildSettings.agentMaxSlope = agent.value("MaxSlope", 45.0f);
			}
			if (isSelected) ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	ImGui::SameLine();

	// --- Add Agent Button ---
	if (ImGui::Button("+", ImVec2(30, 0))) {
		// Prepare for popup
		newAgentNameBuf = "New Agent";
		openAddAgentPopup = true;
	}

	// --- Add Agent Popup Modal ---
	if (openAddAgentPopup) {
		ImGui::OpenPopup("Add New Agent");
		openAddAgentPopup = false; // Reset trigger
	}



	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

	if (ImGui::BeginPopupModal("Add New Agent", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("Enter name for the new agent:");
		ImGui::InputText("##NewAgentName", &newAgentNameBuf);

		// Check for duplicate name
		bool isDuplicate = false;
		for (const auto& name : agentNames) {
			if (name == newAgentNameBuf) {
				isDuplicate = true;
				break;
			}
		}

		if (isDuplicate) {
			ImGui::TextColored(ImVec4(1, 0, 0, 1), "Name already exists!");
		}

		ImGui::Separator();

		if (ImGui::Button("Create", ImVec2(120, 0)) && !isDuplicate && !newAgentNameBuf.empty()) {
			// Create new agent with default values
			json newAgent = {
				{"Agent Name", newAgentNameBuf},
				{"Radius", 0.6f},
				{"Height", 2.0f},
				{"MaxClimb", 0.9f},
				{"MaxSlope", 45.0f}
			};

			config["Agents"].push_back(newAgent);
			selectedAgentIndex = (int)config["Agents"].size() - 1;

			// Immediately sync to build settings
			buildSettings.agentName = newAgentNameBuf;
			buildSettings.agentRadius = 0.6f;
			buildSettings.agentHeight = 2.0f;
			buildSettings.agentMaxClimb = 0.9f;
			buildSettings.agentMaxSlope = 45.0f;

			// Save to disk
			std::ofstream outFile(saveFilePath);
			outFile << config.dump(4);

			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	ImGui::PopStyleColor(2);

	ImGui::SameLine();

	// --- Remove Agent Button ---
	ImGui::BeginDisabled(config["Agents"].size() <= 1); // Prevent deleting the last remaining agent if you wish
	if (ImGui::Button("-", ImVec2(30, 0))) {
		if (!config["Agents"].empty()) {
			// Remove current
			config["Agents"].erase(config["Agents"].begin() + selectedAgentIndex);

			// Decrement index to show the previous one, or 0 if we were at the start
			if (selectedAgentIndex > 0) {
				selectedAgentIndex--;
			}
			else {
				selectedAgentIndex = 0;
			}

			// If there are still agents left, update the UI to show the new selection
			if (!config["Agents"].empty()) {
				auto& agent = config["Agents"][selectedAgentIndex];
				buildSettings.agentName = agent.value("Agent Name", "");
				buildSettings.agentRadius = agent.value("Radius", 0.6f);
				buildSettings.agentHeight = agent.value("Height", 2.0f);
				buildSettings.agentMaxClimb = agent.value("MaxClimb", 0.9f);
				buildSettings.agentMaxSlope = agent.value("MaxSlope", 45.0f);
			}

			std::ofstream outFile(saveFilePath);
			outFile << config.dump(4);
		}
	}
	ImGui::EndDisabled();

	ImGui::Separator();

	// --- Property Editors ---
	if (!config["Agents"].empty()) {
		bool edited = false;

		ImGui::Dummy(ImVec2(10.0f, 0.0f));
		ImGui::SameLine();
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Agent Name:");
		ImGui::SameLine();

		// Optional: Allow renaming existing agents (with duplicate check if needed, simplified here)
		if (ImGui::InputText("##EditAgentName", &buildSettings.agentName)) {
			// Basic rename without complex validation for current fields
			config["Agents"][selectedAgentIndex]["Agent Name"] = buildSettings.agentName;
			edited = true;
		}

		// Radius
		ImGui::Dummy(ImVec2(10.0f, 0.0f));
		ImGui::SameLine();
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Agent Radius:");
		ImGui::SameLine();
		if (ImGui::InputFloat("##Agent Radius", &buildSettings.agentRadius, 0.0f, 0.0f, "%.2f")) {
			config["Agents"][selectedAgentIndex]["Radius"] = buildSettings.agentRadius;
			edited = true;
		}

		// Height
		ImGui::Dummy(ImVec2(10.0f, 0.0f));
		ImGui::SameLine();
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Agent Height:");
		ImGui::SameLine();
		if (ImGui::InputFloat("##Agent Height", &buildSettings.agentHeight, 0.0f, 0.0f, "%.2f")) {
			config["Agents"][selectedAgentIndex]["Height"] = buildSettings.agentHeight;
			edited = true;
		}

		// Max Climb
		ImGui::Dummy(ImVec2(10.0f, 0.0f));
		ImGui::SameLine();
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Agent Max Climb:");
		ImGui::SameLine();
		if (ImGui::InputFloat("##Agent Max Climb:", &buildSettings.agentMaxClimb, 0.0f, 0.0f, "%.2f")) {
			config["Agents"][selectedAgentIndex]["MaxClimb"] = buildSettings.agentMaxClimb;
			edited = true;
		}

		// Max Slope
		ImGui::Dummy(ImVec2(10.0f, 0.0f));
		ImGui::SameLine();
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Agent Max Slope:");
		ImGui::SameLine();
		if (ImGui::SliderFloat("##Agent Max Slope:", &buildSettings.agentMaxSlope, 0.0f, 89.9f, "%.2f")) {
			config["Agents"][selectedAgentIndex]["MaxSlope"] = buildSettings.agentMaxSlope;
			edited = true;
		}

		// Auto-save on specific edit end
		if (edited) {
			// Optional: save immediately or wait for deactivation
		}
		if (ImGui::IsItemDeactivatedAfterEdit()) {
			std::ofstream outFile(saveFilePath);
			outFile << config.dump(4);
		}
	}

	ImGui::Separator();
	ImGui::InputText("Filename:", &filename);

	//-- Buttons
	ImGui::Dummy(ImVec2(00.0f, 20.0f));
	float windowWidth = ImGui::GetWindowContentRegionMax().x;
	ImGui::SetCursorPosX(windowWidth - 220.0f);

	if (ImGui::Button("Reset", ImVec2(100, 40))) { navMeshGenerator.ResetBuildSetting(); };
	ImGui::SameLine();
	if (ImGui::Button("Bake", ImVec2(100, 40))) {
		navMeshGenerator.BuildNavMesh(filename);
		onFileCreate = true;
		step = 0;
	};

	// File creation step logic...
	if (onFileCreate) {
		if (step >= 60) {
			navMeshGenerator.AddNavMeshSurface(filename);
			step = 0;
			onFileCreate = false;
		}
		else {
			step++;
		}
	}

	if (editor.isInSimulationMode()) {
		ImGui::EndDisabled();
	}

	ImGui::End();
}

