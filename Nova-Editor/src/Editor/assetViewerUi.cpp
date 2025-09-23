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

	ImGui::InputText("Name: ",		&descriptorPtr->name);
	ImGui::InputText(selectedResourceExtension.empty() ? "##" : selectedResourceExtension.c_str(), &selectedResourceStemCopy);

	if (ImGui::IsItemDeactivatedAfterEdit() && std::filesystem::path{ descriptorPtr->filepath }.stem() != selectedResourceStemCopy) {
		if (!assetManager.renameFile(selectedResourceId, selectedResourceStemCopy)) {
			// reset rename attempt.
			selectedResourceStemCopy = std::filesystem::path{ descriptorPtr->filepath }.stem().string();
		}
	}

	ImGui::End();
}

void AssetViewerUI::selectNewResourceId(ResourceID id) {
	selectedResourceId = id;

	auto descriptorPtr = assetManager.getDescriptor(selectedResourceId);

	if (!descriptorPtr) {
		return;
	}

	selectedResourceStemCopy = std::filesystem::path{ descriptorPtr->filepath }.stem().string();
	selectedResourceExtension = std::filesystem::path{ descriptorPtr->filepath }.extension().string();
}
