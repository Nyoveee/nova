#pragma once

#include <entt/entt.hpp>
#include "Recast/Recast.h"
#include <string>

class ECS;
class Editor;
class ResourceManager;
class dtNavMesh;
class dtNavMeshQuery;

struct BuildSettings
{
	//Agent Name
	std::string agentName;
	// Cell size in world units
	float cellSize;
	// Cell height in world units
	float cellHeight;
	// Agent height in world units
	float agentHeight;
	// Agent radius in world units
	float agentRadius;
	// Agent max climb in world units
	float agentMaxClimb;
	// Agent max slope in degrees
	float agentMaxSlope;
	// Region minimum size in voxels.
	// regionMinSize = sqrt(regionMinArea)
	float regionMinSize;
	// Region merge size in voxels.
	// regionMergeSize = sqrt(regionMergeArea)
	float regionMergeSize;
	// Edge max length in world units
	float edgeMaxLen;
	// Edge max error in voxels
	float edgeMaxError;
	float vertsPerPoly;
	// Detail sample distance in voxels
	float detailSampleDist;
	// Detail sample max error in voxel heights.
	float detailSampleMaxError;
	// Partition type, see SamplePartitionType
	int partitionType;
	// Bounds of the area to mesh
	float navMeshBMin[3];
	float navMeshBMax[3];
	// Size of the tiles in voxels
	float tileSize;
};


class NavMeshGeneration
{

public:
	NavMeshGeneration(Editor& editor);

	~NavMeshGeneration();

	BuildSettings& GetBuildSettings();

	void ResetBuildSetting();

	void BuildNavMesh();

private:

	//helper functions to free objects
	void CleanUp();

	ECS& ecs;
	BuildSettings buildSettings;
	Editor& editor;
	ResourceManager& resourceManager;

	//Recast Objects
	unsigned char* m_triareas; //for marking walkable or unwalkable surfaces
	unsigned char* navData;
	rcHeightfield* m_solid;
	rcCompactHeightfield* m_chf;
	rcContourSet* m_cset;
	rcPolyMesh* m_pmesh;
	rcConfig m_cfg;
	rcPolyMeshDetail* m_dmesh;
	rcContext m_ctx;

	//NavMesh Objects
	class dtNavMesh* m_navMesh;
	class dtNavMeshQuery* m_navQuery;


};



