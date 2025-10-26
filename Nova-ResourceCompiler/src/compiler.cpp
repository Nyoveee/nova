#include <vector>
#include <iostream>

#include <gli/gli.hpp>

#include <glm/gtc/type_ptr.hpp>
#include "compiler.h"

#include "modelLoader.h"
#include "Serialisation/serialisation.h"
#include "Serialisation/serializeToBinary.h"

#include "Material.h"

// Internal Libraries
#include "Internal/ShaderParser.h"

int Compiler::compileTexture(ResourceFilePath const& resourceFilePath, AssetFilePath const& intermediaryAssetFilepath, AssetInfo<Texture>::Compression compressionFormat) {
	std::string format;
	std::string option;

	// https://learn.microsoft.com/en-us/windows/win32/api/dxgiformat/ne-dxgiformat-dxgi_format
	switch (compressionFormat)
	{
	case AssetInfo<Texture>::Compression::Uncompressed_Linear:
		format = "R8G8B8A8_UNORM";
		break;
	case AssetInfo<Texture>::Compression::Uncompressed_SRGB:
		format = "R8G8B8A8_UNORM_SRGB";
		option = "-srgb";
		break;
	case AssetInfo<Texture>::Compression::BC1_SRGB:
		format = "BC1_UNORM_SRGB";
		option = "-srgb";
		break;
	case AssetInfo<Texture>::Compression::BC1_Linear:
		format = "BC1_UNORM";
		break;
	case AssetInfo<Texture>::Compression::BC3_SRGB:
		format = "BC3_UNORM_SRGB";
		option = "-srgb";
		break;
	case AssetInfo<Texture>::Compression::BC3_Linear:
		format = "BC3_UNORM";
		break;
	case AssetInfo<Texture>::Compression::BC4:
		format = "BC4_UNORM";
		break;
	case AssetInfo<Texture>::Compression::BC5:
		format = "BC5_UNORM";
		break;
	case AssetInfo<Texture>::Compression::BC6H:
		format = "BC6H_UF16";
		break;
	case AssetInfo<Texture>::Compression::BC7_SRGB:
		format = "BC7_UNORM_SRGB";
		option = "-srgb";
		break;
	case AssetInfo<Texture>::Compression::BC7_Linear:
		format = "BC7_UNORM";
		break;
	default:
		Logger::error("Unknown compression format specified.");
		return -1;
	}
	
	std::filesystem::path executableName = std::filesystem::current_path() / "ExternalApplication" / "texconv.exe";
	std::filesystem::path outputDirectory = AssetIO::resourceDirectory / "Texture";

	std::string commandLine = std::format(
		R"(""{}" {} -y -f {} -o "{}" "{}"")", executableName.string(), option, format, outputDirectory.string(), intermediaryAssetFilepath.string
	);

	if (std::system(commandLine.c_str())) {
		// command failed.
		return -1;
	}

	// remove old resource file if it exist..
	std::filesystem::remove(resourceFilePath);

	std::filesystem::path oldFilePath = 
		outputDirectory / 
		std::filesystem::path{ intermediaryAssetFilepath }.stem().replace_extension(".dds");

	// let's rename the resource.
	std::filesystem::rename(oldFilePath, resourceFilePath);
	return 0;
}

int Compiler::compileModel(ResourceFilePath const& resourceFilePath, AssetFilePath const& intermediaryAssetFilepath) {
	std::ofstream resourceFile{ resourceFilePath.string, std::ios::binary };

	if (!resourceFile) {
		Logger::error("Failed to create resource file: {}. Compilation failed.", resourceFilePath.string);
		return -1;
	}

	auto optModelData = ModelLoader::loadModel(intermediaryAssetFilepath);

	if (!optModelData) {
		return -1;
	}

	ModelData modelData = optModelData.value();

	// we utilise reflection to serialise our model..
	reflection::visit(
		[&](auto fieldData) {
			[[maybe_unused]] auto dataMember = fieldData.get();
			using DataMemberType = std::decay_t<decltype(dataMember)>;

			serializeToBinary<DataMemberType>(resourceFile, dataMember);
		}, 
	modelData);

	return 0;
}


