#pragma once

#include "Recast_DebugUtils/DebugDraw.h"

/// Abstract debug draw interface.
struct NavigationDebugDraw : public duDebugDraw
{
	~NavigationDebugDraw();

	void depthMask(bool state) final;

	void texture(bool state) final;

	/// Begin drawing primitives.
	///  @param prim [in] primitive type to draw, one of rcDebugDrawPrimitives.
	///  @param size [in] size of a primitive, applies to point size and line width only.
	void begin(duDebugDrawPrimitives prim, float size = 1.0f) final;

	/// Submit a vertex
	///  @param pos [in] position of the verts.
	///  @param color [in] color of the verts.
	void vertex(const float* pos, unsigned int color) final;

	/// Submit a vertex
	///  @param x,y,z [in] position of the verts.
	///  @param color [in] color of the verts.
	void vertex(const float x, const float y, const float z, unsigned int color) final;

	/// Submit a vertex
	///  @param pos [in] position of the verts.
	///  @param color [in] color of the verts.
	///  @param uv [in] the uv coordinates of the verts.
	void vertex(const float* pos, unsigned int color, const float* uv) final;

	/// Submit a vertex
	///  @param x,y,z [in] position of the verts.
	///  @param color [in] color of the verts.
	///  @param u,v [in] the uv coordinates of the verts.
	void vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v) final;

	/// End drawing primitives.
	void end() final;

	/// Compute a color for given area.
	//virtual unsigned int areaToCol(unsigned int area);
};