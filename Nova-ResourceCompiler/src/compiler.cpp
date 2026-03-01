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

// Y* libraries
#include <ft2build.h>
#include FT_FREETYPE_H

int Compiler::compileTexture(ResourceFilePath const& resourceFilePath, AssetFilePath const& intermediaryAssetFilepath, AssetInfo<Texture>::Compression compressionFormat) {
	std::string format;
	std::string option;

	// https://learn.microsoft.com/en-us/windows/win32/api/dxgiformat/ne-dxgiformat-dxgi_format
	switch (compressionFormat)
	{
	case AssetInfo<Texture>::Compression::Uncompressed_Linear:
		format = "R8G8B8A8_UNORM";
		option = "--ignore-srgb";
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
		option = "--ignore-srgb";
		break;
	case AssetInfo<Texture>::Compression::BC3_SRGB:
		format = "BC3_UNORM_SRGB";
		option = "-srgb";
		break;
	case AssetInfo<Texture>::Compression::BC3_Linear:
		format = "BC3_UNORM";
		option = "--ignore-srgb";
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
		option = "--ignore-srgb";
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

int Compiler::compileFont(ResourceFilePath const& resourceFilePath, AssetFilePath const& intermediaryAssetFilepath, unsigned int fontSize) {
	// ============================
	// 1. Set up FT Library and load font..
	// ============================
	FT_Face face;
	FT_Library ft;

	// Initialize FreeType library
	if (FT_Init_FreeType(&ft)) {
		Logger::error("ERROR::FREETYPE: Could not init FreeType Library");
		return -1;
	}

	// Load font face using the stored fontPath
	if (FT_New_Face(ft, intermediaryAssetFilepath.string.c_str(), 0, &face)) {
		Logger::error("ERROR::FREETYPE: Failed to load font:");
		return -1;
	}

	FT_Set_Pixel_Sizes(face, 0, fontSize);
	
	unsigned int atlasWidth = 0;
	unsigned int atlasHeight = 0;
	std::vector<Font::Data::Sprite> sprites;
	std::unordered_map<char, Font::Character> characters;

	// stores the xOffset of each glyph from the start of the texture atlas.
	int xOffset = 0;

	// ============================
	// 2. Iterate through every ASCII character and generate its respective glyph bitmap..
	// ============================
	// First 32 chars for ascii are control characters
	for (unsigned char c = 32; c < 128; c++) {
	
		// load character glyph 
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
			Logger::error("ERROR::FREETYTPE: Failed to load Glyph for ASCII character: {}", c);
			continue;
		}

		FT_GlyphSlot glyph = face->glyph;

		// ============================
		// 2.1 Calculate atlas
		// @TODO: Cap texture at 4086 width, more than 1 row. ? is that good?
		// ============================
		atlasWidth += glyph->bitmap.width;
		atlasHeight = std::max(atlasHeight, glyph->bitmap.rows);

		// ============================
		// 2.2 Store the metadata of each character sprite, like size, bearing and advance..
		// ============================
		Font::Character character = {
			static_cast<float>(xOffset),	// WE STORE THE XOFFSET FIRST, SUBSEQUENTLY THIS WILL BE NORMALISED BASED ON ATLAS WIDTH.
			glm::ivec2(glyph->bitmap.width, glyph->bitmap.rows),
			glm::ivec2(glyph->bitmap_left, glyph->bitmap_top),
			glyph->advance.x >> 6
		};

		characters.insert({ c, character });
		xOffset += glyph->bitmap.width;

		// ============================
		// 2.3 Store the bytes of the glyph's bitmap into a vector..
		// ============================
		std::vector<unsigned char> bytes;
		bytes.resize(glyph->bitmap.width * glyph->bitmap.rows);
		
		std::memcpy(bytes.data(), glyph->bitmap.buffer, bytes.size());

		sprites.push_back({
			glyph->bitmap.width,
			glyph->bitmap.rows,
			std::move(bytes)
		});
	}

	// normalise texture unit offset in respect to atlas width..
	for (auto& [c, character] : characters) {
		character.tx /= atlasWidth;
	}

	// ============================
	// 3. Time to serialize.
	// ============================
	std::ofstream resourceFile{ resourceFilePath.string, std::ios::binary };

	if (!resourceFile) {
		Logger::error("Failed to create resource file: {}. Compilation failed.", resourceFilePath.string);
		return -1;
	}

	Font::Data data{
		std::move(sprites),
		std::move(characters),
		atlasWidth,
		atlasHeight,
		fontSize
	};

	serializeToBinary(resourceFile, data);

	return 0;
}

int Compiler::compileModel(ResourceFilePath const& resourceFilePath, AssetInfo<Model> descriptor) {
	std::ofstream resourceFile{ resourceFilePath.string, std::ios::binary };

	if (!resourceFile) {
		Logger::error("Failed to create resource file: {}. Compilation failed.", resourceFilePath.string);
		return -1;
	}

	auto optModelData = ModelLoader::loadModel(descriptor.filepath, descriptor.scale, std::move(descriptor.sockets));

	if (!optModelData) {
		return -1;
	}

	serializeToBinary(resourceFile, optModelData.value());

	return 0;
}


int Compiler::compileScriptAsset(ResourceFilePath const& resourceFilePath, AssetFilePath const& intermediaryAssetFilepath, bool adminScript, bool toExecuteEvenWhenPaused) {
	std::ofstream resourceFile{ resourceFilePath.string };

	if (!resourceFile) {
		Logger::error("Failed to create resource file: {}. Compilation failed.", resourceFilePath.string);
		return -1;
	}

	resourceFile << std::filesystem::path{ intermediaryAssetFilepath }.stem().string() << std::endl;
	resourceFile << adminScript << '\n' << toExecuteEvenWhenPaused;
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
	else if (resourceType == "EquirectangularMap") {
		return compileAsset.template operator()<EquirectangularMap>();
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
	else if (resourceType == "Font") {
		return compileAsset.template operator()<Font>();
	}
	else if (resourceType == "Prefab") {
		return compileAsset.template operator()<Prefab>();
	}
	else if (resourceType == "Sequencer") {
		return compileAsset.template operator()<Sequencer>();
	}
	else if (resourceType == "CubeMap") {
		return compileAsset.template operator()<CubeMap>();
	}
	else if (resourceType == "Video") {
		return compileAsset.template operator()<Video>();
	}
	else {
		Logger::warn("Unable to determine asset type of descriptor {}, resourceType {}", descriptorFilepath.string, resourceType);
		return -1;
	}
}
