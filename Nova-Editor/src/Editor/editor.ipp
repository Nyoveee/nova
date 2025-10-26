#include "ResourceManager/resourceManager.h"
#include "AssetManager/assetManager.h"


template<typename T>
void Editor::displayAssetDropDownList(std::optional<ResourceID> id, const char* labelName, std::function<void(ResourceID)> onClickCallback) {
	char const* selectedAssetName = "";

	if (id) {
		auto namePtr = assetManager.getName(id.value());
		selectedAssetName = namePtr ? namePtr->c_str() : "No resource selected.";
		ImGui::PushID(static_cast<int>(static_cast<std::size_t>(id.value())));
	}

	auto allResources = resourceManager.getAllResources<T>();

	if (ImGui::BeginCombo(labelName, selectedAssetName)) {
		for (auto&& resourceId : allResources) {
			std::string const* assetName = assetManager.getName(resourceId);

			if (!assetName) {
				continue;
			}

			ImGui::PushID(static_cast<int>(static_cast<std::size_t>(resourceId)));

			if (ImGui::Selectable(assetName->empty() ? "<no name>" : assetName->c_str(), id ? id.value() == resourceId : false)) {
				onClickCallback(resourceId);
			}

			ImGui::PopID();

		}

		ImGui::EndCombo();
	}

	// handle drag and drop..
	if (ImGui::BeginDragDropTarget()) {
		if (ImGuiPayload const* payload = ImGui::AcceptDragDropPayload("DRAGGING_ASSET_ITEM")) {
			auto&& [id, name] = *((std::pair<std::size_t, const char*>*)payload->Data);
			
			if (resourceManager.isResource<T>(id)) {
				onClickCallback(id);
			}
		}
	}

	if (id) {
		ImGui::PopID();
	}
}