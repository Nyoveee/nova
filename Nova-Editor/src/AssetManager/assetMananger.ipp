#include <sstream>
#include <cstdlib>

#include "assetManager.h"
#include "Profiling.h"
#include "ResourceManager/resourceManager.h"
#include "assetIO.h"

template<ValidResource T>
void AssetManager::compileIntermediaryFile(AssetInfo<T> const& descriptor) {
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
		}
		else {
			Logger::debug("Successful compiling {}", descriptorFilePath.string);
			Logger::debug("Resource file created: {}", AssetIO::getResourceFilename<T>(descriptor.id).string);
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
				Logger::debug("Sucessfully parsed descriptor file.");
				descriptor = opt.value();
			}

			auto assetLastWriteTime = std::filesystem::last_write_time(std::filesystem::path{ descriptor.filepath });
			auto assetEpoch = std::chrono::duration_cast<std::chrono::milliseconds>(assetLastWriteTime.time_since_epoch());

			// doesnt match, asset has changed..
			if (assetEpoch != descriptor.timeLastWrite) {
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
				Logger::debug("A valid resource file exist for this descriptor and asset.\n");
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

		Logger::debug("===========================\n");
	}(), ...);
}

template<ValidResource T>
ResourceID AssetManager::createResourceFile(AssetInfo<T> descriptor) {
	// this compiles a resource file corresponding to the intermediary asset.
	compileIntermediaryFile<T>(descriptor);

	DescriptorFilePath descriptorFilePath = AssetIO::getDescriptorFilename<T>(descriptor.id);
	ResourceFilePath resourceFilePath = AssetIO::getResourceFilename<T>(descriptor.id);

	// we can now add this resource to the resource manager.
	ResourceID id = resourceManager.addResourceFile<T>(resourceFilePath);

	if (id != INVALID_RESOURCE_ID) {
		// we update descriptor with the new time..
		auto assetLastWriteTime = std::filesystem::last_write_time(std::filesystem::path{ descriptor.filepath });
		auto assetEpoch = std::chrono::duration_cast<std::chrono::milliseconds>(assetLastWriteTime.time_since_epoch());
		descriptor.timeLastWrite = assetEpoch;

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
	// 1st line -> asset id, 2nd line -> name, 3rd line -> asset filepath, 4th epoch time / last write time.

	// calculate relative path to the Assets directory.
	std::filesystem::path relativePath = std::filesystem::relative(std::filesystem::path{ assetInfo->filepath }, AssetIO::assetDirectory);
	descriptorFile << static_cast<std::size_t>(assetInfo->id) << '\n' << assetInfo->name << '\n' << relativePath.string() << '\n' << assetInfo->timeLastWrite.count() << '\n';

	// ============================
	// Filestream is now pointing at the 5th line.
	// Do any serialisation for specifc metadata of asset type here!!
	// ============================

	// ============================
	Logger::debug("Successfully serialised descriptor file for {}", assetInfo->filepath.string);
}