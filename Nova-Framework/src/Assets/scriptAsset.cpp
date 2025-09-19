#include "scriptAsset.h"
#include <filesystem>

ScriptAsset::ScriptAsset(ResourceID id)
	: Resource{ id }
{}

std::string ScriptAsset::getClassName() const {
	return "";
}
