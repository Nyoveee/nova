#pragma once

#include <optional>
#include <fstream>

#include "asset.h"

// using class instead of namespace because i want the ability to private certain functions.
class Compiler {
public:
	static void compile(std::filesystem::path const& descriptorFilepath);

	// debug function to test.
	static void test();

private:
	template <ValidAsset T>
	static void compileAsset		(std::filesystem::path const& descriptorFilepath);

	static void compileTexture		(ResourceFilePath const& resourceFilePath, AssetFilePath const& intermediaryAssetFilepath);
	static void compileModel		(ResourceFilePath const& resourceFilePath, AssetFilePath const& intermediaryAssetFilepath);
	static void compileCubeMap		(ResourceFilePath const& resourceFilePath, AssetFilePath const& intermediaryAssetFilepath);
	static void compileScriptAsset	(ResourceFilePath const& resourceFilePath, AssetFilePath const& intermediaryAssetFilepath);
	static void compileAudio		(ResourceFilePath const& resourceFilePath, AssetFilePath const& intermediaryAssetFilepath);
};

#include "compiler.ipp"