#include <vector>
#include <iostream>

#include <gli/gli.hpp>

#include "compiler.h"

#include "modelLoader.h"

// Y* only libraries.
#include "Library/stb_image.hpp"
// Internal Libraries
#include "Internal/ShaderParser.h"
namespace {
	template <typename T>
	void writeBytesToFile(std::ofstream& resourceFile, T const& data) {
		resourceFile.write(reinterpret_cast<const char*>(&data), sizeof(data));
	}
}

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

#if 0
	int width, height, numChannels;
	stbi_uc* data = stbi_load(intermediaryAssetFilepath.string.c_str(), &width, &height, &numChannels, 0);

	if (!data) {
		Logger::error("Failed to load texture! Filepath provided: {}", intermediaryAssetFilepath.string);
		return -1;
	}

	enum gli::format format = gli::format::FORMAT_RGBA8_SRGB_PACK8;

	if (numChannels == 1) {
		format = gli::format::FORMAT_R8_UNORM_PACK8;
	}
	else if (numChannels == 3) {
		format = gli::format::FORMAT_RGB8_UNORM_PACK8;
	}

	// Create a GLI 2D texture with the same dimensions
	gli::texture2d tex{
		format,
		gli::extent2d(width, height)
	};

	// Copy pixel data into the GLI texture
	std::memcpy(tex.data(), data, width * height * numChannels);
	stbi_image_free(data);

	gli::save_dds(tex, resourceFilePath.string);
#endif
	return 0;
}

