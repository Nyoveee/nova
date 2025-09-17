#include "scriptAsset.h"
#include <filesystem>

ScriptAsset::ScriptAsset(ResourceID id)
	: Asset{ id }
{}

std::string ScriptAsset::getClassName() const {
	return "";
}
