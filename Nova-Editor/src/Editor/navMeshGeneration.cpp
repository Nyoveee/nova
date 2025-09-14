#include "navMeshGeneration.h"

#include "Engine/engine.h"
#include "editor.h"
#include "hierarchy.h"
#include "Recast/Recast.h"
#include "Component/component.h"

NavMeshGeneration::NavMeshGeneration(Editor& editor): 
	ecs{ editor.engine.ecs },
	editor{ editor }
{
	buildSettings.agentName = std::string{ "Humanoid" };
	buildSettings.cellSize = 0.3f;
	buildSettings.cellHeight = 0.2f;
	buildSettings.agentHeight = 2.0f;
	buildSettings.agentRadius = 0.6f;
	buildSettings.agentMaxClimb = 0.9f;
	buildSettings.agentMaxSlope = 45.0f;
	buildSettings.regionMinSize = 8;
	buildSettings.regionMergeSize = 20;
	buildSettings.edgeMaxLen = 12.0f;
	buildSettings.edgeMaxError = 1.3f;
	buildSettings.vertsPerPoly = 6.0f;
	buildSettings.detailSampleDist = 6.0f;
	buildSettings.detailSampleMaxError = 1.0f;
	buildSettings.partitionType = 0;
}

BuildSettings& NavMeshGeneration::GetBuildSettings()
{
	return buildSettings;
}


void NavMeshGeneration::BuildNavMesh()
{


}

void NavMeshGeneration::ResetBuildSetting()
{
	buildSettings.agentName = std::string{ "Humanoid" };
	buildSettings.cellSize = 0.3f;
	buildSettings.cellHeight = 0.2f;
	buildSettings.agentHeight = 2.0f;
	buildSettings.agentRadius = 0.6f;
	buildSettings.agentMaxClimb = 0.9f;
	buildSettings.agentMaxSlope = 45.0f;
	buildSettings.regionMinSize = 8;
	buildSettings.regionMergeSize = 20;
	buildSettings.edgeMaxLen = 12.0f;
	buildSettings.edgeMaxError = 1.3f;
	buildSettings.vertsPerPoly = 6.0f;
	buildSettings.detailSampleDist = 6.0f;
	buildSettings.detailSampleMaxError = 1.0f;
	buildSettings.partitionType = 0;


}
