#include "AssetManager/assetManager.h"
#include "ResourceManager/resourceManager.h"
#include "Editor/editor.h"

#include "assetManagerUi.h"
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include "assetViewerUi.h"

#include "../ImGui/misc/cpp/imgui_stdlib.h"

#include "Serialisation/serialisation.h"

#include <sstream>
#include <Windows.h>
#include "Engine/engine.h"

#undef max

AssetManagerUI::AssetManagerUI(Editor& editor, AssetViewerUI& assetViewerUi) :
	editor			 { editor },
	assetManager	 { editor.assetManager },
	resourceManager	 { editor.resourceManager },
	assetViewerUi	 { assetViewerUi },
	selectedFolderId { ASSET_FOLDER },
	folderIcon		 { nullptr }
{
	auto folderPtr = ResourceLoader<Texture>::load(INVALID_RESOURCE_ID,		std::string{ "System/Image/folder" }).value()();
	folderIcon.reset(static_cast<Texture*>(folderPtr.release()));

	auto texturePtr = ResourceLoader<Texture>::load(INVALID_RESOURCE_ID,	std::string{ "System/Image/texture" }).value()();
	textureIcon.reset(static_cast<Texture*>(texturePtr.release()));

	auto audioPtr = ResourceLoader<Texture>::load(INVALID_RESOURCE_ID,		std::string{ "System/Image/audio" }).value()();
	audioIcon.reset(static_cast<Texture*>(audioPtr.release()));

	auto scriptPtr = ResourceLoader<Texture>::load(INVALID_RESOURCE_ID,		std::string{ "System/Image/script" }).value()();
	scriptIcon.reset(static_cast<Texture*>(scriptPtr.release()));

	auto scenePtr = ResourceLoader<Texture>::load(INVALID_RESOURCE_ID,		std::string{ "System/Image/scene" }).value()();
	sceneIcon.reset(static_cast<Texture*>(scenePtr.release()));

	auto modelPtr = ResourceLoader<Texture>::load(INVALID_RESOURCE_ID,		std::string{ "System/Image/model" }).value()();
	modelIcon.reset(static_cast<Texture*>(modelPtr.release()));

	auto cubeMapPtr = ResourceLoader<Texture>::load(INVALID_RESOURCE_ID,	std::string{ "System/Image/cubemap" }).value()();
	cubeMapIcon.reset(static_cast<Texture*>(cubeMapPtr.release()));
}

void AssetManagerUI::update() {
	ImGui::Begin(ICON_FA_BOXES_PACKING " Content Browser");

	displayLeftNavigationPanel();
	ImGui::SameLine(); 
	displayRightContentPanel();

	if (ImGui::BeginDragDropTarget()) {
		if (ImGuiPayload const* payload = ImGui::AcceptDragDropPayload("HIERARCHY_ITEM")) {
			entt::entity entity = *((entt::entity*)payload->Data);
			EntityData* entityData = editor.engine.ecs.registry.try_get<EntityData>(entity);
			std::string fileName = entityData->name;
			std::optional<std::ofstream> opt = createAssetFile(".prefab");

			if (opt != std::nullopt) {
				Serialiser::serialisePrefab(editor.engine.ecs.registry, entity, opt.value());
			}


			//std::unordered_map<FolderID, Folder> directories = assetManager.getDirectories();
			//Folder folder = directories[selectedFolderId];

			//ResourceID id;
			//for (auto iterator = folder.assets.begin(); iterator != folder.assets.end(); iterator++) {
			//	id = *iterator;
			//}
			
			//editor.engine.prefabManager.convertToPrefab(entity, id);
		}

		ImGui::EndDragDropTarget();

	}

	ImGui::End();
	
}

void AssetManagerUI::displayLeftNavigationPanel() {
	ImGui::BeginChild("(Left) Navigation Panel", ImVec2(200, 0), ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX);
	ImGui::Text("Content");
	
	// Seperator has some inbuilt indentable so let's unindent it for a border effect.
	ImGui::Unindent(20.f);
	ImGui::Separator();
	ImGui::Indent(20.f);

	// Remove indentation temporarily
	ImGui::Unindent(20.f);

	displayFolderTreeNode(ASSET_FOLDER);

	ImGui::Indent(20.f);

	ImGui::EndChild();
}

