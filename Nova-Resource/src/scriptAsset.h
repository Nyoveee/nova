#pragma once

#include "asset.h"

class ScriptAsset : public Asset
{
public:
	DLL_API ScriptAsset(std::string filepath);

	DLL_API ~ScriptAsset() = default;
	DLL_API ScriptAsset(ScriptAsset const& other) = delete;
	DLL_API ScriptAsset(ScriptAsset&& other) = default;
	DLL_API ScriptAsset& operator=(ScriptAsset const& other) = delete;
	DLL_API ScriptAsset& operator=(ScriptAsset&& other) = default;

public:
	DLL_API void load() final;
	DLL_API void unload() final;

public:
	DLL_API std::string getClassName() const;

private:
	//std::string className;
};

