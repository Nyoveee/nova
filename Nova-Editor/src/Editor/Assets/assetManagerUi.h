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

	//void displayThumbnail(int imguiId, ImTextureID thumbnail, char const* name, std::function<void()> clickCallback, std::function<void()> doubleClickCallback);
	void displayThumbnail(std::size_t resourceIdOrFolderId, ImTextureID thumbnail, char const* name, std::function<void()> clickCallback, std::function<void()> doubleClickCallback);

	// checks if a given name matches with the current search query.
	bool isAMatchWithSearchQuery(std::string const& name) const;

	void handleThumbnailDoubleClick(ResourceID resourceId);

	void dragAndDrop(const char* name, std::size_t id);
	
	//std::optional<std::ofstream> createAssetFile(std::string const& extension, std::string filename = "", bool binary = false);

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

private:
	float columnWidth = 100.f;

	//Texture folderIcon;
};