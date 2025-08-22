#include "assetManager.h"
#include "editor.h"

#include "assetManagerUi.h"
#include "imgui.h"
#include "IconsFontAwesome6.h"

#include "ImGui/misc/cpp/imgui_stdlib.h"

AssetManagerUI::AssetManagerUI(Editor& editor) :
	assetManager	 { editor.assetManager },
	selectedFolderId { NONE },
	folderIcon		 { "System/Image/folder.png", false }
{
	folderIcon.load(assetManager);
}

void AssetManagerUI::update() {
	ImGui::Begin(ICON_FA_BOXES_PACKING " Content Browser");

	displayLeftNavigationPanel();
	ImGui::SameLine(); 
	displayRightContentPanel();

	ImGui::End();
}

void AssetManagerUI::displayLeftNavigationPanel() {
	ImGui::BeginChild("(Left) Navigation Panel", ImVec2(200, 0), true);
	bool toShow = ImGui::TreeNodeEx("Content", ImGuiTreeNodeFlags_DefaultOpen);

	// Seperator has some inbuilt indentable so let's unindent it for a border effect.
	ImGui::Unindent(20.f);
	ImGui::Separator();
	ImGui::Indent(20.f);

	if (toShow) {
		// Remove indentation temporarily
		ImGui::Unindent(20.f);

		for (FolderID folderId : assetManager.getRootDirectories()) {
			displayFolderTreeNode(folderId);
		}

		ImGui::Indent(20.f);
		ImGui::TreePop();
	}

	ImGui::EndChild();
}

void AssetManagerUI::displayRightContentPanel() {
	ImGui::BeginChild("(Right) Content Browser");
	ImGui::InputText("search", &searchQuery);
	
	// ==== get upper case version of the search query. ===
	allUpperCaseSearchQuery.clear();
	allUpperCaseSearchQuery.reserve(searchQuery.size());

	for (char& character : searchQuery) {
		allUpperCaseSearchQuery.push_back(static_cast<char>(std::toupper(character)));
	}
	// ====================================================
	
	if (selectedFolderId == NONE) {
		ImGui::Text("No folder selected.");
	}
	else {
		displaySelectedFolderRelativePath();

		ImGui::BeginChild("(Main) Content Browser", ImVec2(0, 0), true);
		displayFolderContent(selectedFolderId);
		ImGui::EndChild();
	}

	ImGui::EndChild();
}

void AssetManagerUI::displaySelectedFolderRelativePath() {
	displayClickableFolderPath(selectedFolderId, false);
}

void AssetManagerUI::displayClickableFolderPath(FolderID folderId, bool toDisplayCaret) {
	auto iterator = assetManager.getDirectories().find(folderId);
	assert(iterator != assetManager.getDirectories().end() && "this should never happen. selecting an invalid folder.");

	auto&& [_, folder] = *iterator;

	ImGui::PushID(static_cast<int>(static_cast<std::size_t>(folderId)));

	std::string folderName = folder.name;

	if (folder.parent != NONE) {
		displayClickableFolderPath(folder.parent, true);
	}
	else {
		folderName = "  " + folderName;
	}

	if (ImGui::Selectable(folderName.c_str(), false, ImGuiSelectableFlags_None, ImGui::CalcTextSize(folderName.c_str()))) {
		selectedFolderId = folderId;
	}

	ImGui::PopID();

	if (toDisplayCaret) {
		ImGui::SameLine();
		ImGui::Text("> ");
		ImGui::SameLine();
	}
}

void AssetManagerUI::displayFolderTreeNode(FolderID folderId) {
	auto iterator = assetManager.getDirectories().find(folderId);
	assert(iterator != assetManager.getDirectories().end() && "this should never happen. root directories contain folder id to invalid directory.");

	auto&& [_, folder] = *iterator;

	// Display children recursively..
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_DrawLinesToNodes;

	if (selectedFolderId == folderId) {
		flags |= ImGuiTreeNodeFlags_Selected;
	}

	if (folder.childDirectories.empty()) {
		flags |= ImGuiTreeNodeFlags_Leaf;
		ImGui::Unindent(20.f);
	}

	bool showTreeNode = ImGui::TreeNodeEx(folder.name.c_str(), flags);

	if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
		selectedFolderId = folderId;
	}

	if (showTreeNode) {
		// display children recursively..
		for (FolderID childDirectoryId : folder.childDirectories) {
			displayFolderTreeNode(childDirectoryId);
		}

		ImGui::TreePop();
	}

	if (folder.childDirectories.empty()) {
		ImGui::Indent(20.f);
	}
}

