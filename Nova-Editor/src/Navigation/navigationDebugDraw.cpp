#include "navigationDebugDraw.h"
#include "Graphics/renderer.h"

NavigationDebugDraw::~NavigationDebugDraw() {}

void NavigationDebugDraw::depthMask(bool state) {
	//renderer.setBlendMode(state ? Renderer::BlendingConfig::AlphaBlending : Renderer::BlendingConfig::Disabled);
}

void NavigationDebugDraw::texture(bool state) {

}

/// Begin drawing primitives.
///  @param prim [in] primitive type to draw, one of rcDebugDrawPrimitives.
///  @param size [in] size of a primitive, applies to point size and line width only.
void NavigationDebugDraw::begin(duDebugDrawPrimitives prim, float size) {

}

/// Submit a vertex
///  @param pos [in] position of the verts.
///  @param color [in] color of the verts.
void NavigationDebugDraw::vertex(const float* pos, unsigned int color) {

}

/// Submit a vertex
///  @param x,y,z [in] position of the verts.
///  @param color [in] color of the verts.
void NavigationDebugDraw::vertex(const float x, const float y, const float z, unsigned int color) {

}

/// Submit a vertex
///  @param pos [in] position of the verts.
///  @param color [in] color of the verts.
///  @param uv [in] the uv coordinates of the verts.
void NavigationDebugDraw::vertex(const float* pos, unsigned int color, const float* uv) {

}

/// Submit a vertex
///  @param x,y,z [in] position of the verts.
///  @param color [in] color of the verts.
///  @param u,v [in] the uv coordinates of the verts.
void NavigationDebugDraw::vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v) {

}

/// End drawing primitives.
void NavigationDebugDraw::end() {

}