void AssetManagerUI::displayRightContentPanel() {
	ImGui::BeginChild("(Right) Content Browser");
	ImGui::InputText("search", &searchQuery);
	
	ImGui::SameLine();

	if (ImGui::Button("Reload")) {
		assetManager.reload();
	}

	// ==== get upper case version of the search query. ===
	allUpperCaseSearchQuery.clear();
	allUpperCaseSearchQuery.reserve(searchQuery.size());

	for (char& character : searchQuery) {
		allUpperCaseSearchQuery.push_back(static_cast<char>(std::toupper(character)));
	}
	// ====================================================
	
	displaySelectedFolderRelativePath();

	ImGui::BeginChild("(Main) Content Browser", ImVec2(0, 0), true);
	displayFolderContent(selectedFolderId);
	ImGui::EndChild();
	
	displayCreateAssetContextMenu();

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

	if (folderId != ASSET_FOLDER) {
		displayClickableFolderPath(folder.parent, true);
	}
	else {
		folderName = "  " + folderName;
	}

	if (ImGui::Selectable(folderName.c_str(), false, ImGuiSelectableFlags_None, ImGui::CalcTextSize(folderName.c_str()))) {
		selectedFolderId = folderId;
	}

	handleAssetMoveToFolder(folderId);

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
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DrawLinesToNodes;

	if (selectedFolderId == folderId) {
		flags |= ImGuiTreeNodeFlags_Selected;
	}
	if (folder.childDirectories.empty()) {
		flags |= ImGuiTreeNodeFlags_Leaf;	
	}

	bool showTreeNode = ImGui::TreeNodeEx((ICON_FA_FOLDER + std::string{ " " } + folder.name).c_str(), flags);

	if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
		selectedFolderId = folderId;
	}

	handleAssetMoveToFolder(folderId);

	if (showTreeNode) {
		// display children recursively..
		for (FolderID childDirectoryId : folder.childDirectories) {
			displayFolderTreeNode(childDirectoryId);
		}

		ImGui::TreePop();
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
		for (FolderID childFolderId : folder.childDirectories) {
			displayFolderThumbnail(childFolderId);

			++itemsToDisplay;
		}

		// Display all asset thumbnails..
		for (ResourceID assetId : folder.assets) {
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

void AssetManagerUI::displayAssetThumbnail(ResourceID resourceId) {
	std::string const* assetName = assetManager.getName(resourceId);

	if (!assetName) {
		return;
	}

	ImTextureID texture = NO_TEXTURE;

	if (resourceManager.isResource<Texture>(resourceId)) {
		texture = textureIcon->getTextureId();
	}
	else if (resourceManager.isResource<Audio>(resourceId)) {
		texture = audioIcon->getTextureId();
	}
	else if (resourceManager.isResource<ScriptAsset>(resourceId)) {
		texture = scriptIcon->getTextureId();
	}
	else if (resourceManager.isResource<Scene>(resourceId)) {
		texture = sceneIcon->getTextureId();
	}
	else if (resourceManager.isResource<Model>(resourceId)) {
		texture = modelIcon->getTextureId();
	}
	else if (resourceManager.isResource<CubeMap>(resourceId)) {
		texture = cubeMapIcon->getTextureId();
	}

	displayThumbnail(
		static_cast<std::size_t>(resourceId),
		texture,
		assetName->empty() ? "<no name>" : assetName->c_str(),

		// callback when the thumbnail gets clicked.
		[&]() {
			assetViewerUi.selectNewResourceId(resourceId);
		},

		// callback when the thumbnail gets double clicked.
		[&]() {
			handleThumbnailDoubleClick(resourceId);
		},

		// callback to to perform additional logic after displaying context menu..
		[&]() {
			dragAndDrop(assetName->empty() ? "<no name>" : assetName->c_str(), static_cast<std::size_t>(resourceId));
			displayAssetContextMenu(resourceId);
		}
	);

}

void AssetManagerUI::displayFolderThumbnail(FolderID folderId) {
	auto iterator = assetManager.getDirectories().find(folderId);
	assert(iterator != assetManager.getDirectories().end() && "this should never happen. attempting to display invalid folder id.");

	auto&& [_, folder] = *iterator;

	displayThumbnail(
		static_cast<std::size_t>(folderId),
		static_cast<ImTextureID>(folderIcon->getTextureId()),
		folder.name.c_str(),
		
		// callback when the thumbnail gets clicked.
		[&]() {
			selectedFolderId = folderId;
		},

		// callback when the thumbnail gets double clicked.
		[&]() {},

		// callback to run additional logic after thumbnail is rendered..
		[&]() {
			// Accept asset move request into this folder..
			if (ImGui::BeginDragDropTarget()) {
				if (ImGuiPayload const* payload = ImGui::AcceptDragDropPayload("DRAGGING_ASSET_ITEM")) {
					auto&& [id, name] = *((std::pair<std::size_t, const char*>*)payload->Data);
					assetManager.moveAssetToFolder(static_cast<ResourceID>(id), folderId);
				}
				ImGui::EndDragDropTarget();
			}
		}
	);
}

void AssetManagerUI::displayCreateAssetContextMenu() {
	if (ImGui::BeginPopupContextItem("CreateAssetContextMenu")) {
		if (ImGui::MenuItem("[+] Scene")) {
			std::optional<std::ofstream> opt = createAssetFile(".scene");
		
			if (!opt) {
				Logger::error("Failed to create scene file.");
			}
			else {
				std::ofstream& sceneFile = opt.value();
				sceneFile << "{}";
			}
		}
		
#if 1
		if (ImGui::MenuItem("[+] Shader")) {
			std::optional<std::ofstream> opt = createAssetFile(".shader");

			if (!opt) {
				Logger::error("Failed to create shader file.");
			}
			else {
				std::string sampleShaderCode 
#pragma region sampleShaderCode
= R"(
// Specify tags for rendering..
Tags{
    Blending : AlphaBlending;
    DepthTestingMethod : DepthTest;
    Culling : Enable;
}

// Properties for material instances to configure..
Properties{
    sampler2D albedoMap;
    Color colorTint;

    NormalizedFloat roughness;
    NormalizedFloat metallic;
    NormalizedFloat occulusion;
}

// Vertex shader..
Vert{
// ======================================================
// Uncomment this section of the code if you want to use the Color pipeline.
#if 0
    gl_Position = calculateClipPosition(position);
    vsOut.textureUnit = textureUnit; 
#endif

// ======================================================
// Comment this section of the code if you want to use the Color pipeline.
#if 1
    // Calculate world space of our local attributes..
    WorldSpace worldSpace = calculateWorldSpace(position, normal, tangent);
    gl_Position = calculateClipPosition(worldSpace.position);

    // Pass attributes to fragment shader.. //
    vsOut.textureUnit = textureUnit;
    vsOut.fragWorldPos = worldSpace.position.xyz / worldSpace.position.w;
    vsOut.normal = worldSpace.normal;
    vsOut.TBN = calculateTBN(worldSpace.normal, worldSpace.tangent);
#endif
// ======================================================
}

// Fragment shader..
Frag{
// ======================================================
// Uncomment this section of the code if you want to use the Color pipeline.
#if 0
	return vec4(color, 1.0);
#endif

// ======================================================
// Comment this section of the code if you want to use the Color pipeline.
#if 1
    vec4 albedo = texture(albedoMap, fsIn.textureUnit);
    vec3 pbrColor = PBRCaculation(vec3(albedo) * colorTint, fsIn.normal, roughness, metallic, occulusion);
    return vec4(pbrColor, 1.0);
#endif
})";
#pragma endregion sampleShaderCode
				std::ofstream& shaderFile = opt.value();
				shaderFile << sampleShaderCode;
			}
		}
#endif

		if (ImGui::MenuItem("[+] Material")) {
			std::optional<std::ofstream> opt = createAssetFile(".material");

			if (!opt) {
				Logger::error("Failed to create material file.");
			}
			else {
				MaterialData materialData{};
				Serialiser::serializeToJsonFile(materialData, opt.value());
			}
		}

		if (ImGui::MenuItem("[+] Controller")) {
			std::optional<std::ofstream> opt = createAssetFile(".controller");

			if (!opt) {
				Logger::error("Failed to create animation controller file.");
			}
			else {
				Controller::Data data{};
				Serialiser::serializeToJsonFile(data, opt.value());
			}
		}

		if (ImGui::MenuItem("[+] Sequencer")) {
			std::optional<std::ofstream> opt = createAssetFile(".sequencer");

			if (!opt) {
				Logger::error("Failed to create sequencer file.");
			}
			else {
				Sequencer::Data data{};
				Serialiser::serializeToJsonFile(data, opt.value());
			}
		}

		if (ImGui::MenuItem("[+] Script")) {
			static int counter = 0;
			std::string className = "NewScript" + std::to_string(counter++);

			std::optional<std::ofstream> opt = createAssetFile(".cs");

			if (!opt) {
				Logger::error("Failed to create script file.");
			}
			else {
				std::ofstream& scriptFile = opt.value();

				std::string sampleScript =
					"// Make sure the class name matches the filepath, without space!!.\n"
					"// If you want to change class name, change the asset name in the editor!\n"
					"// Editor will automatically rename and recompile this file.\n"
					"class " + className + " : Script\n{\n"
					"    // This function is first invoked when game starts.\n"
					"    protected override void init()\n    {}\n\n"
					"    // This function is invoked every update.\n"
					"    protected override void update()\n    {}\n\n"
					"    // This function is invoked every update.\n"
					"    protected override void fixedUpdate()\n    {}\n\n"
					"    // This function is invoked when destroyed.\n"
					"    protected override void exit()\n    {}\n\n"
					"}";

				scriptFile << sampleScript;
			}
		}

		ImGui::EndPopup();
	}
}


void AssetManagerUI::displayThumbnail(std::size_t resourceOrFolderId, ImTextureID thumbnail, char const* name, std::function<void()> clickCallback, std::function<void()> doubleClickCallback, std::function<void()> additionalLogicAfterThumbnail) {
	if (!isAMatchWithSearchQuery(name)) {
		return;
	}

	ImGui::TableNextColumn();

	ImVec2 padding = ImGui::GetStyle().WindowPadding;
	constexpr float textHeight = 20.f;

	ImGui::PushID(static_cast<int>(resourceOrFolderId));
	ImGui::BeginChild("Thumbnail", ImVec2{ columnWidth, columnWidth + textHeight + 2 * padding.y }, ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar);

	ImVec2 buttonSize = ImVec2{ columnWidth - 2 * padding.x, columnWidth - 2 * padding.x };

	if (thumbnail != NO_TEXTURE) {
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
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

	if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
		doubleClickCallback();
	}

	additionalLogicAfterThumbnail();

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

void AssetManagerUI::handleThumbnailDoubleClick(ResourceID resourceId) {
	if (resourceManager.isResource<ScriptAsset>(resourceId)) {
		AssetFilePath const* filePath = assetManager.getFilepath(resourceId);

		if (!filePath) {
			Logger::error("Attempting to open script of invalid resource id {}", static_cast<std::size_t>(resourceId));
			return;
		}

		// Launch a process that opens visual studio with the scripts.
		static STARTUPINFO si;
		static PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));
		std::wostringstream wss;

		wss << " /Edit \"" << filePath->string.c_str() << "\"";
		std::wstring path{ wss.str() };

		// The path can be applied to createprocess
		// https://stackoverflow.com/questions/973561/starting-visual-studio-from-a-command-prompt
		CreateProcess(L"C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\Common7\\IDE\\devenv.exe",
			const_cast<LPWSTR>(path.c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	else if (resourceManager.isResource<Scene>(resourceId)) {
		editor.loadScene(resourceId);
	}
}

void AssetManagerUI::dragAndDrop(const char* name, std::size_t id) {
	if (editor.isInSimulationMode()) {
		return;
	}

	if (ImGui::BeginDragDropSource()) {
		std::pair<size_t, const char*> map{ id, name };
		ImGui::SetDragDropPayload("DRAGGING_ASSET_ITEM", &map, sizeof(map));
		ImGui::Text("Dragging: %s", name);
		ImGui::EndDragDropSource();
	}
}

void AssetManagerUI::handleAssetMoveToFolder(FolderID folderId) {
	// Accept asset move request into this folder..
	if (ImGui::BeginDragDropTarget()) {
		if (ImGuiPayload const* payload = ImGui::AcceptDragDropPayload("DRAGGING_ASSET_ITEM")) {
			auto&& [id, name] = *((std::pair<std::size_t, const char*>*)payload->Data);
			assetManager.moveAssetToFolder(static_cast<ResourceID>(id), folderId);
		}
		ImGui::EndDragDropTarget();
	}
}

void AssetManagerUI::displayAssetContextMenu(ResourceID id) {
	if (ImGui::BeginPopupContextItem("Asset Context Menu")) {
		if (ImGui::MenuItem("[-] Delete asset (NO UNDO)")) {
			assetManager.deleteAsset(id);
		}

		ImGui::EndPopup();
	}
}

void AssetManagerUI::displayAssetFolder(ResourceID id) {
	// We need to find out what folder does this id belong to..
	FolderID folderId = assetManager.getParentFolder(id);

	if (folderId == NO_FOLDER) {
		return;
	}

	selectedFolderId = folderId;
}

std::optional<std::ofstream> AssetManagerUI::createAssetFile(std::string const& extension, std::string filename, bool binary) {
	auto iterator = assetManager.getDirectories().find(selectedFolderId);

	if (iterator != assetManager.getDirectories().end()) {
		if(filename.empty()) filename = Logger::getUniqueTimedId();

		std::filesystem::path filepath = AssetIO::assetDirectory / iterator->second.relativePath / filename;
		filepath.replace_extension(extension);

		std::ofstream assetFile{ filepath, binary ? std::ios::binary : std::ios::out };
		return assetFile;
	}
	else {
		return std::nullopt;
	}
}
