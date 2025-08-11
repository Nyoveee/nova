#pragma once

#include <string>

#include "Libraries/type_alias.h"
#include "export.h"

// Unfortunately, copy-and-swap idiom doesn't work well with base abstract classes.
class Asset {
public:
	DLL_API Asset(std::string filepath);

	virtual ~Asset() = 0;
	Asset(Asset const& other)				= delete;
	Asset(Asset&& other)					= default;
	Asset& operator=(Asset const& other)	= delete;
	Asset& operator=(Asset&& other)			= default;

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