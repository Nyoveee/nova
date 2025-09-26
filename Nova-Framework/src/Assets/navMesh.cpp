#include "navMesh.h"
#include "Detour/Detour/DetourNavMesh.h"
#include "Logger.h"

NavMesh::NavMesh(ResourceID id, ResourceFilePath resourceFilePath, std::string agentName, float buildRadius, float buildHeight, unsigned char* navData, dtNavMesh* navMesh) :
	Resource	{ id, resourceFilePath },
	agentName	{ agentName },
	buildRadius	{ buildRadius },
	buildHeight	{ buildHeight },
	navData		{ navData },
	navMesh		{ navMesh }
{}

NavMesh::~NavMesh() {
	if (navMesh) dtFreeNavMesh(navMesh);
	if (navData) dtFree(navData);
}

NavMesh::NavMesh(NavMesh&& other) noexcept :
	Resource	{ std::move(other) },
	navData		{ other.navData },
	navMesh		{ other.navMesh }
{
	other.navData = nullptr;
	other.navMesh = nullptr; 
}

NavMesh& NavMesh::operator=(NavMesh&& other) noexcept {
	Resource::operator=(std::move(other));

	if (navMesh) dtFreeNavMesh(navMesh);
	if (navData) dtFree(navData);

	navData = other.navData;
	navMesh = other.navMesh;

	other.navData = nullptr;
	other.navMesh = nullptr;

	return *this;
}
