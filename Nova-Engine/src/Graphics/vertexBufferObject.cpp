#include <glad/glad.h>
#include <algorithm>

#include "vertexBufferObject.h"

namespace {
	// A VBO holding INVALID_ID means it's not holding to any dynamically allocated resource.
	constexpr inline GLuint INVALID_ID = std::numeric_limits<GLuint>::max();
}

VertexBufferObject::VertexBufferObject(GLsizeiptr amountOfMemory, Usage usage) : 
	allocatedMemory	{ amountOfMemory },
	m_id			{ INVALID_ID } 
{
	glCreateBuffers(1, &m_id);

	GLenum drawMode = GL_DYNAMIC_DRAW;

	switch (usage)
	{
	case VertexBufferObject::Usage::StaticDraw:
		drawMode = GL_STATIC_DRAW;
		break;
	case VertexBufferObject::Usage::StreamDraw:
		drawMode = GL_STREAM_DRAW;
		break;
	case VertexBufferObject::Usage::DynamicDraw:
	default:
		break;
	}

	glNamedBufferData(m_id, amountOfMemory, nullptr, drawMode);
}

VertexBufferObject::~VertexBufferObject() {
	if (m_id == INVALID_ID) {
		return;
	}

	glDeleteBuffers(1, &m_id);
}

VertexBufferObject::VertexBufferObject(VertexBufferObject&& other) noexcept :
	m_id			{ other.m_id },
	allocatedMemory { other.allocatedMemory }
{
	other.m_id = INVALID_ID;
}

VertexBufferObject& VertexBufferObject::operator=(VertexBufferObject&& other) noexcept {
	VertexBufferObject tmp{ std::move(other) };
	swap(other);
	return *this;
}

void VertexBufferObject::swap(VertexBufferObject& rhs) {
	std::swap(m_id,				rhs.m_id);
	std::swap(allocatedMemory,	rhs.allocatedMemory);
}

GLuint VertexBufferObject::id() const {
	return m_id;
}

GLsizeiptr VertexBufferObject::size() const {
	return allocatedMemory;
}
