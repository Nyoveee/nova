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

	Logger::debug("Running command: {}", command);

	if (std::system(command.c_str())) {
		Logger::error("Error compiling {}", descriptorFilePath.string);
		return false;
	}
	else {
		Logger::debug("Successful compiling {}", descriptorFilePath.string);
		Logger::debug("Resource file created: {}", AssetIO::getResourceFilename<T>(descriptor.id).string);
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

	long long cachedAssetDuration;
	long long cachedDescriptorDuration;

	// Attempt to read the time..
	try {
		std::string durationInString;
		std::getline(cacheFile, durationInString);
		cachedAssetDuration = std::stoull(durationInString);

		std::getline(cacheFile, durationInString);
		cachedDescriptorDuration = std::stoull(durationInString);
	}
	catch (std::exception const&) {
		// Failed to read cache file.. invalid..
		Logger::error("Failed to read cache file for {}", static_cast<std::size_t>(descriptor.id));
		return true;
	}

	// We compare our cached asset duration and see if it matches with the filepath...
	auto assetLastWriteTime = std::filesystem::last_write_time(descriptor.filepath);
	auto assetEpoch = std::chrono::duration_cast<std::chrono::milliseconds>(assetLastWriteTime.time_since_epoch());
	long long assetDuration = assetEpoch.count();

	// duration is different..
	if (cachedAssetDuration != assetDuration) {
		return true;
	}

	// We also compare our cached descriptor duration and see if it matches with the descriptor..
	auto descriptorLastWriteTime = std::filesystem::last_write_time(AssetIO::getDescriptorFilename<T>(descriptor.id));
	auto descriptorEpoch = std::chrono::duration_cast<std::chrono::milliseconds>(descriptorLastWriteTime.time_since_epoch());
	long long descriptorDuration = descriptorEpoch.count();

	return cachedDescriptorDuration != descriptorDuration;
}

template<ValidResource T>
void AssetManager::updateAssetCache(AssetInfo<T> const& descriptor) const {
	auto cacheFilePath = AssetIO::getAssetCacheFilename<T>(descriptor.id);

	std::ofstream cacheFile{ cacheFilePath };

	if (!cacheFile) {
		Logger::error("Unable to update asset cache file.");
		return;
	}

	// Get asset last write time..
	auto assetLastWriteTime = std::filesystem::last_write_time(descriptor.filepath);
	auto assetEpoch = std::chrono::duration_cast<std::chrono::milliseconds>(assetLastWriteTime.time_since_epoch());
	long long assetDuration = assetEpoch.count();

	// Get descriptor last write time..
	auto descriptorLastWriteTime = std::filesystem::last_write_time(AssetIO::getDescriptorFilename<T>(descriptor.id));
	auto descriptorEpoch = std::chrono::duration_cast<std::chrono::milliseconds>(descriptorLastWriteTime.time_since_epoch());
	long long descriptorDuration = descriptorEpoch.count();

	cacheFile << assetDuration << '\n' << descriptorDuration << '\n';
	Logger::debug("Successfully updated {} cache.", descriptor.filepath.string);
}

template<ValidResource T>
void AssetManager::loadSystemResourceDescriptor(std::unordered_map<ResourceID, ResourceFilePath> const& systemResources) {
	for (auto&& [id, filepath] : systemResources) {
		DescriptorFilePath descriptorFilePath = std::filesystem::path{ filepath }.replace_extension(".desc");
		
		auto optional = AssetIO::parseDescriptorFile<T>(descriptorFilePath, AssetIO::systemResourceDirectory);

		AssetInfo<T> descriptor;
		
		if (optional) {
			descriptor = optional.value();
		}
		else {
			// descriptor.id = id;
			descriptor.name = static_cast<std::filesystem::path>(filepath).filename().string();
		}

		descriptor.id = id;
		assetToDescriptor.insert({ id, std::make_unique<AssetInfo<T>>(descriptor) });
		systemResourcesId.insert(id);
	}
}

template<ValidResource T>
void AssetManager::serializeAllResources() {
	for (auto const& resourceId : resourceManager.getAllResources<T>()) {
		serialiseResource<T>(resourceId);
	}
}

