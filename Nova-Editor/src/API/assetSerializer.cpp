#include <DirectXTex/DirectXTex.h>
#include <glad/glad.h>

#include "stb_image_write.h"
#include "resource.h"
#include "assetIO.h"
#include "assetSerializer.h"


namespace AssetSerializer {
#if 1
	void flipImageData(std::vector<unsigned char>& image, int width, int height, int components) {
		int rowsToProcess = height / 2; // i want flooring here.
		int rowStride = width * components * 2;

		std::vector<unsigned char> buffer;
		buffer.resize(rowStride);

		int sizeOfWidthInBytes = rowStride;

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
#endif

	void serialiseCubeMap(CubeMap const& cubeMap) {
		DirectX::ScratchImage cubemapImage;

		HRESULT initialisationResult = cubemapImage.InitializeCube(
			DXGI_FORMAT_R16G16B16A16_FLOAT,
			cubeMap.getWidth(),
			cubeMap.getHeight(),
			1,
			cubeMap.getMipmap()
		);

		if (FAILED(initialisationResult)) {
			Logger::error("Failed to initialise DirectX cubemap {}.", static_cast<unsigned int>(initialisationResult));
			return;
		}

		GLint oldAlignment;
		glGetIntegerv(GL_PACK_ALIGNMENT, &oldAlignment);
		glPixelStorei(GL_PACK_ALIGNMENT, 1);

		static const std::string temporaryDirectory = ".temp";
		constexpr int channels = 4; // RGBA

		static const std::array<std::string, 6> cubemapFaceFileNames{
			"positive_x",
			"negative_x",
			"positive_y",
			"negative_y",
			"positive_z",
			"negative_z"
		};

		int width = cubeMap.getWidth();
		int height = cubeMap.getHeight();

		for (int mipmapLevel = 0; mipmapLevel < cubeMap.getMipmap(); ++mipmapLevel) {
			for (std::size_t face = 0; face < 6; ++face) {

				// 1. Allocate enough buffer to store result..
				std::vector<unsigned char> buffer;
				buffer.resize(width * height * channels * 2);

				// 2. Download only ONE face (depth = 1) at the correct Z-offset
				glGetTextureSubImage(
					cubeMap.getTextureId(),
					mipmapLevel,								// mipmap level
					0, 0, static_cast<GLint>(face),				// Z-offset is the face index
					width, height, 1,							// Download 1 face at a time
					GL_RGBA,
					GL_HALF_FLOAT,								// this cubemap is a HALF_FLoat
					static_cast<GLsizei>(buffer.size()),
					buffer.data()
				);

				std::size_t faceToSave = face;
#if 1
				// Flip image in the y direction.
				flipImageData(buffer, width, height, channels);

				// we flip y face..
				if (face == 2) {		// 2, POSITIVE_Y
					faceToSave = 3;		// 3, NEGATIVE_Y
				}
				else if (face == 3) {	// 3, NEGATIVE_Y
					faceToSave = 2;		// 2, POSITIVE_Y
				}
#endif
				// Copy data into directx subsection of the image..
				DirectX::Image const* img = cubemapImage.GetImage(mipmapLevel, faceToSave, 0);
				std::memcpy(img->pixels, buffer.data(), buffer.size());
			}

			width /= 2;
			height /= 2;
		}

		glPixelStorei(GL_PACK_ALIGNMENT, oldAlignment);

		std::filesystem::path outputFileName = AssetIO::assetDirectory / (Logger::getUniqueTimedId() + ".dds");
	
		HRESULT saveResult = DirectX::SaveToDDSFile(
			cubemapImage.GetImages(),
			cubemapImage.GetImageCount(),
			cubemapImage.GetMetadata(),
			DirectX::DDS_FLAGS_NONE,
			outputFileName.c_str()
		);

		if (FAILED(saveResult)) {
			Logger::error("Failed to bake irradiance map {}.", static_cast<unsigned int>(saveResult));
		}
	}
}