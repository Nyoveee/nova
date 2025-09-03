#pragma once
#include "asset.h"
class ScriptAsset : public Asset
{
public:
	ScriptAsset(std::string filepath);

	~ScriptAsset() = default;
	ScriptAsset(ScriptAsset const& other) = delete;
	ScriptAsset(ScriptAsset&& other) = default;
	ScriptAsset& operator=(ScriptAsset const& other) = delete;
	ScriptAsset& operator=(ScriptAsset&& other) = default;
public:
	void load(AssetManager&) final {};
	void unload() final {};
};

