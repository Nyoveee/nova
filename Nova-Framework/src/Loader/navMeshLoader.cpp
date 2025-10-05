#include "loader.h"
#include "Logger.h"
#include <string>

#include "Detour/Detour/DetourAlloc.h"

std::optional<ResourceConstructor> ResourceLoader<NavMesh>::load(ResourceID id, ResourceFilePath const& resourceFilePath) {
	std::ifstream resourceFile{ resourceFilePath, std::ios::binary };
	
	if (!resourceFile) {
		Logger::error("Failed to open nav mesh file.");
		return std::nullopt;
	}

	//Get file size
	auto totalFilesize= std::filesystem::file_size(std::filesystem::path{ resourceFilePath });


	//See NavMeshGeneration::BuildNavMesh for more info
	//I have currently preappended some data for binary serialisation into the navmesh resource. Need to extract that before navmesh is constructed
	// as of now  [u_int32 size of string][string data][float agent_radius][float agent_height][navmesh data]
	// required info as of now: 
	// std::string agent name, float agentradius, float agentheight

	//-------------------- Preprocessing -----------
	std::string agentName;
	uint32_t strLen = 0;
	float agentRadius =0;
	float agentHeight = 0;

	if (!resourceFile.read(reinterpret_cast<char*>(&strLen), sizeof(strLen))) {
		Logger::error("Failed to read string length from nav mesh file.");
		return std::nullopt;
	}

	//Read String
	if (strLen > 0) {
		agentName.resize(strLen);
		if (!resourceFile.read(agentName.data(), static_cast<std::streamsize>(strLen))) {
			Logger::error("Failed to read agent name from nav mesh file.");
			return std::nullopt;
		}
	}

	// Read floats
	if (!resourceFile.read(reinterpret_cast<char*>(&agentRadius), sizeof(agentRadius))) {
		Logger::error("Failed to read agent radius from nav mesh file.");
		return std::nullopt;
	}
	if (!resourceFile.read(reinterpret_cast<char*>(&agentHeight), sizeof(agentHeight))) {
		Logger::error("Failed to read agent height from nav mesh file.");
		return std::nullopt;
	}

	//Alignment Here
	// Align to 4 bytes 
	size_t offset = sizeof(strLen) + static_cast<size_t>(strLen) + sizeof(float) * 2;
	offset = (offset + static_cast<uint32_t>(3)) & ~ static_cast<uint32_t>(3);
	if (offset >= totalFilesize) {
		Logger::error("Corrupt navmesh file please delete or regenerate the file");
		return std::nullopt;
	}

	//------- Navmesh Stuff ------------------

	//Get actual navmeshfile size
	auto navDataSize = totalFilesize - offset;
	
	// yo this is a warcrime but what do..
	unsigned char* navData = (unsigned char*)dtAlloc(sizeof(unsigned char) * navDataSize, DT_ALLOC_PERM);

	//Seek to navmesh start
	resourceFile.seekg(static_cast<std::streamoff>(offset), std::ios::beg);
	resourceFile.read(reinterpret_cast<char*>(navData), navDataSize);

	dtNavMesh* navMesh = dtAllocNavMesh();

	if (!navMesh) {
		Logger::error("Failed to create nav mesh asset.");
		return std::nullopt;
	}

	dtStatus status = navMesh->init(navData, static_cast<int const>(navDataSize), 0);;

	if (dtStatusFailed(status)) {
		Logger::error("Could not init Detour navmesh query.");
		return std::nullopt;
	}

	return { {[id, resourceFilePath, agentName, agentRadius, agentHeight , navData, navMesh] {
		return std::make_unique<NavMesh>(id, std::move(resourceFilePath), std::move(agentName), agentRadius, agentHeight , navData, navMesh);
	}} };
}