#if 0
int Compiler::compileCubeMap(ResourceFilePath const& resourceFilePath, AssetFilePath const& intermediaryAssetFilepath) {
	float* data; // width * height * RGBA

	int width;
	int height;
	const char* err = nullptr; // or nullptr in C++11

	int status = LoadEXR(&data, &width, &height, intermediaryAssetFilepath.string.c_str(), &err);

	if (status != TINYEXR_SUCCESS) {
		if (err) {
			Logger::error("ERR : {}", err);
			FreeEXRErrorMessage(err); // release memory of error message.
		}

		return -1;
	}

	std::size_t size = 4 * sizeof(float) * static_cast<size_t>(width) * static_cast<size_t>(height);

	enum gli::format format = gli::format::FORMAT_RGBA32_SFLOAT_PACK32;

	// Create a GLI 2D texture with the same dimensions
	gli::texture2d tex{
		format,
		gli::extent2d(width, height)
	};

	// Copy pixel data into the GLI texture
	std::memcpy(tex.data(), data, size);
	stbi_image_free(data);

	gli::save_dds(tex, resourceFilePath.string);

#if 0
	// im assuming GL_RGBA16F format.
	// @TODO: actually check the header file of OpenEXR.
	glCreateTextures(GL_TEXTURE_2D, 1, &textureId);

	glTextureStorage2D(textureId, 1, GL_RGBA16F, width, height);
	glTextureSubImage2D(textureId, 0, 0, 0, width, height, GL_RGBA, GL_FLOAT, data);

	glTextureParameteri(textureId, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(textureId, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTextureParameteri(textureId, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameteri(textureId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenerateTextureMipmap(textureId);
	free(data); // release memory of image data

	this->width = width;
	this->height = height;

	loadStatus = Asset::LoadStatus::Loaded;
	//TracyAlloc(this, sizeof(*this));
#endif

	return 0;
}
#endif

// =================================
// Model file format
// - First 8 bytes => indicate number of meshes, M
// - Next series of bytes => M * mesh file format.
// 
// Mesh file format
// - First 8 bytes => indicate number of vertices, V
// - Next 8 bytes => indicate number of indices, I
// - Next 8 bytes => indicate the number of characters that make up the material name, C
// - Next 4 bytes => indicate the number of triangles.
// - Next V * (12 pos + 8 TU + 12 normal + 12 tangent + 12 bitangent) bytes => series of vertices data
// - Next 1 byte => null terminator. used for error checking to make sure it matches V.
// - Next I * 4 bytes => series of indices data.
// - Next 1 byte => null terminator. used for error checking to make sure it matches I.
// - Next C bytes => series of characters representing material name.
// - Next 1 byte => null terminator. used for error checking and to end the mesh file format.
// =================================

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

	ModelLoader::ModelData modelData = optModelData.value();

	// serialise number of meshes
	writeBytesToFile(resourceFile, modelData.meshes.size());

	// serialise the meshes.
	for (auto&& mesh : modelData.meshes) {
		// serialise number of vertices, indices, num of characters making up a material name, and num of triangles.
		writeBytesToFile(resourceFile, mesh.vertices.size());
		writeBytesToFile(resourceFile, mesh.indices.size());
		writeBytesToFile(resourceFile, mesh.materialName.size());
		writeBytesToFile(resourceFile, mesh.numOfTriangles);
		
		// serialise the vertices.
		for (auto&& vertex : mesh.vertices) {
			writeBytesToFile(resourceFile, vertex.pos.x);
			writeBytesToFile(resourceFile, vertex.pos.y);
			writeBytesToFile(resourceFile, vertex.pos.z);

			writeBytesToFile(resourceFile, vertex.textureUnit.x);
			writeBytesToFile(resourceFile, vertex.textureUnit.y);

			writeBytesToFile(resourceFile, vertex.normal.x);
			writeBytesToFile(resourceFile, vertex.normal.y);
			writeBytesToFile(resourceFile, vertex.normal.z);

			writeBytesToFile(resourceFile, vertex.tangent.x);
			writeBytesToFile(resourceFile, vertex.tangent.y);
			writeBytesToFile(resourceFile, vertex.tangent.z);

			writeBytesToFile(resourceFile, vertex.bitangent.x);
			writeBytesToFile(resourceFile, vertex.bitangent.y);
			writeBytesToFile(resourceFile, vertex.bitangent.z);
		}

		// null terminator.
		resourceFile.write("", 1);

		// serialise the indices.
		for (auto&& index : mesh.indices) {
			writeBytesToFile(resourceFile, index);
		}

		// null terminator.
		resourceFile.write("", 1);

		// serialise the materialName.
		resourceFile.write(mesh.materialName.data(), mesh.materialName.size());

		// null terminator.
		resourceFile.write("", 1);
	}

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

int Compiler::compileShaderAsset(ResourceFilePath const& resourceFilePath, AssetFilePath const& intermediaryAssetFilepath)
{
	std::ofstream resourceFile{ resourceFilePath.string, std::ios::binary };

	if (!resourceFile) {
		Logger::error("Failed to create resource file: {}. Compilation failed.", resourceFilePath.string);
		return -1;
	}
	ShaderParserData shaderParserData;
	if (!ShaderParser::Parse(intermediaryAssetFilepath, shaderParserData)) {
		Logger::error("Failed to create resource file: {}. Compilation failed.", resourceFilePath.string);
		return -1;
	}
	// Blending Config
	writeBytesToFile(resourceFile, shaderParserData.blendingConfig);
	resourceFile.write("", 1);
	// Depth Testing Method
	writeBytesToFile(resourceFile, shaderParserData.depthTestingMethod);
	resourceFile.write("", 1);
	// Uniforms
	writeBytesToFile(resourceFile, shaderParserData.uniforms.size());
	resourceFile.write("", 1);
	for (std::pair<std::string, std::string> uniform : shaderParserData.uniforms) {
		writeBytesToFile(resourceFile, uniform.first.size());
		writeBytesToFile(resourceFile, uniform.second.size());
		resourceFile.write(uniform.first.data(), uniform.first.size());
		resourceFile.write(uniform.second.data(), uniform.second.size());
		resourceFile.write("", 1);
	}
	// Fragment Shader Code
	writeBytesToFile(resourceFile, shaderParserData.fShaderCode.size());
	writeBytesToFile(resourceFile, shaderParserData.fShaderCode);
	resourceFile.write("", 1);
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
	// Verify asset type.
	std::string resourceType = std::filesystem::path{ descriptorFilepath }.parent_path().stem().string();
	if (resourceType == "Texture") {
		return Compiler::compileAsset<Texture>(descriptorFilepath);
	}
	else if (resourceType == "Model") {
		return Compiler::compileAsset<Model>(descriptorFilepath);
	}
	else if (resourceType == "CubeMap") {
		return Compiler::compileAsset<CubeMap>(descriptorFilepath);
	}
	else if (resourceType == "ScriptAsset") {
		return Compiler::compileAsset<ScriptAsset>(descriptorFilepath);
	}
	else if (resourceType == "Audio") {
		return Compiler::compileAsset<Audio>(descriptorFilepath);
	}
	else if (resourceType == "Scene") {
		return Compiler::compileAsset<Scene>(descriptorFilepath);
	}
	else if (resourceType == "NavMesh") {
		return Compiler::compileAsset<NavMesh>(descriptorFilepath);
	}
	else if (resourceType == "CustomShader") {
		return Compiler::compileAsset<CustomShader>(descriptorFilepath);
	}
	else {
		Logger::warn("Unable to determine asset type of descriptor {}, resourceType {}", descriptorFilepath.string, resourceType);
		return -1;
	}
}