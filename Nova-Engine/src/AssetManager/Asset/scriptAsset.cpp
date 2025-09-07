#include "scriptAsset.h"
#include <filesystem>

ScriptAsset::ScriptAsset(std::string filepath)
	: Asset(filepath)
{}

void ScriptAsset::load(AssetManager& assetManager) {	
	(void) assetManager;
	loadStatus = LoadStatus::Loaded;
}

void ScriptAsset::unload() {
	//className.clear();
}

std::string ScriptAsset::getClassName() const {
	return std::filesystem::path{ getFilePath() }.stem().string();
}
