#pragma once

#include <filesystem>
#include <functional>
#include <vector>
#include <string>
#include "type_alias.h"

struct Folder {
	FolderID id;		// each folder is assigned a id such that it is able to references one another.
	FolderID parent;

	std::vector<ResourceID> assets;
	std::vector<FolderID> childDirectories;
	std::string name;
	std::filesystem::path relativePath;
};