#pragma once

#include "asset.h"

class ScriptAsset : public Asset
{
public:
	FRAMEWORK_DLL_API ScriptAsset(std::string filepath);

	FRAMEWORK_DLL_API ~ScriptAsset() = default;
	FRAMEWORK_DLL_API ScriptAsset(ScriptAsset const& other) = delete;
	FRAMEWORK_DLL_API ScriptAsset(ScriptAsset&& other) = default;
	FRAMEWORK_DLL_API ScriptAsset& operator=(ScriptAsset const& other) = delete;
	FRAMEWORK_DLL_API ScriptAsset& operator=(ScriptAsset&& other) = default;

public:
	FRAMEWORK_DLL_API void load() final;
	FRAMEWORK_DLL_API void unload() final;

public:
	FRAMEWORK_DLL_API std::string getClassName() const;

private:
	//std::string className;
};

