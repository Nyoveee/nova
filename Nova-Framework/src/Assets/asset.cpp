#include "asset.h"
#include "Logger.h"
#include <fstream>

Asset::Asset(ResourceFilePath p_filepath) :
	filepath		{ std::move(p_filepath) },
	resourceFile	{ filepath, std::ios::binary },
	id				{ INVALID_ASSET_ID }, 
	loadStatus		{ LoadStatus::NotLoaded } 
{
	if (!resourceFile) {
		Logger::error("Failed to construct asset with filepath {}", filepath.string);
		return;
	}

	std::size_t id_sizet;
	resourceFile.read(reinterpret_cast<char*>(&id_sizet), sizeof(id_sizet));
	id = id_sizet;
}

Asset::~Asset(){}

std::string const& Asset::getFilePath() const {
	return filepath.string;
}

Asset::LoadStatus Asset::getLoadStatus() const {
	return loadStatus;
}

bool Asset::isLoaded() const {
	return loadStatus == LoadStatus::Loaded;
}

void Asset::toLoad() {
	loadStatus = LoadStatus::Loading;

	// reset file pointer. skipped to the 1st 8 bytes because it's used as resource id.
	resourceFile.clear();
	resourceFile.seekg(8);

	if (!load()) {
		loadStatus = LoadStatus::LoadingFailed;
	}
	else {
		loadStatus = LoadStatus::Loaded;
	}
}

void Asset::toUnload() {
	unload();
	loadStatus = LoadStatus::NotLoaded;
}
