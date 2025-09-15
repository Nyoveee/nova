#include "navMeshGeneration.h"

#include "Engine/engine.h"
#include "editor.h"
#include "hierarchy.h"
#include "ResourceManager/resourceManager.h"
#include "Component/component.h"
#include "../../Nova-Engine/src/Detour/Detour/DetourNavMesh.h"
#include "../../Nova-Engine/src/Detour/Detour/DetourNavMeshBuilder.h"
#include "../../Nova-Engine/src/Detour/Detour/DetourAlloc.h"
#include <vector>
#include <array>
#include <limits>
#undef min and max

NavMeshGeneration::NavMeshGeneration(Editor& editor) :
	ecs{ editor.engine.ecs },
	editor{ editor },
	resourceManager{ editor.resourceManager },

	//Recast Objects
	m_triareas	{ nullptr },
	navData		{ nullptr },
	m_solid		{ nullptr },
	m_chf		{ nullptr },
	m_cset		{ nullptr },
	m_pmesh		{ nullptr },
	m_cfg		{},
	m_ctx		{},
	m_dmesh		{ nullptr }
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

NavMeshGeneration::~NavMeshGeneration()
{
	CleanUp();
}

BuildSettings& NavMeshGeneration::GetBuildSettings()
{
	return buildSettings;
}