template<ValidResource ...T>
void AssetManager::loadAllDescriptorFiles() {
	([&] {
		auto iterator = AssetIO::subDescriptorDirectories.find(Family::id<T>());
		assert(iterator != AssetIO::subDescriptorDirectories.end() && "This descriptor sub directory is not recorded.");
		std::filesystem::path const& directory = iterator->second;

		Logger::debug("===========================");
		Logger::debug("Loading all {} descriptors.", directory.stem().string());
		Logger::debug("----");

		// recursively iterate through a directory and parse all descriptor files.
		for (const auto& entry : std::filesystem::recursive_directory_iterator{ directory }) {
			std::filesystem::path descriptorFilepath = entry.path();

			if (!entry.is_regular_file()) {
				continue;
			}

			if (descriptorFilepath.extension() != ".desc") {
				continue;
			}

			Logger::debug("Parsing {}..", descriptorFilepath.string());

			// We attempt to parse the descriptor file, to find out the original asset it is referencing...
			std::optional<AssetInfo<T>> opt = AssetIO::parseDescriptorFile<T>(descriptorFilepath);
			AssetInfo<T> descriptor;

			if (!opt) {
				Logger::debug("Parsing failed, removing invalid descriptor file..\n");
				std::filesystem::remove(descriptorFilepath);
				continue;
			}
			// We check the validity of descriptor file, whether the asset path is pointing to a valid file.
			else if (!std::filesystem::is_regular_file(std::filesystem::path{ opt.value().filepath })) {
				Logger::debug("Descriptor file is pointing to an invalid asset filepath, removing descriptor file..\n");
				std::filesystem::remove(descriptorFilepath);
				continue;
			}
			else {
				Logger::debug("Successfully parsed descriptor file.");
				descriptor = opt.value();
			}
			
			if (hasAssetChanged<T>(descriptor)) {
				Logger::debug("Asset has changed, recompiling it's corresponding resource file..\n");
				resourceManager.removeResource(descriptor.id);
				createResourceFile<T>(descriptor);
			}
			// If it does, we check if the resource manager already have this particular resource loaded.
			// If it doesn't exist, it means that this resource file is missing / invalid.
			// Let's compile the corresponding intermediary asset file and load it to the resource manager.
			else if (!resourceManager.doesResourceExist(descriptor.id)) {
				Logger::debug("Corresponding resource file does not exist, compiling intermediary asset {}\n", descriptor.filepath.string);
				createResourceFile<T>(descriptor);
			}
			else {
				BasicAssetInfo assetInfo;
				assetInfo.id = descriptor.id;

				Logger::debug("A valid resource file exist for this descriptor and asset.\n");

				// we record all encountered intermediary assets with corresponding filepaths.
				intermediaryAssetsToDescriptor.insert({ descriptor.filepath, { descriptorFilepath, std::move(assetInfo) } });

				// we associate resource id with a name.
				assetToDescriptor.insert({ descriptor.id, std::make_unique<AssetInfo<T>>(descriptor) });

				// for each asset, we associate it with a descriptor functor.
				serialiseDescriptorFunctors.insert({ descriptor.id, std::make_unique<SerialiseDescriptorFunctor<T>>() });

				// map each resource id to the original type.
				resourceToType.insert({ descriptor.id, Family::id<T>() });
			}
		}

		Logger::debug("===========================\n");
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
		BasicAssetInfo assetInfo;
		assetInfo.id = descriptor.id;

		// we update cache with the new time..
		updateAssetCache<T>(descriptor);

		// we record all encountered intermediary assets with corresponding filepaths.
		intermediaryAssetsToDescriptor.insert({ descriptor.filepath, { descriptorFilePath, std::move(assetInfo) } });

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
void AssetManager::serialiseResource(ResourceID resourceId) {
	if constexpr (
			!std::same_as<T, Controller>
		&&	!std::same_as<T, Sequencer>
		&&	!std::same_as<T, Material>
		&&	!std::same_as<T, CustomShader>
	) {
		// other resource type are not edited by the editor, therefore asset manager has no responsiblity in serialising them.
		return;
	}
	else {
		if (systemResourcesId.contains(resourceId)) {
			return;
		}

		T* resource = resourceManager.getResourceOnlyIfLoaded<T>(resourceId);

		if (!resource) {
			return;
		}

		// get descriptor..
		auto iterator = assetToDescriptor.find(resourceId);

		if (iterator == assetToDescriptor.end()) {
			Logger::error("Failed to serialise {}", static_cast<std::size_t>(resourceId));
			return;
		}

		auto&& [__, descriptor] = *iterator;

		std::ofstream outputFile = [&]() -> std::ofstream {
			// Controller, material and sequencer wants to overwrite the original asset file..
			if constexpr (std::same_as<T, Controller> || std::same_as<T, Material> || std::same_as<T, Sequencer>) {
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
			return;
		}

		// get resource file..
		if constexpr (std::same_as<T, Controller>) {
			Serialiser::serializeToJsonFile(resource->data, outputFile);
			Logger::debug("Serialised controller: {}", static_cast<std::size_t>(resourceId));
		}
		else if constexpr (std::same_as<T, Sequencer>) {
			Serialiser::serializeToJsonFile(resource->data, outputFile);
			Logger::debug("Serialised sequencer: {}", static_cast<std::size_t>(resourceId));
		}
		else if constexpr (std::same_as<T, Material>) {
			Serialiser::serializeToJsonFile(resource->materialData, outputFile);
			Logger::debug("Serialised material: {}", static_cast<std::size_t>(resourceId));
		}
		else if constexpr (std::same_as<T, CustomShader>) {
			Serialiser::serializeToJsonFile(resource->customShaderData, outputFile);
			Logger::debug("Serialised shader: {}", static_cast<std::size_t>(resourceId));
		}
		else {
			static_assert(dependent_false<T> && "Unhandled serialisation case" __FUNCSIG__);
			return {};
		}
	}
}

template<ValidResource T>
void AssetManager::serializeDescriptor(ResourceID id) {
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
	descriptorFile << static_cast<std::size_t>(assetInfo->id) << '\n' << relativePath.string() << '\n';

	// ============================
	// Filestream is now pointing at the 4th line.
	// Do any serialisation for specifc metadata of asset type here!!
	// ============================
	if constexpr (std::same_as<T, Texture>) {
		descriptorFile << magic_enum::enum_name(assetInfo->compression) << '\n';
		descriptorFile << magic_enum::enum_name(assetInfo->type) << '\n';
	}
	else if constexpr (std::same_as<T, CustomShader>) {
		descriptorFile << magic_enum::enum_name(assetInfo->pipeline) << '\n';
	}
	else if constexpr (std::same_as<T, Font>) {
		descriptorFile << assetInfo->fontSize << '\n';
	}
	else if constexpr (std::same_as<T, Model>) {
		descriptorFile << assetInfo->scale << '\n';

		for (auto&& socket : assetInfo->sockets) {
			descriptorFile << socket << ' ';
		}

		descriptorFile << '\n';

		for (auto&& material : assetInfo->materials) {
			descriptorFile << static_cast<std::size_t>(material) << ' ';
		}

		descriptorFile << '\n';
	}

	// ============================
	Logger::debug("Successfully serialised descriptor file for {}", assetInfo->filepath.string);
}