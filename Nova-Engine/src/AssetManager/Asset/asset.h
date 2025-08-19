#pragma once

#include <string>

#include "Libraries/type_alias.h"
#include "export.h"

// Unfortunately, copy-and-swap idiom doesn't work well with base abstract classes.
class Asset {
public:
	DLL_API Asset(std::string filepath);

	DLL_API virtual ~Asset() = 0;
	DLL_API Asset(Asset const& other)				= delete;
	DLL_API Asset(Asset&& other)					= default;
	DLL_API Asset& operator=(Asset const& other)	= delete;
	DLL_API Asset& operator=(Asset&& other)			= default;

public:
	DLL_API virtual void load  () = 0;
	DLL_API virtual void unload() = 0;

	DLL_API std::string const& getFilePath() const;
	DLL_API bool isLoaded() const;

protected:
	bool hasLoaded;

public:
	std::string name;
	AssetID id;

private:
	std::string filepath;
};