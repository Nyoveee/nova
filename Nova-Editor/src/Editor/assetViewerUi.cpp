#include <iterator> 
#include <regex>

#include "assetViewerUi.h"

#include "imgui.h"
#include "IconsFontAwesome6.h"

#include "misc/cpp/imgui_stdlib.h"

AssetViewerUI::AssetViewerUI(AssetManager& assetManager, ResourceManager& resourceManager) :
	assetManager		{ assetManager },
	resourceManager		{ resourceManager },
	selectedResourceId	{ INVALID_RESOURCE_ID }
{}

void AssetViewerUI::update() {
	ImGui::Begin(ICON_FA_AUDIO_DESCRIPTION " Asset Viewer");

	if (selectedResourceId == INVALID_RESOURCE_ID) {
		ImGui::Text("No resource selected.");
		ImGui::End();
		return;
	}

	auto descriptorPtr = assetManager.getDescriptor(selectedResourceId);

	if (!descriptorPtr) {
		ImGui::Text("Invalid resource id selected.");
		ImGui::End();
		return;
	}

	ImGui::Text("Resource ID: %zu", static_cast<std::size_t>(descriptorPtr->id));

	ImGui::InputText("Name: ", &selectedResourceName);

	if (ImGui::IsItemDeactivatedAfterEdit()) {
		// empty name is not a valid name.
		if (selectedResourceName == "") {
			selectedResourceName = descriptorPtr->name;
		}
		else {
			descriptorPtr->name = selectedResourceName;

			// we recompile scripts when there is a name change..
			if (resourceManager.isResource<ScriptAsset>(selectedResourceId)) {
				updateScriptFileName(descriptorPtr->filepath, selectedResourceName, selectedResourceId);
			}
		}
	}

	ImGui::Text("Filepath: %s", selectedResourceStemCopy.c_str());

#if 0
	ImGui::InputText(selectedResourceExtension.empty() ? "##" : selectedResourceExtension.c_str(), &selectedResourceStemCopy);

	if (ImGui::IsItemDeactivatedAfterEdit() && std::filesystem::path{ descriptorPtr->filepath }.stem() != selectedResourceStemCopy) {
		if (!assetManager.renameFile(selectedResourceId, selectedResourceStemCopy)) {
			// reset rename attempt.
			selectedResourceStemCopy = std::filesystem::path{ descriptorPtr->filepath }.stem().string();
		}
	}
#endif

	ImGui::End();
}

void AssetViewerUI::updateScriptFileName(AssetFilePath const& filepath, std::string const& newName, ResourceID id) {
	std::ifstream inputScriptFile{ filepath };

	if (!inputScriptFile) {
		Logger::error("Fail to read script file.");
		return;
	}

	// we read the whole file's content into a variable
	std::string scriptContents{ std::istreambuf_iterator<char>(inputScriptFile), std::istreambuf_iterator<char>() };

	// i love regex. ask me if u need regex explaination.
	std::regex classRegex(R"(class [\w\s]+\s?:)");
	scriptContents = std::regex_replace(scriptContents, classRegex, "class " + newName + " :");
	
	std::ofstream outputScriptFile{ filepath };

	if (!outputScriptFile) {
		Logger::error("Fail to open script file for writing.");
		return;
	}

	outputScriptFile << scriptContents;

	Logger::info("Succesfully updated script name.");

	// we need to serialise the descriptor for recompilation later, script resource will hold the updated name.
	assetManager.serialiseDescriptor<ScriptAsset>(id);
}

void AssetViewerUI::selectNewResourceId(ResourceID id) {
	selectedResourceId = id;

	auto descriptorPtr = assetManager.getDescriptor(selectedResourceId);

	if (!descriptorPtr) {
		return;
	}

	selectedResourceName = descriptorPtr->name;
	selectedResourceStemCopy = std::filesystem::path{ descriptorPtr->filepath }.filename().string();
	selectedResourceExtension = std::filesystem::path{ descriptorPtr->filepath }.extension().string();
}
