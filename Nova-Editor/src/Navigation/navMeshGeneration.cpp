#include "navMeshGeneration.h"

#include "Engine/engine.h"
#include "Editor/editor.h"
#include "Editor/hierarchy.h"
#include "ResourceManager/resourceManager.h"
#include "ECS/component.h"
#include "Detour/Detour/DetourNavMesh.h"
#include "Detour/Detour/DetourNavMeshBuilder.h"
#include "Detour/Detour/DetourAlloc.h"
#include "Navigation/Navigation.h"
#include <vector>
#include <array>
#include <limits>

#undef min 
#undef max

NavMeshGeneration::NavMeshGeneration(Editor& editor) :
	ecs				{ editor.engine.ecs },
	editor			{ editor },
	resourceManager	{ editor.resourceManager }
{
	buildSettings.agentName				= "Humanoid";
	buildSettings.cellSize				= 0.3f;
	buildSettings.cellHeight			= 0.2f;
	buildSettings.agentHeight			= 2.0f;
	buildSettings.agentRadius			= 0.6f;
	buildSettings.agentMaxClimb			= 0.9f;
	buildSettings.agentMaxSlope			= 45.0f;
	buildSettings.regionMinSize			= 8;
	buildSettings.regionMergeSize		= 20;
	buildSettings.edgeMaxLen			= 12.0f;
	buildSettings.edgeMaxError			= 1.3f;
	buildSettings.vertsPerPoly			= 6.0f;
	buildSettings.detailSampleDist		= 6.0f;
	buildSettings.detailSampleMaxError	= 1.0f;
	buildSettings.partitionType			= 0;
}

NavMeshGeneration::~NavMeshGeneration()
{}

BuildSettings& NavMeshGeneration::GetBuildSettings()
{
	return buildSettings;
}

