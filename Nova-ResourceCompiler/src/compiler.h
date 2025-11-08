#pragma once

#include <optional>
#include <fstream>

#include "resource.h"

// using class instead of namespace because i want the ability to private certain functions.
class Compiler {
public:
	static int compile(DescriptorFilePath const& descriptorFilepath);
	static void recompileAllSystemAssets();

private:
	template <ValidResource T>
	static int compileAsset			(AssetInfo<T> const& assetInfo, ResourceFilePath const& resourceFilePath);

	static int compileTexture		(ResourceFilePath const& resourceFilePath, AssetFilePath const& intermediaryAssetFilepath, AssetInfo<Texture>::Compression compressionFormat);
	static int compileFont			(ResourceFilePath const& resourceFilePath, AssetFilePath const& intermediaryAssetFilepath, unsigned int fontSize);

	static int compileModel			(ResourceFilePath const& resourceFilePath, AssetFilePath const& intermediaryAssetFilepath, float scale);
	static int compileScriptAsset	(ResourceFilePath const& resourceFilePath, AssetFilePath const& intermediaryAssetFilepath, std::string className);
	static int compileShaderAsset   (ResourceFilePath const& resourceFilePath, AssetFilePath const& intermediaryAssetFilepath, Pipeline pipeline);
	
	// default compiling an asset just makes a copy of the original intermediary asset as resource.
	static int defaultCompile		(ResourceFilePath const& resourceFilePath, AssetFilePath const& intermediaryAssetFilepath);
};

#include "compiler.ipp"