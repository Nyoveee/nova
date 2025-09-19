#include "scriptAsset.h"
#include <filesystem>

ScriptAsset::ScriptAsset(ResourceID id, ResourceFilePath resourceFilePath)
	: Resource{ id, std::move(resourceFilePath) }
{}

std::string ScriptAsset::getClassName() const {
	return "";
}
