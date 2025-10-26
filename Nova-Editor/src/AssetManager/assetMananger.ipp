#include <sstream>
#include <cstdlib>

#include "assetManager.h"
#include "Profiling.h"
#include "ResourceManager/resourceManager.h"
#include "assetIO.h"

#include "Serialisation/serialisation.h"

template<typename T>
constexpr bool dependent_false = false;

template<ValidResource T>
bool AssetManager::compileIntermediaryFile(AssetInfo<T> const& descriptor) {
#if _DEBUG
	const char* executableConfiguration = "Debug";
#else
	const char* executableConfiguration = "Release";
#endif
	constexpr const char* executableName = "Nova-ResourceCompiler.exe";

	std::filesystem::path compilerPath = std::filesystem::current_path() / "ExternalApplication" / executableConfiguration / executableName;

	DescriptorFilePath descriptorFilePath = AssetIO::getDescriptorFilename<T>(descriptor.id);

	// https://stackoverflow.com/questions/27975969/how-to-run-an-executable-with-spaces-using-stdsystem-on-windows/27976653#27976653
	std::string command = "\"\"" + compilerPath.string() + "\" \"" + descriptorFilePath.string + "\"\"";

	Logger::info("Running command: {}", command);

	if (std::system(command.c_str())) {
		Logger::error("Error compiling {}", descriptorFilePath.string);
		return false;
	}
	else {
		Logger::info("Successful compiling {}", descriptorFilePath.string);
		Logger::info("Resource file created: {}", AssetIO::getResourceFilename<T>(descriptor.id).string);
		return true;
	}
}

template<ValidResource T>
bool AssetManager::hasAssetChanged(AssetInfo<T> const& descriptor) const {
	auto cacheFilePath = AssetIO::getAssetCacheFilename<T>(descriptor.id);

	std::ifstream cacheFile{ cacheFilePath };

	// Cache file doesn't exist.., we assume asset has changed.
	if (!cacheFile) {
		return true;
	}

	long long cachedDuration;

	// Attempt to read the time..
	try {
		std::string durationInString;
		std::getline(cacheFile, durationInString);
		cachedDuration = std::stoull(durationInString);
	}
	catch (std::exception const&) {
		// Failed to read cache file.. invalid..
		return true;
	}

	// We compare our cached duration and see if it matches with the filepath...
	auto assetLastWriteTime = std::filesystem::last_write_time(descriptor.filepath);
	auto assetEpoch = std::chrono::duration_cast<std::chrono::milliseconds>(assetLastWriteTime.time_since_epoch());
	long long duration = assetEpoch.count();

	return cachedDuration != duration;
}

template<ValidResource T>
void AssetManager::updateAssetCache(AssetInfo<T> const& descriptor) const {
	auto cacheFilePath = AssetIO::getAssetCacheFilename<T>(descriptor.id);

	std::ofstream cacheFile{ cacheFilePath };

	if (!cacheFile) {
		Logger::error("Unable to update asset cache file.");
		return;
	}

	auto assetLastWriteTime = std::filesystem::last_write_time(descriptor.filepath);
	auto assetEpoch = std::chrono::duration_cast<std::chrono::milliseconds>(assetLastWriteTime.time_since_epoch());
	long long duration = assetEpoch.count();

	cacheFile << duration << '\n';
	Logger::info("Successfully updated {} cache.", descriptor.filepath.string);
}

template<ValidResource T>
void AssetManager::loadSystemResourceDescriptor(std::unordered_map<ResourceID, ResourceFilePath> const& systemResources) {
	for (auto&& [id, filepath] : systemResources) {
		DescriptorFilePath descriptorFilePath = std::filesystem::path{ filepath }.replace_extension(".desc");
		
		auto optional = AssetIO::parseDescriptorFile<T>(descriptorFilePath, AssetIO::systemResourceDirectory);

		AssetInfo<T> descriptor = [&]() {
			if (optional) { return optional.value(); }
			else		  { return AssetInfo<T>{id, static_cast<std::filesystem::path>(filepath).filename().string()}; }
		}();

		descriptor.id = id;
		assetToDescriptor.insert({ id, std::make_unique<AssetInfo<T>>(descriptor) });
		systemResourcesId.insert(id);
	}
}

