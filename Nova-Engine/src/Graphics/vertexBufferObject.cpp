#include <glad/glad.h>
#include <algorithm>

#include "VertexBufferObject.h"

VertexBufferObject::VertexBufferObject() : m_id{ INVALID_ID } {
	glCreateBuffers(1, &m_id);
}

VertexBufferObject::~VertexBufferObject() {
	if (m_id == INVALID_ID) {
		return;
	}

	glDeleteBuffers(1, &m_id);
}

VertexBufferObject::VertexBufferObject(VertexBufferObject&& other) noexcept :
	m_id{ other.m_id }
{
	other.m_id = INVALID_ID;
}

VertexBufferObject& VertexBufferObject::operator=(VertexBufferObject&& other) noexcept {
	VertexBufferObject tmp{ std::move(other) };
	swap(other);
	return *this;
}

void VertexBufferObject::swap(VertexBufferObject& rhs) {
	std::swap(m_id, rhs.m_id);
}

GLuint VertexBufferObject::id() const {
	return m_id;
}
