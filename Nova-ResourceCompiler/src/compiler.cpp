#include "compiler.h"

#include "texture.h"
#include "model.h"
#include "cubemap.h"
#include "scriptAsset.h"
#include "audio.h"

#include "modelLoader.h"

#include <vector>
#include <iostream>

namespace {
	template <typename T>
	void writeBytesToFile(std::ofstream& resourceFile, T const& data) {
		resourceFile.write(reinterpret_cast<const char*>(&data), sizeof(data));
	}
}
void Compiler::compileTexture(std::ofstream& resourceFile, std::string const& intermediaryAssetFilepath) {

}

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

void Compiler::compileModel(std::ofstream& resourceFile, std::string const& intermediaryAssetFilepath) {
	auto optModelData = ModelLoader::loadModel(intermediaryAssetFilepath);

	if (!optModelData) {
		return;
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
}

void Compiler::compileCubeMap(std::ofstream& resourceFile, std::string const& intermediaryAssetFilepath) {
}

void Compiler::compileScriptAsset(std::ofstream& resourceFile, std::string const& intermediaryAssetFilepath) {
}

void Compiler::compileAudio(std::ofstream& resourceFile, std::string const& intermediaryAssetFilepath) {
}

void Compiler::compile(std::filesystem::path const& descriptorFilepath) {
	// Verify asset type.
	std::string resourceType = descriptorFilepath.parent_path().stem().string();

	if (resourceType == "Texture") {
		Compiler::compileAsset<Texture>(descriptorFilepath);
	}
	else if (resourceType == "Model") {
		Compiler::compileAsset<Model>(descriptorFilepath);
	}
	else if (resourceType == "CubeMap") {
		Compiler::compileAsset<CubeMap>(descriptorFilepath);
	}
	else if (resourceType == "ScriptAsset") {
		Compiler::compileAsset<ScriptAsset>(descriptorFilepath);
	}
	else if (resourceType == "Audio") {
		Compiler::compileAsset<Audio>(descriptorFilepath);
	}
	else {
		Logger::warn("Unable to determine asset type of descriptor {}", descriptorFilepath.string());
		return;
	}
}

void Compiler::test() {
	std::vector descriptorPaths {
		"C:\\Users\\Nyove\\Desktop\\nova\\Descriptors\\Texture\\Farm_Table_Low_Table_frame_mtl_BaseColor.desc",
		"C:\\Users\\Nyove\\Desktop\\nova\\Descriptors\\Model\\Farm_Table_Low.desc",
		"C:\\Users\\Nyove\\Desktop\\nova\\Descriptors\\Model\\box.desc",
		"C:\\Users\\Nyove\\Desktop\\nova\\Descriptors\\Model\\smallbox.desc",
		"C:\\Users\\Nyove\\Desktop\\nova\\Descriptors\\Audio\\SFX_AudioTest1.desc",
		"C:\\Users\\Nyove\\Desktop\\nova\\Descriptors\\CubeMap\\citrus_orchard_road_puresky_2k.desc",
		"C:\\Users\\Nyove\\Desktop\\nova\\Descriptors\\ScriptAsset\\TestScript.desc"
	};

	for (auto&& filepath : descriptorPaths) {
		compile(filepath);
	}
}