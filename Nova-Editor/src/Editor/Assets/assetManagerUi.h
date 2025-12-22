#pragma once

#include <limits>
#include <functional>

#include "imgui.h"
#include "texture.h"
#include "type_alias.h"

#include "loader.h"

#undef max
constexpr ImTextureID NO_TEXTURE = std::numeric_limits<ImTextureID>::max();

class Editor;
class AssetManager;
class ResourceManager;
class AssetViewerUI;

class AssetManagerUI {
public:
	AssetManagerUI(Editor& assetManager, AssetViewerUI& assetViewerUi);

public:
	void update();

	// Displays the folder of this particular asset
	void displayAssetFolder(ResourceID id);

	std::optional<std::ofstream> createAssetFile(std::string const& extension, std::string filename = "", bool binary = false);

private:
	void displayLeftNavigationPanel();
	void displayRightContentPanel();
	
	// displays the full path
	void displaySelectedFolderRelativePath();
	
	// recursively calls this to form a full path of clickable links
	void displayClickableFolderPath(FolderID folderId, bool toDisplayCaret);
	
	void displayFolderTreeNode(FolderID folderId);
	void displayFolderContent(FolderID folderId);
	void displayAssetThumbnail(ResourceID resourceId);
	void displayFolderThumbnail(FolderID folderId);
	void displayCreateAssetContextMenu();

	void displayThumbnail(
		std::size_t resourceIdOrFolderId,						// This function can be invoked to display either folder or asset. Pass the respective ids here.
		ImTextureID thumbnail,									// Thumbnail preview..
		char const* name,										// Name of the thumbnail..
		std::function<void()> clickCallback,					// When the thumbnail is clicked, this callback is invoked.
		std::function<void()> doubleClickCallback,				// When the thumbnail is double clicked, this callback is invoked.
		std::function<void()> additionalLogicAfterThumbnail		// After rendering the thumbnail, this function is invoked. Good for context menu, drag and drop, etc..
	);

	// checks if a given name matches with the current search query.
	bool isAMatchWithSearchQuery(std::string const& name) const;

	void handleThumbnailDoubleClick(ResourceID resourceId);

	void dragAndDrop(const char* name, std::size_t id);

	void handleAssetMoveToFolder(FolderID folderId);

	void displayAssetContextMenu(ResourceID id);

	//std::optional<std::ofstream> createAssetFile(std::string const& extension, std::string filename = "", bool binary = false);

	ImTextureID getAssetThumbnailImage(ResourceID id);

private:
	Editor& editor;
	AssetManager& assetManager;
	ResourceManager& resourceManager;
	AssetViewerUI& assetViewerUi;

	FolderID selectedFolderId;

	std::string searchQuery;
	std::string allUpperCaseSearchQuery; // we keep all upper case version of the search query.

private:
	std::unique_ptr<Texture> folderIcon;
	std::unique_ptr<Texture> textureIcon;
	std::unique_ptr<Texture> audioIcon;
	std::unique_ptr<Texture> scriptIcon;
	std::unique_ptr<Texture> sceneIcon;
	std::unique_ptr<Texture> modelIcon;
	std::unique_ptr<Texture> cubeMapIcon;
	std::unique_ptr<Texture> materialIcon;
	std::unique_ptr<Texture> shaderIcon;
	std::unique_ptr<Texture> navmeshIcon;
	std::unique_ptr<Texture> prefabIcon;
	std::unique_ptr<Texture> sequencerIcon;
	std::unique_ptr<Texture> animationControllerIcon;
	std::unique_ptr<Texture> fontIcon;
private:
	float columnWidth = 100.f;

	//Texture folderIcon;
};