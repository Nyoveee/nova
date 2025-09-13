#include "asset.h"
#include "model.h"
#include "texture.h"

#include "Logger.h"
#include "resourceManager.h"

template <ValidAsset T>
ResourceManager::ResourceQuery<T> ResourceManager::getResource(ResourceID id) {
	// Let's find our resource in the map.
	auto iterator = resources.find(id);

	// resource doesn't exist in map.
	if (iterator == resources.end()) {
		return ResourceQuery<T>{ nullptr, QueryResult::Invalid };
	}

	// We retrieve the resource, and attempts to load it if it's not loaded.
	auto&& [_, resource] = *iterator;

	switch (resource->getLoadStatus())
	{
	case Asset::LoadStatus::NotLoaded:
		resource->toLoad();
		break;
	case Asset::LoadStatus::Loaded:
		break;
	case Asset::LoadStatus::Loading:
		return ResourceQuery<T>{ nullptr, QueryResult::Loading };
	case Asset::LoadStatus::LoadingFailed:
		Logger::error("Loading operator for asset id of {} has failed. Retrying..", static_cast<std::size_t>(id));
		resource->toLoad();
		break;
	}

	// Asset has already been loaded after we attempt to load it. (Single-threaded loading, no multi threading.)
	if (resource->isLoaded()) {
		T* typedAsset = dynamic_cast<T*>(resource.get());
		return ResourceQuery<T>{ typedAsset, typedAsset ? QueryResult::Success : QueryResult::WrongType };
	}

	if (resource->getLoadStatus() == Asset::LoadStatus::LoadingFailed) {
		return ResourceQuery<T>{ nullptr, QueryResult::LoadingFailed };
	}
	else {
		return ResourceQuery<T>{ nullptr, QueryResult::Loading };
	}
}

template<ValidAsset T>
void ResourceManager::addResourceFile(std::filesystem::path const& filepath) {
	if constexpr (std::same_as<T, Texture>) {
		
	}
	else if constexpr (std::same_as<T, Model>) {

	}
	else {
		[] <bool flag = true>() {
			static_assert(flag, "Attempting to parse a unsupported resource file type.\n");
		}();
	}

	// record this asset to the corresponding asset type.
	// resourcesByType[Family::id<T>()].push_back(assetInfo.id);

	// associate this asset id with this asset type.
	// assetIdToType[assetInfo.id] = Family::id<T>();
}

template<ValidAsset T>
void ResourceManager::addResourceFile(AssetInfo<T> assetInfo) {
	std::unique_ptr<T> newAsset = std::make_unique<T>(
		createAsset<T>(assetInfo)
	);

	newAsset->id = assetInfo.id;
	newAsset->name = assetInfo.name;

	auto [iterator, success] = resources.insert({ assetInfo.id, std::move(newAsset) });

	if (!success) {
		// asset id collision occur! this shouldn't happen though.
		Logger::error("Asset ID collision occured for: {}", assetInfo.filepath);
		return;
	}

	auto&& [assetId, asset] = *iterator;

	// record this asset to the corresponding asset type.
	resourcesByType[Family::id<T>()].push_back(assetInfo.id);

	// associate this asset id with this asset type.
	resourceIdToType[assetInfo.id] = Family::id<T>();
}

template<ValidAsset T>
void ResourceManager::loadAllResources(std::filesystem::path const& directory) {
	// recursively iterate through a directory and parse all resource files.
	for (const auto& entry : std::filesystem::recursive_directory_iterator{ directory }) {
		std::filesystem::path currentPath = entry.path();

		if (entry.is_regular_file()) {
			addResourceFile<T>(currentPath);
		}
	}
}

template<ValidAsset T>
std::vector<ResourceID> const& ResourceManager::getAllResources() const {
	auto iterator = resourcesByType.find(Family::id<T>());

	if (iterator == resourcesByType.end()) {
		Logger::error("Attempt to retrieve all assets of an invalid type?");
		static std::vector<ResourceID> empty;
		return empty;
	}

	auto&& [_, allAssets] = *iterator;
	return allAssets;
}

template<ValidAsset T>
ResourceID ResourceManager::getSomeResourceID() const {
	auto iterator = resourcesByType.find(Family::id<T>());

	if (iterator == resourcesByType.end()) {
		Logger::error("Attempt to get an asset id of an invalid type?");
		return INVALID_ASSET_ID;
	}

	auto&& [_, allResources] = *iterator;

	if (allResources.empty()) {
		Logger::error("This asset type has no asset?");
		return INVALID_ASSET_ID;
	}

	return allResources[0];
}

template<ValidAsset T>
bool ResourceManager::isResource(ResourceID id) const {
	auto iterator = resourceIdToType.find(id);

	assert(iterator != resourceIdToType.end() && "asset id doesn't exist / have no associated asset type id?");

	auto [_, resourceTypeId] = *iterator;
	return resourceTypeId == Family::id<T>();
}