int Compiler::compileScriptAsset(ResourceFilePath const& resourceFilePath, std::string className) {
	std::ofstream resourceFile{ resourceFilePath.string };

	if (!resourceFile) {
		Logger::error("Failed to create resource file: {}. Compilation failed.", resourceFilePath.string);
		return -1;
	}

	// script literally only stores the	class name. class name is part of the descriptor.
	resourceFile << className;

	return 0;
}

int Compiler::compileShaderAsset(ResourceFilePath const& resourceFilePath, AssetFilePath const& intermediaryAssetFilepath, Pipeline pipeline)
{
	std::ofstream resourceFile{ resourceFilePath.string };

	if (!resourceFile) {
		Logger::error("Failed to create resource file: {}. Compilation failed.", resourceFilePath.string);
		return -1;
	}

	CustomShader::ShaderParserData shaderParserData;

	if (!ShaderParser::Parse(intermediaryAssetFilepath, shaderParserData)) {
		Logger::error("Failed to create resource file: {}. Compilation failed.", resourceFilePath.string);
		return -1;
	}

	shaderParserData.pipeline = pipeline;
	Serialiser::serializeToJsonFile(shaderParserData, resourceFile);
	return 0;
}

int Compiler::defaultCompile(ResourceFilePath const& resourceFilePath, AssetFilePath const& intermediaryAssetFilepath) {
	std::ofstream resourceFile{ resourceFilePath.string, std::ios::binary };

	if (!resourceFile) {
		Logger::error("Failed to create resource file: {}. Compilation failed.", resourceFilePath.string);
		return -1;
	}

	std::ifstream assetFile{ intermediaryAssetFilepath.string, std::ios::binary };

	if (!assetFile) {
		Logger::error("Failed to load asset file: {}. Compilation failed.", resourceFilePath.string);
		return -1;
	}

	std::uintmax_t fileSize = std::filesystem::file_size(std::filesystem::path{ intermediaryAssetFilepath });
	std::array<char, 4096> buffer;

	std::size_t iterationCount = fileSize / buffer.size() + 1;

	for (std::size_t i = 0; i < iterationCount; ++i) {
		auto count = i + 1 != iterationCount ? buffer.size() : fileSize % buffer.size();

		assetFile.read(buffer.data(), count);
		resourceFile.write(buffer.data(), count);
	}

	return 0;
}

int Compiler::compile(DescriptorFilePath const& descriptorFilepath) {
	auto compileAsset = [&]<typename T>() {
		// Retrieve descriptor info.
		std::optional<AssetInfo<T>> optAssetInfo = AssetIO::parseDescriptorFile<T>(descriptorFilepath);

		if (!optAssetInfo) {
			Logger::error("Failed to parse descriptor file: {}. Compilation failed.", descriptorFilepath.string);
			return -1;
		}

		AssetInfo<T> const& assetInfo = optAssetInfo.value();

		return Compiler::compileAsset<T>(assetInfo, AssetIO::getResourceFilename<T>(assetInfo.id));
	};

	// Verify asset type.
	std::string resourceType = std::filesystem::path{ descriptorFilepath }.parent_path().stem().string();
	if (resourceType == "Texture") {
		return compileAsset.template operator()<Texture>();
	}
	else if (resourceType == "Model") {
		return compileAsset.template operator()<Model>();
	}
	else if (resourceType == "CubeMap") {
		return compileAsset.template operator()<CubeMap>();
	}
	else if (resourceType == "ScriptAsset") {
		return compileAsset.template operator()<ScriptAsset>();
	}
	else if (resourceType == "Audio") {
		return compileAsset.template operator()<Audio>();
	}
	else if (resourceType == "Scene") {
		return compileAsset.template operator()<Scene>();
	}
	else if (resourceType == "NavMesh") {
		return compileAsset.template operator()<NavMesh>();
	}
	else if (resourceType == "Controller") {
		return compileAsset.template operator()<Controller>();
	}
	else if (resourceType == "CustomShader") {
		return compileAsset.template operator()<CustomShader>();
	}
	else if (resourceType == "Material") {
		return compileAsset.template operator()<Material>();
	}
	else {
		Logger::warn("Unable to determine asset type of descriptor {}, resourceType {}", descriptorFilepath.string, resourceType);
		return -1;
	}
}
