#pragma once

#include <string>

#include "Libraries/type_alias.h"

// Unfortunately, copy-and-swap idiom doesn't work well with base abstract classes.
class Asset {
public:
	Asset(std::string filepath);

	virtual ~Asset() = 0;
	Asset(Asset const& other)				= delete;
	Asset(Asset&& other)					= default;
	Asset& operator=(Asset const& other)	= delete;
	Asset& operator=(Asset&& other)			= default;

public:
	virtual void load  () = 0;
	virtual void unload() = 0;

	std::string const& getFilePath() const;
	bool isLoaded() const;

protected:
	bool hasLoaded;

public:
	std::string name;
	AssetID id;

private:
	std::string filepath;
};