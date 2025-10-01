#pragma once

#include <optional>
#include <fstream>

#include "resource.h"

// using class instead of namespace because i want the ability to private certain functions.
class Compiler {
public:
	static int compile(DescriptorFilePath const& descriptorFilepath);

private:
	template <ValidResource T>
	static int compileAsset			(DescriptorFilePath const& descriptorFilepath);

	static int compileTexture		(ResourceFilePath const& resourceFilePath, AssetFilePath const& intermediaryAssetFilepath, AssetInfo<Texture>::Compression compressionFormat);
	static int compileModel			(ResourceFilePath const& resourceFilePath, AssetFilePath const& intermediaryAssetFilepath);
	static int compileCubeMap		(ResourceFilePath const& resourceFilePath, AssetFilePath const& intermediaryAssetFilepath);
	static int compileScriptAsset	(ResourceFilePath const& resourceFilePath, std::string className);

	// default compiling an asset just makes a copy of the original intermediary asset as resource.
	static int defaultCompile		(ResourceFilePath const& resourceFilePath, AssetFilePath const& intermediaryAssetFilepath);
};

#include "compiler.ipp"