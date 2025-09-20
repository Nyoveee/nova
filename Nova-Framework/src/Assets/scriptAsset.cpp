#include "scriptAsset.h"
#include <filesystem>

ScriptAsset::ScriptAsset(ResourceID id, ResourceFilePath resourceFilePath, std::string className) : 
	Resource	{ id, std::move(resourceFilePath) },
	className	{ std::move(className) }
{}

std::string const& ScriptAsset::getClassName() const {
	return className;
}
