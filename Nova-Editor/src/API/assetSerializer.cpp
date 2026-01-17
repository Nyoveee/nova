#include "assetSerializer.h"
#include "resource.h"
#include "assetIO.h"

#include "stb_image_write.h"

#include <glad/glad.h>

namespace AssetSerializer {
	void flipImageData(std::vector<float>& image, int width, int height, int components) {
		int rowsToProcess = height / 2; // i want flooring here.
		int rowStride = width * components;

		std::vector<float> buffer;
		buffer.resize(rowStride);

		int sizeOfWidthInBytes = rowStride * sizeof(float);

		for (int i = 0; i < rowsToProcess; ++i) {
			// top row
			int topRowOffset = i * rowStride;

			// bottom row
			int bottomRowOffset = (height - 1 - i) * rowStride;

			// copy bottom row to temporary buffer..
			std::memcpy(buffer.data(), image.data() + bottomRowOffset, sizeOfWidthInBytes);

			// copy top row to bottom row..
			std::memcpy(image.data() + bottomRowOffset, image.data() + topRowOffset, sizeOfWidthInBytes);

			// copy buffer to top row..
			std::memcpy(image.data() + topRowOffset, buffer.data(), sizeOfWidthInBytes);
		}
	}

	void serialiseCubeMap(CubeMap const& cubeMap) {
		static const std::string temporaryDirectory = ".temp";
		constexpr int channels = 3; // RGB

		static const std::array<std::string, 6> cubemapFaceFileNames {
			"positive_x.hdr",
			"negative_x.hdr",
			"positive_y.hdr",
			"negative_y.hdr",	// WE FLIP Y HERE BECAUSE OPENGL LOADS THEM BOTTOM UP.
			"positive_z.hdr",
			"negative_z.hdr"
		};

		// 1. Create a temporary directory..
		std::filesystem::create_directory(temporaryDirectory);
		
		GLint oldAlignment;
		glGetIntegerv(GL_PACK_ALIGNMENT, &oldAlignment);

		glPixelStorei(GL_PACK_ALIGNMENT, 1);

		// store the all the full filepaths..
		std::array<std::string, 6> fullFilePaths;

		// For each face.. copy its data into stbi image..
		// Create 6 different png files, to be used to feed into texassemble.exe..
		for (std::size_t face = 0; face < 6; ++face) {
			std::filesystem::path filepath = std::filesystem::current_path() / temporaryDirectory / cubemapFaceFileNames[face];
			fullFilePaths[face] = "\"" + filepath.string() + "\" ";

			// 1. Allocate enough buffer to store result..
			std::vector<float> buffer;
			buffer.resize(cubeMap.getWidth() * cubeMap.getHeight() * channels);

			// 2. Download only ONE face (depth = 1) at the correct Z-offset
			glGetTextureSubImage(
				cubeMap.getTextureId(),
				0,												// mipmap level
				0, 0, static_cast<GLint>(face),					// Z-offset is the face index
				cubeMap.getWidth(), cubeMap.getHeight(), 1,		// Download 1 face at a time
				GL_RGB,
				GL_FLOAT,										// natively this cubemap is a HALF_FLoat, but stb requires 32 bit floating point.
				static_cast<GLsizei>(buffer.size() * sizeof(float)),
				buffer.data()
			);

			// Flip image in the y direction.
			flipImageData(buffer, cubeMap.getWidth(), cubeMap.getHeight(), 3);

			// 3. Serialise as .HDR
			if (!stbi_write_hdr(filepath.string().c_str(), cubeMap.getWidth(), cubeMap.getHeight(), channels, buffer.data())) {
				Logger::error("Failed to serialise cubemap");
				return;
			}
		}

		// Use texassemble.exe to convert the 6 .hdr files into one combined .dds file..
		constexpr const char* executableName = "texassemble.exe";
		std::filesystem::path texAssemblePath = std::filesystem::current_path() / "ExternalApplication" / executableName;
		std::filesystem::path outputFileName = AssetIO::assetDirectory / (Logger::getUniqueTimedId() + ".dds");

		// command line arguments..
		std::string commandLineArguments = "cube -o \"" + outputFileName.string() + "\" ";

		// also flip the y cube faces..
		std::swap(fullFilePaths[2], fullFilePaths[3]);

		for (auto const& filepath : fullFilePaths) {
			commandLineArguments += filepath;
		}

		// https://stackoverflow.com/questions/27975969/how-to-run-an-executable-with-spaces-using-stdsystem-on-windows/27976653#27976653
		std::string command = 
			"\""										// outer quotation encapsulating everything..
			"\"" + texAssemblePath.string() + "\" " 
			+ commandLineArguments +
			"\"";										// outer quotation encapsulating everything..

		Logger::debug("Running command: {}", command);

		if (std::system(command.c_str())) {
			Logger::error("Error baking cubemap.");
		}
		else {
			Logger::debug("Successful baked cube map.");
		}

		glPixelStorei(GL_PACK_ALIGNMENT, oldAlignment);
	}
}