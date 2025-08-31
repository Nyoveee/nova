#include "debugRenderer.h"
#include "Graphics/renderer.h"

#include <iostream>

namespace {
	glm::vec3 toGlmVec3(JPH::RVec3Arg const& jphVec3) {
		return { jphVec3.GetX(), jphVec3.GetY(), jphVec3.GetZ() };
	}

	ColorA toColorA(JPH::ColorArg const& jphColor) {
		auto vec4 = jphColor.ToVec4();
		return { vec4.GetX(), vec4.GetY(), vec4.GetZ(), vec4.GetW() };
	}
}

DebugRenderer::DebugRenderer(Renderer& renderer) :
	renderer { renderer }
{
	JPH::DebugRenderer::Initialize();
}

DebugRenderer::~DebugRenderer() {}

void DebugRenderer::DrawLine(JPH::RVec3Arg from, JPH::RVec3Arg to, JPH::ColorArg color) {
	(void) from;
	(void) to;
	(void) color;
}

void DebugRenderer::DrawTriangle(JPH::RVec3Arg vertice1, JPH::RVec3Arg vertice2, JPH::RVec3Arg vertice3, JPH::ColorArg color, ECastShadow castShadow) {
	(void) castShadow;

	renderer.submitTriangle(toGlmVec3(vertice1), toGlmVec3(vertice2), toGlmVec3(vertice3), toColorA(color));
}

void DebugRenderer::DrawText3D(JPH::RVec3Arg position, JPH::string_view const& string, JPH::ColorArg color, float height) {
	(void) position;
	(void) string;
	(void) color;
	(void) height;
	
	// our engine has no text support yet.
	return;
}

//DebugRenderer::Batch DebugRenderer::CreateTriangleBatch(const Vertex* inVertices, int inVertexCount, const JPH::uint32* inIndices, int inIndexCount) {
//	return Batch{};
//}
//
//void DebugRenderer::DrawGeometry(JPH::RMat44Arg inModelMatrix, const JPH::AABox& inWorldSpaceBounds, float inLODScaleSq, JPH::ColorArg inModelColor, const GeometryRef& inGeometry, ECullMode inCullMode, ECastShadow inCastShadow, EDrawMode inDrawMode) {
//
//}
