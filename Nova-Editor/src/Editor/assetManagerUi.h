#pragma once

#include <limits>
#include <functional>

#include "imgui.h"
#include "texture.h"
#include "type_alias.h"

#undef max
constexpr ImTextureID NO_TEXTURE = std::numeric_limits<ImTextureID>::max();

class Editor;
class AssetManager;
class ResourceManager;

class AssetManagerUI {
public:
	AssetManagerUI(Editor& assetManager);

public:
	void update();

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

	void displayThumbnail(int imguiId, ImTextureID thumbnail, char const* name, std::function<void()> clickCallback, std::function<void()> doubleClickCallback);

	// checks if a given name matches with the current search query.
	bool isAMatchWithSearchQuery(std::string const& name) const;

	void handleThumbnailDoubleClick(Asset& resource);

private:
	AssetManager& assetManager;
	ResourceManager& resourceManager;

	FolderID selectedFolderId;

	std::string searchQuery;
	std::string allUpperCaseSearchQuery; // we keep all upper case version of the search query.

private:
	float columnWidth = 100.f;

	//Texture folderIcon;
};