#include "compiler.h"
#include "assetIO.h"

#include "Material.h"

#include <filesystem>

void Compiler::recompileAllSystemAssets() {
	for (const auto& entry : std::filesystem::recursive_directory_iterator{ AssetIO::systemResourceDirectory }) {
		std::filesystem::path path = entry.path();

		if (!std::filesystem::is_regular_file(path)) {
			continue;
		}

		auto fileExtension = path.extension();

		auto compileAsset = [&]<typename T>() {
			// attempt to find the corresponding descriptor.. same name, different extension.
			DescriptorFilePath descriptorFilePath = std::filesystem::path{ path }.replace_extension(".desc");
			std::optional<AssetInfo<T>> descriptorOpt = AssetIO::parseDescriptorFile<T>(descriptorFilePath, AssetIO::systemResourceDirectory);

			AssetInfo<T> descriptor = [&]() {
				if (descriptorOpt)	{ return descriptorOpt.value(); }
				else				{ return AssetIO::createSystemDescriptorFile<T>(path, descriptorFilePath); }
			}();

			// Get appropriate resource file path.
			ResourceFilePath resourceFilePath = std::filesystem::path{ path }.replace_extension("");

			Logger::info("Compiling.. {}", path.string());
			int result = Compiler::compileAsset<T>(descriptor, resourceFilePath);
			
			assert(result == 0 && "Ensure validity of your system resources.");
		};

		if (fileExtension == ".fbx") {
			compileAsset.template operator()<Model>();
		}
		else if (fileExtension == ".png") {
			compileAsset.template operator()<Texture>();
		}
		else if (fileExtension == ".shader") {
			compileAsset.template operator()<CustomShader>();
		}
		else if (fileExtension == ".material") {
			compileAsset.template operator()<Material>();
		}
		else {
			// Logger::warn("Unsupported file type of: {} has been found.", path.string());
		}
	}
}