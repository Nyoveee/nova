#pragma once

#include "resource.h"

class ScriptAsset : public Resource
{
public:
	FRAMEWORK_DLL_API ScriptAsset(ResourceID id, ResourceFilePath resourceFilePath, std::string className, bool adminScript, bool toExecuteEvenWhenPaused);

	FRAMEWORK_DLL_API ~ScriptAsset()									= default;
	FRAMEWORK_DLL_API ScriptAsset(ScriptAsset const& other)				= delete;
	FRAMEWORK_DLL_API ScriptAsset(ScriptAsset&& other)					= default;
	FRAMEWORK_DLL_API ScriptAsset& operator=(ScriptAsset const& other)	= delete;
	FRAMEWORK_DLL_API ScriptAsset& operator=(ScriptAsset&& other)		= default;

public:
	FRAMEWORK_DLL_API std::string const& getClassName() const;
	FRAMEWORK_DLL_API bool isAdminScript() const;
	FRAMEWORK_DLL_API bool toExecuteWhenPaused() const;

private:
	std::string className;
	bool adminScript = false;
	bool toExecuteEvenWhenPaused = false;
};

template <>
struct AssetInfo<ScriptAsset> : public BasicAssetInfo {
	AssetInfo() = default;
	AssetInfo(BasicAssetInfo assetInfo) : BasicAssetInfo{ std::move(assetInfo) } {};

	bool adminScript = false;
	bool toExecuteEvenWhenPaused = false;
};