void NavMeshGeneration::BuildNavMesh()
{

	CleanUp();

	//preprocessing parameters
	std::vector<float> vertexSoup; // an collection of vertexes 1 2 3
	std::vector<int>   triangleSoup; // a collection of indices that make sup the triangles of the scene
	size_t vertexOffset = 0; //for appending indices of different meshes with new ids
	size_t triangleCount = 0;
	size_t vertexCount = 0;
	std::array<float, 3> boundaryMin{std::numeric_limits<float>::max(), std::numeric_limits<float>::max() , std::numeric_limits<float>::max() }; // (x,y,z)
	std::array<float, 3> boundaryMax{std::numeric_limits<float>::min() , std::numeric_limits<float>::min() , std::numeric_limits<float>::min()}; // (x,y,z)

	//preprocessing step -------
 
	//gather all meshes <Exclude mesh using navmesh modifier component next time>
	for (auto&& [entity, transform, meshRenderer] : ecs.registry.view<Transform, MeshRenderer>().each()) {
			
		auto [model, _] = resourceManager.getResource<Model>(meshRenderer.modelId);
		
		for (const Model::Mesh& meshData : model->meshes)
		{
			for (const Vertex& vertex : meshData.vertices)
			{
				vertexSoup.push_back(vertex.pos.x);
				vertexSoup.push_back(vertex.pos.y);
				vertexSoup.push_back(vertex.pos.z);

				//Bounds checking to find the most extreme vertexes 
				if (vertex.pos.x < boundaryMin[0]) { boundaryMin[0] = vertex.pos.x; }
				if (vertex.pos.y < boundaryMin[1]) { boundaryMin[1] = vertex.pos.y; }
				if (vertex.pos.z < boundaryMin[2]) { boundaryMin[2] = vertex.pos.z; }

				if (vertex.pos.x > boundaryMax[0]) { boundaryMax[0] = vertex.pos.x; }
				if (vertex.pos.y > boundaryMax[1]) { boundaryMax[1] = vertex.pos.y; }
				if (vertex.pos.z > boundaryMax[2]) { boundaryMax[2] = vertex.pos.z; }

			}

			//map triangle soup with corresponding indices in each mesh
			for (size_t i = 0; i < meshData.indices.size(); i += 3)
			{
				triangleSoup.push_back(meshData.indices[i + 0] + vertexOffset);
				triangleSoup.push_back(meshData.indices[i + 1] + vertexOffset);
				triangleSoup.push_back(meshData.indices[i + 2] + vertexOffset);
			}

			vertexOffset += meshData.vertices.size();
		}
	
	}

	vertexCount =	static_cast<int>(vertexSoup.size() / 3);
	triangleCount = static_cast<int>(triangleSoup.size() / 3);

	//if empty scene
	if (vertexCount < 3 || triangleCount == 0)
	{
		return;
	}

	//Initialise configurations -> Step 1
	memset(&m_cfg, 0, sizeof(m_cfg));
	m_cfg.cs = buildSettings.cellSize;
	m_cfg.ch = buildSettings.cellHeight;
	m_cfg.walkableSlopeAngle = buildSettings.agentMaxSlope;
	m_cfg.walkableHeight = (int)ceilf(buildSettings.agentHeight / m_cfg.ch);
	m_cfg.walkableClimb = (int)floorf(buildSettings.agentMaxClimb / m_cfg.ch);
	m_cfg.walkableRadius = (int)ceilf(buildSettings.agentRadius / m_cfg.cs);
	m_cfg.maxEdgeLen = (int)(buildSettings.edgeMaxLen / buildSettings.cellSize);
	m_cfg.maxSimplificationError = buildSettings.edgeMaxError;
	m_cfg.minRegionArea = (int)rcSqr(buildSettings.regionMinSize);		// Note: area = size*size
	m_cfg.mergeRegionArea = (int)rcSqr(buildSettings.regionMergeSize);	// Note: area = size*size
	m_cfg.maxVertsPerPoly = (int)buildSettings.vertsPerPoly;
	m_cfg.detailSampleDist = buildSettings.detailSampleDist < 0.9f ? 0 : buildSettings.cellSize * buildSettings.detailSampleDist;
	m_cfg.detailSampleMaxError = buildSettings.cellHeight * buildSettings.detailSampleMaxError;

	//bounding area
	rcVcopy(m_cfg.bmin, boundaryMin.data());
	rcVcopy(m_cfg.bmax, boundaryMax.data());
	rcCalcGridSize(m_cfg.bmin, m_cfg.bmax, m_cfg.cs, &m_cfg.width, &m_cfg.height);

	// Step 2. Rasterize input polygon soup.
	// Allocate voxel heightfield where we rasterize our input data to.
	m_solid = rcAllocHeightfield();

	if (!rcCreateHeightfield(&m_ctx, *m_solid, m_cfg.width, m_cfg.height, m_cfg.bmin, m_cfg.bmax, m_cfg.cs, m_cfg.ch))
	{
		//m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not create solid heightfield.");
		//return false;
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
	if (!rcRasterizeTriangles(&m_ctx, vertexSoup.data(), vertexCount, triangleSoup.data(), m_triareas, triangleCount, *m_solid, m_cfg.walkableClimb))
	{
		//m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not rasterize triangles.");
		//return false;
	}

	//clear triareas
	delete[] m_triareas;
	m_triareas = nullptr;

	//
	// Step 3. Filter walkable surfaces.
	rcFilterLowHangingWalkableObstacles(&m_ctx, m_cfg.walkableClimb, *m_solid);
	rcFilterLedgeSpans(&m_ctx, m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid);
	rcFilterWalkableLowHeightSpans(&m_ctx, m_cfg.walkableHeight, *m_solid);


	//
	// Step 4. Partition walkable surface to simple regions.
	m_chf = rcAllocCompactHeightfield();

	if (!rcBuildCompactHeightfield(&m_ctx, m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid, *m_chf))
	{
		//m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build compact data.");
		//return false;
	}

	rcFreeHeightField(m_solid);
	m_solid = nullptr;

	// Erode the walkable area by agent radius.
	if (!rcErodeWalkableArea(&m_ctx, m_cfg.walkableRadius, *m_chf))
	{
		//m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not erode.");
		//return false;
	}

	//USE WATERSHED PARTITION
	// Prepare for region partitioning, by calculating distance field along the walkable surface.
	if (!rcBuildDistanceField(&m_ctx, *m_chf))
	{
		//m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build distance field.");
		//return false;
	}

	// Partition the walkable surface into simple regions without holes.
	if (!rcBuildRegions(&m_ctx, *m_chf, 0, m_cfg.minRegionArea, m_cfg.mergeRegionArea))
	{
		//m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build watershed regions.");
		//return false;
	}

	//
	// Step 5. Trace and simplify region contours.
	m_cset = rcAllocContourSet();
	if (!rcBuildContours(&m_ctx, *m_chf, m_cfg.maxSimplificationError, m_cfg.maxEdgeLen, *m_cset))
	{
		//m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not create contours.");
		//return false;
	}


	//
	// Step 6. Build polygons mesh from contours.
	m_pmesh = rcAllocPolyMesh();
	if (!rcBuildPolyMesh(&m_ctx, *m_cset, m_cfg.maxVertsPerPoly, *m_pmesh))
	{
		//m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not triangulate contours.");
		//return false;
	}

	//
	// Step 7. Create detail mesh which allows to access approximate height on each polygon.
	m_dmesh = rcAllocPolyMeshDetail();
	if (!rcBuildPolyMeshDetail(&m_ctx, *m_pmesh, *m_chf, m_cfg.detailSampleDist, m_cfg.detailSampleMaxError, *m_dmesh))
	{
		//m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build detail mesh.");
		//return false;
	}

	//Free Data
	rcFreeCompactHeightfield(m_chf);
	m_chf = nullptr;
	rcFreeContourSet(m_cset);
	m_cset = nullptr;

	//
	// Step 8. Create Detour data from Recast poly mesh.
	if (m_cfg.maxVertsPerPoly <= DT_VERTS_PER_POLYGON)
	{
		//unsigned char* navData = 0;
		int navDataSize = 0;

		// Update poly flags from areas.
		//for (int i = 0; i < m_pmesh->npolys; ++i)
		//{
		//	if (m_pmesh->areas[i] == RC_WALKABLE_AREA)
		//		m_pmesh->areas[i] = SAMPLE_POLYAREA_GROUND;

		//	if (m_pmesh->areas[i] == SAMPLE_POLYAREA_GROUND ||
		//		m_pmesh->areas[i] == SAMPLE_POLYAREA_GRASS ||
		//		m_pmesh->areas[i] == SAMPLE_POLYAREA_ROAD)
		//	{
		//		m_pmesh->flags[i] = SAMPLE_POLYFLAGS_WALK;
		//	}
		//	else if (m_pmesh->areas[i] == SAMPLE_POLYAREA_WATER)
		//	{
		//		m_pmesh->flags[i] = SAMPLE_POLYFLAGS_SWIM;
		//	}
		//	else if (m_pmesh->areas[i] == SAMPLE_POLYAREA_DOOR)
		//	{
		//		m_pmesh->flags[i] = SAMPLE_POLYFLAGS_WALK | SAMPLE_POLYFLAGS_DOOR;
		//	}
		//}


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

		if (!dtCreateNavMeshData(&params, &navData, &navDataSize))
		{
			//m_ctx->log(RC_LOG_ERROR, "Could not build Detour navmesh.");
			//return false;
		}

		m_navMesh = dtAllocNavMesh();
		if (!m_navMesh)
		{
			dtFree(navData);
			//m_ctx->log(RC_LOG_ERROR, "Could not create Detour navmesh");
			//return false;
		}

		dtStatus status;

		status = m_navMesh->init(navData, navDataSize, DT_TILE_FREE_DATA); //here navmesh is built
		
		if (dtStatusFailed(status))
		{
			dtFree(navData);
		//	m_ctx->log(RC_LOG_ERROR, "Could not init Detour navmesh");
		//	return false;
		}

		//status = m_navQuery->init(m_navMesh, 2048);
		//if (dtStatusFailed(status))
		//{
		//	//m_ctx->log(RC_LOG_ERROR, "Could not init Detour navmesh query");
		//	//return false;
		//}
	}
}

void NavMeshGeneration::CleanUp()
{
	delete[] m_triareas;
	m_triareas = 0;
	rcFreeHeightField(m_solid);
	m_solid = 0;
	rcFreeCompactHeightfield(m_chf);
	m_chf = 0;
	rcFreeContourSet(m_cset);
	m_cset = 0;
	rcFreePolyMesh(m_pmesh);
	m_pmesh = 0;
	rcFreePolyMeshDetail(m_dmesh);
	m_dmesh = 0;
	dtFreeNavMesh(m_navMesh);
	m_navMesh = 0;
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
