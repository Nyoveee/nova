#pragma once

#include <limits>
#include <functional>

#include "imgui.h"
#include "AssetManager/Asset/texture.h"
#include "Libraries/type_alias.h"

#undef max
constexpr ImTextureID NO_TEXTURE = std::numeric_limits<ImTextureID>::max();

class Editor;
class AssetManager;

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
	void displayAssetThumbnail(AssetID assetId);
	void displayFolderThumbnail(FolderID folderId);

	void displayThumbnail(int imguiId, ImTextureID thumbnail, char const* name, std::function<void()> clickCallback);

	// checks if a given name matches with the current search query.
	bool isAMatchWithSearchQuery(std::string const& name) const;

private:
	AssetManager& assetManager;
	FolderID selectedFolderId;

	std::string searchQuery;
	std::string allUpperCaseSearchQuery; // we keep all upper case version of the search query.

private:
	float columnWidth = 100.f;

	Texture folderIcon;
};