#include "loader.h"
#include "Logger.h"

#include "Detour/Detour/DetourAlloc.h"

std::optional<ResourceConstructor> ResourceLoader<NavMesh>::load(ResourceID id, ResourceFilePath const& resourceFilePath) {
	std::ifstream resourceFile{ resourceFilePath, std::ios::binary };
	
	if (!resourceFile) {
		Logger::error("Failed to open nav mesh file.");
		return std::nullopt;
	}

	auto navDataSize = std::filesystem::file_size(std::filesystem::path{ resourceFilePath });
	
	// yo this is a warcrime but what do..
	unsigned char* navData = (unsigned char*)dtAlloc(sizeof(unsigned char) * navDataSize, DT_ALLOC_PERM);
	resourceFile.read(reinterpret_cast<char*>(navData), navDataSize);

	dtNavMesh* navMesh = dtAllocNavMesh();

	if (!navMesh) {
		Logger::error("Failed to create nav mesh asset.");
		return std::nullopt;
	}

	dtStatus status = navMesh->init(navData, navDataSize, 0);;

	if (dtStatusFailed(status)) {
		Logger::error("Could not init Detour navmesh query.");
		return std::nullopt;
	}

	return { {[id, resourceFilePath, navData, navMesh] {
		return std::make_unique<NavMesh>(id, std::move(resourceFilePath), navData, navMesh);
	}} };
}