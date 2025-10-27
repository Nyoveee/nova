#include "ResourceManager/resourceManager.h"
#include "AssetManager/assetManager.h"
#include "IconsFontAwesome6.h"
#include "editor.h"

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
			auto&& [draggedId, name] = *((std::pair<std::size_t, const char*>*)payload->Data);
			
			if (resourceManager.isResource<T>(draggedId)) {
				onClickCallback(draggedId);
			}
		}
	}

	if (id) {
		ImGui::SameLine();

		if (ImGui::Button(ICON_FA_ANCHOR_CIRCLE_XMARK)) {
			assetViewerUi.selectNewResourceId(id.value());
		}

		ImGui::PopID();
	}
}

template<IsEnum T>
void Editor::displayEnumDropDownList(T value, const char* labelName, std::function<void(T)> onClickCallback) {
	// get the list of all possible enum values
	constexpr auto listOfEnumValues = magic_enum::enum_entries<T>();

	if (ImGui::BeginCombo(labelName, std::string{ magic_enum::enum_name<T>(value) }.c_str())) {

		for (auto&& [enumValue, enumInString] : listOfEnumValues) {
			if (ImGui::Selectable(std::string{ enumInString }.c_str(), enumValue == value)) {
				onClickCallback(enumValue);
			}
		}

		ImGui::EndCombo();
	}
}
