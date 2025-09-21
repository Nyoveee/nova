#pragma once

#include "resource.h"

class ScriptAsset : public Resource
{
public:
	FRAMEWORK_DLL_API ScriptAsset(ResourceID id, ResourceFilePath resourceFilePath, std::string className);

	FRAMEWORK_DLL_API ~ScriptAsset()									= default;
	FRAMEWORK_DLL_API ScriptAsset(ScriptAsset const& other)				= delete;
	FRAMEWORK_DLL_API ScriptAsset(ScriptAsset&& other)					= default;
	FRAMEWORK_DLL_API ScriptAsset& operator=(ScriptAsset const& other)	= delete;
	FRAMEWORK_DLL_API ScriptAsset& operator=(ScriptAsset&& other)		= default;

public:
	FRAMEWORK_DLL_API std::string const& getClassName() const;

private:
	std::string className;
};