void NavMeshGeneration::BuildNavMesh(std::string const& filename) {
	// =====================================================
	// Variable initialisation..
	// =====================================================

	unsigned char* m_triareas	= nullptr; 
	unsigned char* navData		= nullptr;
	rcHeightfield* m_solid		= nullptr;
	rcCompactHeightfield* m_chf = nullptr;
	rcContourSet* m_cset		= nullptr;
	rcPolyMesh* m_pmesh			= nullptr;
	rcPolyMeshDetail* m_dmesh	= nullptr;
	dtNavMeshQuery* m_navQuery	= nullptr;

	rcConfig m_cfg;
	rcContext m_ctx;

	std::vector<float>		vertexSoup;						// Collection of all the vertices
	std::vector<int>		triangleSoup;					// Collection of all the indices
	std::vector<int>		unwalkableTriangles;			// Marked certain indices to be unwalkable.

	std::size_t				triIndex				= 0;
	std::size_t				vertexOffset			= 0;	// For appending indices of different meshes with new ids

	std::array<float, 3>	boundaryMin				{ std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() }; // (x,y,z)
	std::array<float, 3>	boundaryMax				{ std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min() }; // (x,y,z)

 
	// =====================================================
	// 1. We first gather all the vertices and indices of our current scene
	// and deal with nav mesh modifiers..
	// =====================================================

	// Gather all meshes
	for (auto&& [entity, transform, meshRenderer] : ecs.registry.view<Transform, MeshRenderer>().each()) {
		auto [model, _] = resourceManager.getResource<Model>(meshRenderer.modelId);
		
		if (!model) {
			continue;
		}

		NavMeshModifier* navMeshModifier = nullptr;

		//Extra settings base on NavmeshModifier
		if (navMeshModifier = ecs.registry.try_get<NavMeshModifier>(entity))
		{
			if (navMeshModifier->Area_Type == NavMeshModifier::Area_Type::Exclude)
			{
				continue;
			}
		
		}

		for (const Model::Mesh& meshData : model->meshes)
		{

			for (const Vertex& vertex : meshData.vertices)
			{
				glm::vec4 worldPos = transform.modelMatrix * glm::vec4(vertex.pos, 1.0f);
				vertexSoup.push_back(worldPos.x);
				vertexSoup.push_back(worldPos.y);
				vertexSoup.push_back(worldPos.z);

				// Bounds checking to find the most extreme vertexes 
				if (worldPos.x < boundaryMin[0]) { boundaryMin[0] = worldPos.x; }
				if (worldPos.y < boundaryMin[1]) { boundaryMin[1] = worldPos.y; }
				if (worldPos.z < boundaryMin[2]) { boundaryMin[2] = worldPos.z; }

				if (worldPos.x > boundaryMax[0]) { boundaryMax[0] = worldPos.x; }
				if (worldPos.y > boundaryMax[1]) { boundaryMax[1] = worldPos.y; }
				if (worldPos.z > boundaryMax[2]) { boundaryMax[2] = worldPos.z; }
			}

			// map triangle soup with corresponding indices in each mesh
			for (size_t i = 0; i < meshData.indices.size(); i += 3)
			{
				triangleSoup.push_back(meshData.indices[i + 0] + vertexOffset);
				triangleSoup.push_back(meshData.indices[i + 1] + vertexOffset);
				triangleSoup.push_back(meshData.indices[i + 2] + vertexOffset);

				// Mark obstacles
				if (navMeshModifier && navMeshModifier->Area_Type == NavMeshModifier::Area_Type::Obstacle) {
					unwalkableTriangles.push_back(triIndex);
				}
				triIndex++;
			}
			
			vertexOffset += meshData.vertices.size();
		}
	
	}

	std::size_t	vertexCount		= static_cast<int>(vertexSoup.size() / 3);
	std::size_t	triangleCount   = static_cast<int>(triangleSoup.size() / 3);

	//if empty scene
	if (vertexCount < 3 || triangleCount == 0)
	{
		return;
	}

	// ======================================
	// 2. We initialise configurations based on our build settings..
	// ======================================

	memset(&m_cfg, 0, sizeof(m_cfg));

	m_cfg.cs						= buildSettings.cellSize;
	m_cfg.ch						= buildSettings.cellHeight;
	m_cfg.walkableSlopeAngle		= buildSettings.agentMaxSlope;
	m_cfg.walkableHeight			= (int)ceilf(buildSettings.agentHeight / m_cfg.ch);
	m_cfg.walkableClimb				= (int)floorf(buildSettings.agentMaxClimb / m_cfg.ch);
	m_cfg.walkableRadius			= (int)ceilf(buildSettings.agentRadius / m_cfg.cs);
	m_cfg.maxEdgeLen				= (int)(buildSettings.edgeMaxLen / buildSettings.cellSize);
	m_cfg.maxSimplificationError	= buildSettings.edgeMaxError;
	m_cfg.minRegionArea				= (int)rcSqr(buildSettings.regionMinSize);		// Note: area = size*size
	m_cfg.mergeRegionArea			= (int)rcSqr(buildSettings.regionMergeSize);	// Note: area = size*size
	m_cfg.maxVertsPerPoly			= (int)buildSettings.vertsPerPoly;
	m_cfg.detailSampleDist			= buildSettings.detailSampleDist < 0.9f ? 0 : buildSettings.cellSize * buildSettings.detailSampleDist;
	m_cfg.detailSampleMaxError		= buildSettings.cellHeight * buildSettings.detailSampleMaxError;

	// ======================================
	// 3. We set bounding area
	// ======================================
	
	rcVcopy(m_cfg.bmin, boundaryMin.data());
	rcVcopy(m_cfg.bmax, boundaryMax.data());
	rcCalcGridSize(m_cfg.bmin, m_cfg.bmax, m_cfg.cs, &m_cfg.width, &m_cfg.height);

	// Rasterize input polygon soup.
	// Allocate voxel heightfield where we rasterize our input data to.
	m_solid = rcAllocHeightfield();

	if (!rcCreateHeightfield(&m_ctx, *m_solid, m_cfg.width, m_cfg.height, m_cfg.bmin, m_cfg.bmax, m_cfg.cs, m_cfg.ch)) {
		Logger::error("Building navigation: Could not create solid heightfield.");
		return;
	}

	// Allocate array that can hold triangle area types.
	// If you have multiple meshes you need to process, allocate
	// and array which can hold the max number of triangles you need to process.
	m_triareas = new unsigned char[triangleCount];

	// Find triangles which are walkable based on their slope and rasterize them.
	// If your input data is multiple meshes, you can transform them here, calculate
	// the are type for each of the meshes and rasterize them.
	memset(m_triareas, 0, triangleCount * sizeof(unsigned char));
	rcMarkWalkableTriangles(&m_ctx, m_cfg.walkableSlopeAngle, vertexSoup.data(), vertexCount, triangleSoup.data(), triangleCount, m_triareas); //use walkable triangles first. maybe can check how to exclude

	// ======================================
	// 4. We specific which indices are unwalkable..
	// ======================================

	for (int& triIndex : unwalkableTriangles) {
		m_triareas[triIndex] = RC_NULL_AREA;
	}

	//---

	// ======================================
	// 5. We rasterize these vertices and indices to workable triangle areas..
	// ======================================

	if (!rcRasterizeTriangles(&m_ctx, vertexSoup.data(), vertexCount, triangleSoup.data(), m_triareas, triangleCount, *m_solid, m_cfg.walkableClimb))
	{
		Logger::error("Building navigation: Could not rasterize triangles.");
		return;
	}

	//clear triareas
	delete[] m_triareas;

	// ======================================
	// 6. We filter walkable surfaces..
	// ======================================
	rcFilterLowHangingWalkableObstacles(&m_ctx, m_cfg.walkableClimb, *m_solid);
	rcFilterLedgeSpans(&m_ctx, m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid);
	rcFilterWalkableLowHeightSpans(&m_ctx, m_cfg.walkableHeight, *m_solid);

	// ======================================
	// 7. We build compact height field data
	// ======================================
	m_chf = rcAllocCompactHeightfield();

	if (!rcBuildCompactHeightfield(&m_ctx, m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid, *m_chf))
	{
		Logger::error("Building navigation: Could not build compact data.");
		return;
	}

	rcFreeHeightField(m_solid);

	// ======================================
	// 8. We erode the walkable area by agent radius.
	// ======================================

	// Erode the walkable area by agent radius.
	if (!rcErodeWalkableArea(&m_ctx, m_cfg.walkableRadius, *m_chf))
	{
		Logger::error("Building navigation: Could not erode.");
		return;
	}

	// ======================================
	// 9. We build our distance field..
	// ======================================

	//USE WATERSHED PARTITION
	// Prepare for region partitioning, by calculating distance field along the walkable surface.
	if (!rcBuildDistanceField(&m_ctx, *m_chf))
	{
		Logger::error("Building navigation: Could not build distance field.");
		return;
	}

	// ======================================
	// 10. We partition our walkable surfaces..
	// ======================================
	
	// Partition the walkable surface into simple regions without holes.
	if (!rcBuildRegions(&m_ctx, *m_chf, 0, m_cfg.minRegionArea, m_cfg.mergeRegionArea))
	{
		Logger::error("Building navigation: Could not build watershed regions.");
		return;
	}

	// ======================================
	// 11. We trace and simplify region contours..
	// ======================================
	
	m_cset = rcAllocContourSet();
	if (!rcBuildContours(&m_ctx, *m_chf, m_cfg.maxSimplificationError, m_cfg.maxEdgeLen, *m_cset))
	{
		Logger::error("Building navigation: Could not create contours.");
		return;
	}

	// ======================================
	// 12. We build polygon mesh from contours..
	// ======================================
	
	m_pmesh = rcAllocPolyMesh();
	if (!rcBuildPolyMesh(&m_ctx, *m_cset, m_cfg.maxVertsPerPoly, *m_pmesh))
	{
		Logger::error("Building navigation: Could not triangulate contours.");
		return;
	}

	// ======================================
	// 12. We create detail mesh which allows to access approximate height on each polygon.
	// ======================================
	
	m_dmesh = rcAllocPolyMeshDetail();
	if (!rcBuildPolyMeshDetail(&m_ctx, *m_pmesh, *m_chf, m_cfg.detailSampleDist, m_cfg.detailSampleMaxError, *m_dmesh))
	{
		Logger::error("Building navigation: Could not build detail mesh.");
		return;
	}

	//Free Data
	rcFreeCompactHeightfield(m_chf);
	rcFreeContourSet(m_cset);

	//NOTE if m_dmesh fails to build dtCreateNavMeshData fails to initilise and it will crash , so return here
	if (m_dmesh->tris == nullptr)
	{
		return; 
	}

	// ======================================
	// 13. Create Detour data from Recast poly mesh.
	// ======================================
	if (m_cfg.maxVertsPerPoly > DT_VERTS_PER_POLYGON)
	{
		return;
	}

	int navDataSize = 0;

	// Update poly flags from areas.
	for (int i = 0; i < m_pmesh->npolys; ++i)
	{
		if (m_pmesh->areas[i] == RC_WALKABLE_AREA)
			m_pmesh->areas[i] = SAMPLE_POLYAREA_GROUND;

		if (m_pmesh->areas[i] == SAMPLE_POLYAREA_GROUND ||
			m_pmesh->areas[i] == SAMPLE_POLYAREA_GRASS ||
			m_pmesh->areas[i] == SAMPLE_POLYAREA_ROAD)
		{
			m_pmesh->flags[i] = SAMPLE_POLYFLAGS_WALK;
		}
		else if (m_pmesh->areas[i] == SAMPLE_POLYAREA_WATER)
		{
			m_pmesh->flags[i] = SAMPLE_POLYFLAGS_SWIM;
		}
		else if (m_pmesh->areas[i] == SAMPLE_POLYAREA_DOOR)
		{
			m_pmesh->flags[i] = SAMPLE_POLYFLAGS_WALK | SAMPLE_POLYFLAGS_DOOR;
		}
	}

	dtNavMeshCreateParams params;
	memset(&params, 0, sizeof(params));
	params.verts = m_pmesh->verts;
	params.vertCount = m_pmesh->nverts;
	params.polys = m_pmesh->polys;
	params.polyAreas = m_pmesh->areas;
	params.polyFlags = m_pmesh->flags;
	params.polyCount = m_pmesh->npolys;
	params.nvp = m_pmesh->nvp;
	params.detailMeshes = m_dmesh->meshes;
	params.detailVerts = m_dmesh->verts;
	params.detailVertsCount = m_dmesh->nverts;
	params.detailTris = m_dmesh->tris;
	params.detailTriCount = m_dmesh->ntris;
	params.offMeshConVerts = 0;
	params.offMeshConRad = 0;
	params.offMeshConDir = 0;
	params.offMeshConAreas = 0;
	params.offMeshConFlags = 0;
	params.offMeshConUserID = 0;
	params.offMeshConCount = 0;
	params.walkableHeight = buildSettings.agentHeight;
	params.walkableRadius = buildSettings.agentRadius;
	params.walkableClimb = buildSettings.agentMaxClimb;
	rcVcopy(params.bmin, m_pmesh->bmin);
	rcVcopy(params.bmax, m_pmesh->bmax);
	params.cs = m_cfg.cs;
	params.ch = m_cfg.ch;
	params.buildBvTree = true;

	if (!dtCreateNavMeshData(&params, &navData, &navDataSize)) //Serialisation
	{
		Logger::error("Building navigation: Could not build Detour navmesh.");
		return;
	}

	rcFreePolyMesh(m_pmesh);
	rcFreePolyMeshDetail(m_dmesh);

	std::filesystem::path temporaryMeshFilePath = AssetIO::assetDirectory / "NavMesh" / filename;
	temporaryMeshFilePath.replace_extension(".navmesh");

	std::ofstream navMeshFile{ temporaryMeshFilePath, std::ios::binary };

	if (!navMeshFile) {
		Logger::error("Failed to create nav mesh file.");
		return;
	}

	navMeshFile.write(reinterpret_cast<char *>(navData), navDataSize);
	dtFree(navData);
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
