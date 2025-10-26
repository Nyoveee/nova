#pragma once

#include <functional>
#include <memory>
#include <optional>

#include "resource.h"


#define ResourceLoaderDefinition(AssetType) \
template<> \
class ResourceLoader<AssetType> { \
public: \
	FRAMEWORK_DLL_API static std::optional<ResourceConstructor> load(ResourceID id, ResourceFilePath const& resourceFilePath); \
}; \

// a functor that constructs the underlying resource when invoked.
struct ResourceConstructor {
	ResourceConstructor(std::function<std::unique_ptr<Resource>()> function) : function{ function } {};

	std::unique_ptr<Resource> operator()() {
		return function();
	}

	std::function<std::unique_ptr<Resource>()> function;
};

// default loader..
template <typename T>
class ResourceLoader {
public:
	// all implementation of resource loader HAS to implement the function below.
	static std::optional<ResourceConstructor> load(ResourceID id, ResourceFilePath const& resourceFilePath) {
		return { {[id, resourceFilePath] {
			return std::make_unique<T>(id, resourceFilePath);
		}} };
	}
};

inline bool readNextByteIfNull(std::ifstream& inputFile) {
	char c;
	inputFile.read(&c, 1);

	return c == 0;
}

class CustomShader;
class Material;

// explicit template specialisation for respective loaders.
ResourceLoaderDefinition(Model)
ResourceLoaderDefinition(Texture)
ResourceLoaderDefinition(CubeMap)
ResourceLoaderDefinition(ScriptAsset)
ResourceLoaderDefinition(NavMesh)
ResourceLoaderDefinition(Controller)
ResourceLoaderDefinition(CustomShader)
ResourceLoaderDefinition(Material)
ResourceLoaderDefinition(Font)