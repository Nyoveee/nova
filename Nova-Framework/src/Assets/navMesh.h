#pragma once

#include "resource.h"
#include "Detour/Detour/DetourNavMesh.h"

class NavMesh : public Resource {
public:
	FRAMEWORK_DLL_API NavMesh(ResourceID id, ResourceFilePath resourceFilePath, unsigned char* navData, dtNavMesh* navMesh);
	FRAMEWORK_DLL_API ~NavMesh();

	FRAMEWORK_DLL_API NavMesh(NavMesh const& other) = delete;
	FRAMEWORK_DLL_API NavMesh(NavMesh&& other) noexcept;
	FRAMEWORK_DLL_API NavMesh& operator=(NavMesh const& other) = delete;
	FRAMEWORK_DLL_API NavMesh& operator=(NavMesh&& other) noexcept;

public:
	unsigned char* navData;
	dtNavMesh* navMesh;
};