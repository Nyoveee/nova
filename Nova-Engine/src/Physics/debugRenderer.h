#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Renderer/DebugRendererSimple.h>

class Renderer;

// our implementation of debug renderer to ... debug rendering :rofl:
// we need to provide our own implementation because obviously the physics library can't provide it's own renderer
// we simply override the virtual member functions required.

/// Implement the following virtual functions:
/// - DrawLine
/// - DrawTriangle
/// - DrawText3D
/// - CreateTriangleBatch
/// - DrawGeometry
/// 
/// Make sure you call Initialize() from the constructor of your implementation.
/// 
/// The CreateTriangleBatch is used to prepare a batch of triangles to be drawn by a single DrawGeometry call,
/// which means that Jolt can render a complex scene much more efficiently than when each triangle in that scene would have been drawn through DrawTriangle.
///
/// Note that an implementation that implements CreateTriangleBatch and DrawGeometry is provided by DebugRendererSimple which can be used to start quickly.

class DebugRenderer final : public JPH::DebugRendererSimple {
public:
	DebugRenderer(Renderer& renderer);

	~DebugRenderer();
	DebugRenderer(DebugRenderer const& other)				= delete;
	DebugRenderer(DebugRenderer&& other)					= delete;
	DebugRenderer& operator=(DebugRenderer const& other)	= delete;
	DebugRenderer& operator=(DebugRenderer&& other)			= delete;

public:
	// interfaces we need to implement.
	void	DrawLine			(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) final;
	void	DrawTriangle		(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, ECastShadow inCastShadow = ECastShadow::Off) final;
	void	DrawText3D			(JPH::RVec3Arg inPosition, JPH::string_view const& inString, JPH::ColorArg inColor = JPH::Color::sWhite, float inHeight = 0.5f) final;
	
	// this is implemented by sample debug renderer.
	// Batch	CreateTriangleBatch	(const Vertex* inVertices, int inVertexCount, const JPH::uint32* inIndices, int inIndexCount) final;
	// void		DrawGeometry		(JPH::RMat44Arg inModelMatrix, const JPH::AABox& inWorldSpaceBounds, float inLODScaleSq, JPH::ColorArg inModelColor, const GeometryRef& inGeometry, ECullMode inCullMode = ECullMode::CullBackFace, ECastShadow inCastShadow = ECastShadow::On, EDrawMode inDrawMode = EDrawMode::Solid) final;

private:
	Renderer& renderer;
};