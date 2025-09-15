#include "scriptAsset.h"
#include <filesystem>

ScriptAsset::ScriptAsset(ResourceFilePath filepath)
	: Asset{ std::move(filepath) }
{}

bool ScriptAsset::load() {
	return false;
}

void ScriptAsset::unload() {
	//className.clear();
}

std::string ScriptAsset::getClassName() const {
	return std::filesystem::path{ getFilePath() }.stem().string();
}
