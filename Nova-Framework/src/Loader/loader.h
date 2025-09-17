#pragma once

#include <functional>
#include <memory>
#include <optional>

#include "asset.h"

class Texture;
class Model;
class ScriptAsset;
class Audio;
class CubeMap;

#define ResourceLoaderDefinition(AssetType) \
template<> \
class ResourceLoader<AssetType> { \
public: \
	FRAMEWORK_DLL_API static std::optional<ResourceConstructor> load(ResourceFilePath const& resourceFilePath); \
}; \

// a functor that constructs the underlying resource when invoked.
struct ResourceConstructor {
	ResourceConstructor(std::function<std::unique_ptr<Asset>()> function) : function{ function } {};

	std::unique_ptr<Asset> operator()() {
		return function();
	}

	std::function<std::unique_ptr<Asset>()> function;
};

template <typename T>
class ResourceLoader {
	static_assert(false, "No explicit template specialisation found for this asset type");

	// all implementation of resource loader HAS to implement the function below.
	// static std::optional<ResourceConstructor> load(ResourceFilePath const& resourceFilePath);
};

// explicit template specialisation for respective loaders.
ResourceLoaderDefinition(Model)
ResourceLoaderDefinition(Texture)
ResourceLoaderDefinition(CubeMap)
ResourceLoaderDefinition(ScriptAsset)
ResourceLoaderDefinition(Audio)