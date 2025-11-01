#include <fstream>
#include <glm/gtc/type_ptr.hpp>

#include "loader.h"
#include "Logger.h"

#include "model.h"

#include "Serialisation/deserializeFromBinary.h"

std::optional<ResourceConstructor> ResourceLoader<Model>::load(ResourceID id, ResourceFilePath const& resourceFilePath) {
	std::ifstream resourceFile{ resourceFilePath.string, std::ios::binary };

	if (!resourceFile) {
		Logger::error("Failed to parse model.");
		return std::nullopt;
	}

	resourceFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	
	//try {
		ModelData modelData;
		
		reflection::visit(
			[&](auto&& fieldData) {
				auto&& dataMember = fieldData.get();
				using DataMemberType = std::decay_t<decltype(dataMember)>;

				deserializeFromBinary<DataMemberType>(resourceFile, dataMember);
			},
		modelData);
#if 0
		// =================================
		// Model file format
		// - First 8 bytes => indicate number of meshes, M
		// - Next series of bytes => M * mesh file format.
		// 
		// Mesh file format
		// - First 8 bytes => indicate number of vertices, V
		// - Next 8 bytes => indicate number of indices, I
		// - Next 8 bytes => indicate the number of characters that make up the material name, C1
		// - Next 8 bytes => indicate the number of characters that make up the mesh name, C2
		// - Next 4 bytes => indicate the number of triangles.
		// - Next V * (12 pos + 8 TU + 12 normal + 12 tangent + 12 bitangent) bytes => series of vertices data
		// - Next 1 byte => null terminator. used for error checking to make sure it matches V.
		// - Next I * 4 bytes => series of indices data.
		// - Next 1 byte => null terminator. used for error checking to make sure it matches I.
		// - Next C1 bytes => series of characters representing material name.
		// - Next 1 byte => null terminator. used for error checking and to make sure it matches C1
		// - Next C2 bytes => series of characters representing mesh name.
		// - Next 1 byte => null terminator. used for error checking and to end the mesh file format.
		// =================================
		
		// read M.
		std::size_t numOfMeshes;
		readFromFile(resourceFile, numOfMeshes);

		std::vector<Mesh> meshes;
		meshes.reserve(numOfMeshes);

		std::unordered_set<std::string> materialNames;

		// for each mesh..
		for (std::size_t i = 0; i < numOfMeshes; ++i) {
			std::vector<Vertex> vertices;
			std::vector<unsigned int> indices;

			// read V, I, C & num of triangles.
			std::size_t numOfVertices;
			std::size_t numOfIndices;
			std::size_t numOfCharactersMaterialName;
			std::size_t numOfCharactersMeshName;
			int numOfTriangles;

			readFromFile(resourceFile, numOfVertices);
			readFromFile(resourceFile, numOfIndices);
			readFromFile(resourceFile, numOfCharactersMaterialName);
			readFromFile(resourceFile, numOfCharactersMeshName);
			readFromFile(resourceFile, numOfTriangles);

			vertices.reserve(numOfVertices);
			indices.reserve(numOfIndices);

			// read all vertices..
			for (std::size_t vertex = 0; vertex < numOfVertices; ++vertex) {
				glm::vec3 pos;
				glm::vec2 textureUnit;
				glm::vec3 normal;
				glm::vec3 tangent;
				glm::vec3 bitangent;

				resourceFile.read(reinterpret_cast<char*>(glm::value_ptr(pos)), sizeof(pos));
				resourceFile.read(reinterpret_cast<char*>(glm::value_ptr(textureUnit)), sizeof(textureUnit));
				resourceFile.read(reinterpret_cast<char*>(glm::value_ptr(normal)), sizeof(normal));
				resourceFile.read(reinterpret_cast<char*>(glm::value_ptr(tangent)), sizeof(tangent));
				resourceFile.read(reinterpret_cast<char*>(glm::value_ptr(bitangent)), sizeof(bitangent));

				vertices.push_back({
					pos,
					textureUnit,
					normal,
					tangent,
					bitangent
				});
			}

			// read null byte delimiter.
			if (!readNextByteIfNull(resourceFile)) {
				return std::nullopt;
			}

			// read all indices..
			for (std::size_t index = 0; index < numOfIndices; ++index) {
				unsigned int indice;
				readFromFile(resourceFile, indice);
				indices.push_back(indice);
			}

			// read null byte delimiter.
			if (!readNextByteIfNull(resourceFile)) {
				return std::nullopt;
			}

			// read material name.
			std::string materialName;
			materialName.resize(numOfCharactersMaterialName);

			resourceFile.read(materialName.data(), numOfCharactersMaterialName);

			// read null byte delimiter.
			if (!readNextByteIfNull(resourceFile)) {
				return std::nullopt;
			}

			// read mesh name.
			std::string meshName;
			meshName.resize(numOfCharactersMeshName);

			resourceFile.read(meshName.data(), numOfCharactersMeshName);

			// read null byte delimiter.
			if (!readNextByteIfNull(resourceFile)) {
				return std::nullopt;
			}

			materialNames.insert(materialName);
			meshes.push_back({ std::move(meshName), std::move(vertices), std::move(indices), std::move(materialName), numOfTriangles});
		}

		Logger::info("Successfully load resource.\n");
		
		// returns a resource constructor
		return { ResourceConstructor{[id, resourceFilePath, meshes = std::move(meshes), materialNames = std::move(materialNames)]() {
			return std::make_unique<Model>(id, std::move(resourceFilePath), std::move(meshes), std::move(materialNames));
		}} };
#endif
		// returns a resource constructor
		return { ResourceConstructor{[id, resourceFilePath, modelData = std::move(modelData)]() {
			return std::make_unique<Model>(id, std::move(resourceFilePath), std::move(modelData));
		}} };
	//}
	//catch (std::exception const& ex) {
	//	Logger::error("Failed to load resource, {}", ex.what());
	//	return std::nullopt;
	//}
}