#pragma once

#include <entt/entt.hpp>
#include "Navigation/Recast/Recast.h"
#include <string>

class ECS;
class Editor;
class ResourceManager;
class dtNavMesh;
class dtNavMeshQuery;

enum SamplePolyAreas
{
	SAMPLE_POLYAREA_GROUND,
	SAMPLE_POLYAREA_WATER,
	SAMPLE_POLYAREA_ROAD,
	SAMPLE_POLYAREA_DOOR,
	SAMPLE_POLYAREA_GRASS,
	SAMPLE_POLYAREA_JUMP
};

enum SamplePolyFlags
{
	SAMPLE_POLYFLAGS_WALK = 0x01,		// Ability to walk (ground, grass, road)
	SAMPLE_POLYFLAGS_SWIM = 0x02,		// Ability to swim (water).
	SAMPLE_POLYFLAGS_DOOR = 0x04,		// Ability to move through doors.
	SAMPLE_POLYFLAGS_JUMP = 0x08,		// Ability to jump.
	SAMPLE_POLYFLAGS_DISABLED = 0x10,		// Disabled polygon
	SAMPLE_POLYFLAGS_ALL = 0xffff	// All abilities.
};

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


class NavMeshGeneration {
public:
	NavMeshGeneration(Editor& editor);
	~NavMeshGeneration();

	BuildSettings& GetBuildSettings();
	void ResetBuildSetting();
	void BuildNavMesh();

	dtNavMesh const* getNavMesh() const;

private:
	//helper functions to free objects
	void CleanUp();

private:
	ECS& ecs;
	Editor& editor;
	ResourceManager& resourceManager;

private:
	BuildSettings buildSettings;

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
	dtNavMesh* m_navMesh;
	dtNavMeshQuery* m_navQuery;
};