template<ValidResource T>
void AssetManager::serializeAllResources() {
	for (auto const& resourceId : resourceManager.getAllResources<T>()) {
		if (systemResourcesId.contains(resourceId)) {
			continue;
		}

		T* resource = resourceManager.getResourceOnlyIfLoaded<T>(resourceId);

		if (!resource) {
			continue;
		}

		// get descriptor..
		auto iterator = assetToDescriptor.find(resourceId);

		if (iterator == assetToDescriptor.end()) {
			Logger::error("Failed to serialise {}", static_cast<std::size_t>(resourceId));
			continue;
		}

		auto&& [__, descriptor] = *iterator;

		std::ofstream outputFile = [&]() -> std::ofstream {
			// Controller and material wants to overwrite the original asset file..
			if constexpr (std::same_as<T, Controller> || std::same_as<T, Material>) {
				return std::ofstream{ descriptor->filepath };
			}
			// Custom shader wants to overwrite the resource file..
			else if constexpr (std::same_as <T, CustomShader>) {
				return std::ofstream{ AssetIO::getResourceFilename<T>(resourceId) };
			}
			else {
				static_assert(dependent_false<T> && "Unhandled serialisation case" __FUNCSIG__);
				return {};
			}
		}();

		if (!outputFile) {
			assert(false && "Invalid file?");
			continue;
		}

		// get resource file..
		if constexpr (std::same_as<T, Controller>) {
			Serialiser::serializeToJsonFile(resource->data, outputFile);
			Logger::info("Serialised controller: {}", static_cast<std::size_t>(resourceId));
		}
		else if constexpr (std::same_as<T, Material>) {
			Serialiser::serializeToJsonFile(resource->materialData, outputFile);
			Logger::info("Serialised material: {}", static_cast<std::size_t>(resourceId));
		}
		else if constexpr (std::same_as<T, CustomShader>) {
			Serialiser::serializeToJsonFile(resource->customShaderData, outputFile);
			Logger::info("Serialised shader: {}", static_cast<std::size_t>(resourceId));
		}
		else {
			static_assert(dependent_false<T> && "Unhandled serialisation case" __FUNCSIG__);
			return;
		}

	}
}

template<ValidResource ...T>
void AssetManager::loadAllDescriptorFiles() {
	([&] {
		auto iterator = AssetIO::subDescriptorDirectories.find(Family::id<T>());
		assert(iterator != AssetIO::subDescriptorDirectories.end() && "This descriptor sub directory is not recorded.");
		std::filesystem::path const& directory = iterator->second;

		Logger::info("===========================");
		Logger::info("Loading all {} descriptors.", directory.stem().string());
		Logger::info("----");

		// recursively iterate through a directory and parse all descriptor files.
		for (const auto& entry : std::filesystem::recursive_directory_iterator{ directory }) {
			std::filesystem::path descriptorFilepath = entry.path();

			if (!entry.is_regular_file()) {
				continue;
			}

			if (descriptorFilepath.extension() != ".desc") {
				continue;
			}

			Logger::info("Parsing {}..", descriptorFilepath.string());

			// We attempt to parse the descriptor file, to find out the original asset it is referencing...
			std::optional<AssetInfo<T>> opt = AssetIO::parseDescriptorFile<T>(descriptorFilepath);
			AssetInfo<T> descriptor;

			if (!opt) {
				Logger::info("Parsing failed, removing invalid descriptor file..\n");
				std::filesystem::remove(descriptorFilepath);
				continue;
			}
			// We check the validity of descriptor file, whether the asset path is pointing to a valid file.
			else if (!std::filesystem::is_regular_file(std::filesystem::path{ opt.value().filepath })) {
				Logger::info("Descriptor file is pointing to an invalid asset filepath, removing descriptor file..\n");
				std::filesystem::remove(descriptorFilepath);
				continue;
			}
			else {
				Logger::info("Sucessfully parsed descriptor file.");
				descriptor = opt.value();
			}
			
			if (hasAssetChanged<T>(descriptor)) {
				Logger::info("Asset has changed, recompiling it's corresponding resource file..\n");
				resourceManager.removeResource(descriptor.id);
				createResourceFile<T>(descriptor);
			}
			// If it does, we check if the resource manager already have this particular resource loaded.
			// If it doesn't exist, it means that this resource file is missing / invalid.
			// Let's compile the corresponding intermediary asset file and load it to the resource manager.
			else if (!resourceManager.doesResourceExist(descriptor.id)) {
				Logger::info("Corresponding resource file does not exist, compiling intermediary asset {}\n", descriptor.filepath.string);
				createResourceFile<T>(descriptor);
			}
			else {
				Logger::info("A valid resource file exist for this descriptor and asset.\n");
				// we record all encountered intermediary assets with corresponding filepaths.
				intermediaryAssetsToDescriptor.insert({ descriptor.filepath, { descriptorFilepath, descriptor.id } });

				// we associate resource id with a name.
				assetToDescriptor.insert({ descriptor.id, std::make_unique<AssetInfo<T>>(descriptor) });

				// for each asset, we associate it with a descriptor functor.
				serialiseDescriptorFunctors.insert({ descriptor.id, std::make_unique<SerialiseDescriptorFunctor<T>>() });

				// map each resource id to the original type.
				resourceToType.insert({ descriptor.id, Family::id<T>() });
			}
		}

		Logger::info("===========================\n");
	}(), ...);
}