void AssetManagerUI::displayFolderContent(FolderID folderId) {
	// This displays a table of asset content
	// We need to calculate the number of columns based on a fixed size of the asset thumbnail.

	auto iterator = assetManager.getDirectories().find(folderId);
	assert(iterator != assetManager.getDirectories().end() && "this should never happen. attempting to display invalid folder id.");

	auto&& [_, folder] = *iterator;

	float windowWidth = ImGui::GetContentRegionMax().x - ImGui::GetStyle().WindowPadding.x * 2;
	int numOfColumns = std::max(1, static_cast<int>(windowWidth / columnWidth));

	// Start of the content table..
	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, 0));

	int itemsToDisplay = 0;

	if (ImGui::BeginTable("Main Content Table", numOfColumns)) {
		// Set fixed column widths..
		for (int i = 0; i < numOfColumns; ++i) {
			ImGui::TableSetupColumn(std::string{"Column" + std::to_string(i)}.c_str(), ImGuiTableColumnFlags_WidthFixed, columnWidth);
		}

		// Display all folder thumbnails..
		for (FolderID childFolderId: folder.childDirectories) {
			displayFolderThumbnail(childFolderId);
			++itemsToDisplay;
		}

		// Display all asset thumbnails..
		for (AssetID assetId : folder.assets) {
			displayAssetThumbnail(assetId);
			++itemsToDisplay;
		}

		ImGui::EndTable();
	}

	ImGui::PopStyleVar();

	if (!itemsToDisplay) {
		ImGui::Text("This folder is empty.");
	}
}

void AssetManagerUI::displayAssetThumbnail(AssetID assetId) {
	Asset* asset = assetManager.getAssetInfo(assetId);
	
	if (!asset) {
		return;
	}

	displayThumbnail(
		static_cast<int>(static_cast<std::size_t>(assetId)),
		NO_TEXTURE,
		asset->name.empty() ? "<no name>" : asset->name.c_str(),

		// callback when the thumbnail gets clicked.
		[&]() {

		}
	);
}

void AssetManagerUI::displayFolderThumbnail(FolderID folderId) {
	auto iterator = assetManager.getDirectories().find(folderId);
	assert(iterator != assetManager.getDirectories().end() && "this should never happen. attempting to display invalid folder id.");

	auto&& [_, folder] = *iterator;

	displayThumbnail(
		static_cast<int>(static_cast<std::size_t>(folderId)),
		folderIcon.getTextureId(),
		folder.name.c_str(),
		
		// callback when the thumbnail gets clicked.
		[&]() {
			selectedFolderId = folderId;
		}
	);
}

void AssetManagerUI::displayThumbnail(int imguiId, ImTextureID thumbnail, char const* name, std::function<void()> clickCallback) {
	if (!isAMatchWithSearchQuery(name)) {
		return;
	}

	ImGui::TableNextColumn();

	ImVec2 padding = ImGui::GetStyle().WindowPadding;
	constexpr float textHeight = 20.f;

	ImGui::PushID(imguiId);
	ImGui::BeginChild("Thumbnail", ImVec2{ columnWidth, columnWidth + textHeight + 2 * padding.y }, ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar);

	ImVec2 buttonSize = ImVec2{ columnWidth - 2 * padding.x, columnWidth - 2 * padding.x };

	if (thumbnail != NO_TEXTURE) {
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 0 ,0 });
		
		if (ImGui::ImageButton("##", thumbnail, buttonSize)) {
			clickCallback();
		}

		ImGui::PopStyleVar();
	}
	else {
		if (ImGui::Button("##", buttonSize)) {
			clickCallback();
		}
	}

	ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + columnWidth - 2 * padding.x);
	ImGui::Text(name);
	ImGui::PopTextWrapPos();

	ImGui::EndChild();
	ImGui::PopID();
}

bool AssetManagerUI::isAMatchWithSearchQuery(std::string const& name) const {
	if (allUpperCaseSearchQuery.empty()) {
		return true;
	}

	std::string allUpperCaseName;
	allUpperCaseName.reserve(name.size());

	for (char const& character : name) {
		allUpperCaseName.push_back(static_cast<char>(std::toupper(character)));
	}
	return allUpperCaseName.find(allUpperCaseSearchQuery) != std::string::npos;
}
