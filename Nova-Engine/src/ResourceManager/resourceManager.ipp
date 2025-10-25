#include "resource.h"

#include "Logger.h"
#include "resourceManager.h"
#include "assetIO.h"

template <ValidResource T>
ResourceManager::ResourceQuery<T> ResourceManager::getResource(ResourceID id) {
	// Let's find our loaded resource in the map
	auto iterator = loadedResources.find(id);

	// resource is already loaded, let's return it.
	if (iterator != loadedResources.end()) {
		T* resource = dynamic_cast<T*>(iterator->second.get());
		return ResourceQuery<T>{ resource, resource ? QueryResult::Success : QueryResult::WrongType };
	}

	// resource is not loaded, let's load it via our loaders.
	// we first get the filepath of this resource id.
	
	// verify if it's the correct type..
	if (!isResource<T>(id)) {
		return ResourceQuery<T>{ nullptr, QueryResult::WrongType };
	}

	auto filepathIterator = resourceFilePaths.find(id);

	// this resource file was never recorded. invalid resource id?
	if (filepathIterator == resourceFilePaths.end()) {
		return ResourceQuery<T>{ nullptr, QueryResult::Invalid };
	}

	auto&& [_, resourceFilePath] = *filepathIterator;

	// attempts to load the resource..
	std::optional<ResourceConstructor> resourceConstructor = ResourceLoader<T>::load(id, resourceFilePath);

	if (!resourceConstructor) {
		return ResourceQuery<T>{ nullptr, QueryResult::LoadingFailed };
	}

	// constructs the resource..
	auto resourcePtr = resourceConstructor.value()();
	auto&& [resourceIterator, __]  = loadedResources.insert({id, std::move(resourcePtr)});
	assert(resourceIterator != loadedResources.end());

	T* resource = static_cast<T*>(resourceIterator->second.get());
	return ResourceQuery<T>{ resource, QueryResult::Success };
}

template<ValidResource T>
T* ResourceManager::getResourceOnlyIfLoaded(ResourceID id) {
	// Let's find our loaded resource in the map
	auto iterator = loadedResources.find(id);

	// resource is already loaded, let's return it.
	if (iterator != loadedResources.end()) {
		T* resource = dynamic_cast<T*>(iterator->second.get());
		return resource;
	}

	return nullptr;
}

template<ValidResource T>
ResourceID ResourceManager::addResourceFile(ResourceFilePath const& filepath, ResourceID id) {
	try {
		if (id == INVALID_RESOURCE_ID) {
			// Retrieve the Resource ID from filepath.
			// May throw an exception.
			id = std::stoull(std::filesystem::path{ filepath }.stem().string());
		}

		auto [iterator, success] = resourceFilePaths.insert({ id, filepath });

		if (!success) {
			// asset id collision occur! this shouldn't happen though.
			Logger::error("Asset ID collision occured {} for: {}", static_cast<std::size_t>(id), filepath.string);
			return INVALID_RESOURCE_ID;
		}

		// record this asset to the corresponding asset type.
		resourcesByType[Family::id<T>()].push_back(id);

		return id;
	}
	catch (std::exception const& ex) {
		Logger::error("Failed to add resource file. {}", ex.what());
		return INVALID_RESOURCE_ID;
	}
}

template<ValidResource ...T>
void ResourceManager::loadAllResources() {
	([&] {
		auto iterator = AssetIO::subResourceDirectories.find(Family::id<T>());
		assert(iterator != AssetIO::subResourceDirectories.end() && "This descriptor sub directory is not recorded.");
		std::filesystem::path const& directory = iterator->second;

		// recursively iterate through a directory and parse all resource files.
		for (const auto& entry : std::filesystem::recursive_directory_iterator{ directory }) {
			std::filesystem::path currentPath = entry.path();

			if (entry.is_regular_file()) {
				addResourceFile<T>(currentPath);
			}
		}
	}(), ...);
}

template<ValidResource T>
std::vector<ResourceID> const& ResourceManager::getAllResources() const {
	auto iterator = resourcesByType.find(Family::id<T>());

	if (iterator == resourcesByType.end()) {
		static std::vector<ResourceID> empty;
		return empty;
	}

	auto&& [_, allAssets] = *iterator;
	return allAssets;
}

template<ValidResource T>
ResourceID ResourceManager::getSomeResourceID() const {
	auto iterator = resourcesByType.find(Family::id<T>());

	if (iterator == resourcesByType.end()) {
		Logger::error("Attempt to get an asset id of an invalid type?");
		return INVALID_RESOURCE_ID;
	}

	auto&& [_, allResources] = *iterator;

	if (allResources.empty()) {
		Logger::error("This asset type has no asset?");
		return INVALID_RESOURCE_ID;
	}

	return allResources[0];
}

template<ValidResource T>
bool ResourceManager::isResource(ResourceID id) const {
	auto iterator = resourcesByType.find(Family::id<T>());
	
	// this type of resource is never recorded.
	if (iterator == resourcesByType.end()) {
		return false;
	}

	auto&& [_, resourceIds] = *iterator;

	auto findIterator = std::ranges::find(resourceIds, id);
	return findIterator != resourceIds.end();
}