template<ValidResource T>
ResourceID AssetManager::createResourceFile(AssetInfo<T> descriptor) {
	// this compiles a resource file corresponding to the intermediary asset.
	if (!compileIntermediaryFile<T>(descriptor)) {
		return INVALID_RESOURCE_ID;
	}

	DescriptorFilePath descriptorFilePath = AssetIO::getDescriptorFilename<T>(descriptor.id);
	ResourceFilePath resourceFilePath = AssetIO::getResourceFilename<T>(descriptor.id);

	// we can now add this resource to the resource manager.
	ResourceID id = resourceManager.addResourceFile<T>(resourceFilePath);

	if (id != INVALID_RESOURCE_ID) {
		// we update cache with the new time..
		updateAssetCache<T>(descriptor);

		// we record all encountered intermediary assets with corresponding filepaths.
		intermediaryAssetsToDescriptor.insert({ descriptor.filepath, { descriptorFilePath, descriptor.id } });

		// we associate resource id with a name.
		assetToDescriptor.insert({ id, std::make_unique<AssetInfo<T>>(descriptor) });

		// for each asset, we associate it with a descriptor functor.
		serialiseDescriptorFunctors.insert({ id, std::make_unique<SerialiseDescriptorFunctor<T>>() });

		// map each resource id to the original type.
		resourceToType.insert({ id, Family::id<T>() });
	}

	return id;
}

template<ValidResource T>
void AssetManager::serialiseDescriptor(ResourceID id) {
	std::ofstream descriptorFile{ AssetIO::getDescriptorFilename<T>(id) };

	if (!descriptorFile) {
		Logger::error("Failure to serialise descriptor file of: {}", AssetIO::getDescriptorFilename<T>(id).string);
		return;
	}

	// retrieve asset info..
	std::unique_ptr<BasicAssetInfo> const& basicAssetInfoPtr = assetToDescriptor[id];
	assert(basicAssetInfoPtr && "Should never be nullptr");
	
	AssetInfo<T>* assetInfo = static_cast<AssetInfo<T>*>(basicAssetInfoPtr.get());
	assert(assetInfo && "Should also never be nullptr");

	// serialise basic asset info..
	// 1st line -> asset id, 2nd line -> name, 3rd line -> asset filepath.

	// calculate relative path to the Assets directory.
	std::filesystem::path relativePath = std::filesystem::relative(std::filesystem::path{ assetInfo->filepath }, AssetIO::assetDirectory);
	descriptorFile << static_cast<std::size_t>(assetInfo->id) << '\n' << assetInfo->name << '\n' << relativePath.string() << '\n';

	// ============================
	// Filestream is now pointing at the 4th line.
	// Do any serialisation for specifc metadata of asset type here!!
	// ============================
	if constexpr (std::same_as<T, Texture>) {
		descriptorFile << magic_enum::enum_name(assetInfo->compression) << '\n';
	}
	else if constexpr (std::same_as<T, CustomShader>) {
		descriptorFile << magic_enum::enum_name(assetInfo->pipeline) << '\n';
	}

	// ============================
	Logger::info("Successfully serialised descriptor file for {}", assetInfo->filepath.string);
}