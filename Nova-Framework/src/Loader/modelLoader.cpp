#include <fstream>
#include <glm/gtc/type_ptr.hpp>

#include "loader.h"
#include "Logger.h"

#include "model.h"

namespace {
	template <typename T>
	void readFromFile(std::ifstream& inputFile, T& data) {
		inputFile.read(reinterpret_cast<char*>(&data), sizeof(data));
	}

	bool readNextByteIfNull(std::ifstream& inputFile) {
		char c;
		inputFile.read(&c, 1);

		return c == 0;
	}
}

std::optional<ResourceConstructor> ResourceLoader<Model>::load(ResourceID id, ResourceFilePath const& resourceFilePath) {
	Logger::info("Loading model resource file {}", resourceFilePath.string);

	std::ifstream resourceFile{ resourceFilePath.string, std::ios::binary };

	if (!resourceFile) {
		Logger::error("Failed to parse model.");
		return std::nullopt;
	}

	resourceFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	
	try {
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
		
		// read M.
		std::size_t numOfMeshes;
		readFromFile(resourceFile, numOfMeshes);

		std::vector<Model::Mesh> meshes;
		meshes.reserve(numOfMeshes);

		std::unordered_set<std::string> materialNames;

		// for each mesh..
		for (std::size_t i = 0; i < numOfMeshes; ++i) {
			std::vector<Vertex> vertices;
			std::vector<unsigned int> indices;

			// read V, I, C & num of triangles.
			std::size_t numOfVertices;
			std::size_t numOfIndices;
			std::size_t numOfCharacters;
			int numOfTriangles;

			readFromFile(resourceFile, numOfVertices);
			readFromFile(resourceFile, numOfIndices);
			readFromFile(resourceFile, numOfCharacters);
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
			materialName.resize(numOfCharacters);

			resourceFile.read(materialName.data(), numOfCharacters);

			// read null byte delimiter.
			if (!readNextByteIfNull(resourceFile)) {
				return std::nullopt;
			}

			materialNames.insert(materialName);
			meshes.push_back({ std::move(vertices), std::move(indices), std::move(materialName), numOfTriangles });
		}

		Logger::info("Successfully load resource.\n");
		
		// returns a resource constructor
		return { ResourceConstructor{[id, resourceFilePath, meshes = std::move(meshes), materialNames = std::move(materialNames)]() {
			return std::make_unique<Model>(id, std::move(resourceFilePath), std::move(meshes), std::move(materialNames));
		}} };
	}
	catch (std::exception const& ex) {
		Logger::error("Failed to load resource, {}", ex.what());
		return std::nullopt;
	}
}