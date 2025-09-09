#include "scriptAsset.h"
#include <filesystem>

ScriptAsset::ScriptAsset(std::string filepath)
	: Asset(filepath)
{}

void ScriptAsset::load() {	
	loadStatus = LoadStatus::Loaded;
}

void ScriptAsset::unload() {
	//className.clear();
}

std::string ScriptAsset::getClassName() const {
	return std::filesystem::path{ getFilePath() }.stem().string();
}
