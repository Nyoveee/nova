#include "scriptAsset.h"
#include <filesystem>

ScriptAsset::ScriptAsset(ResourceID id, ResourceFilePath resourceFilePath, std::string className, bool adminScript, bool toExecuteEvenWhenPaused) :
	Resource				{ id, std::move(resourceFilePath) },
	className				{ std::move(className) },
	adminScript				{ adminScript },
	toExecuteEvenWhenPaused	{ toExecuteEvenWhenPaused }
{}

std::string const& ScriptAsset::getClassName() const {
	return className;
}

bool ScriptAsset::isAdminScript() const {
	return adminScript;
}

bool ScriptAsset::toExecuteWhenPaused() const {
	return toExecuteEvenWhenPaused;
}
