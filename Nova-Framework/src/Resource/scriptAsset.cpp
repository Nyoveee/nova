#include "scriptAsset.h"
#include <filesystem>

ScriptAsset::ScriptAsset(ResourceID id, ResourceFilePath resourceFilePath, std::string className, bool AdminScript) :
	Resource	{ id, std::move(resourceFilePath) },
	className	{ std::move(className) },
	adminScript {std::move(AdminScript)}
{}

std::string const& ScriptAsset::getClassName() const {
	return className;
}

bool const& ScriptAsset::isAdminScript() const
{
	return adminScript;
}
