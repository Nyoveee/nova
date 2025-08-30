#include <glad/glad.h>
#include <algorithm>

#include "bufferObject.h"

namespace {
	// A VBO holding INVALID_ID means it's not holding to any dynamically allocated resource.
	constexpr inline GLuint INVALID_ID = std::numeric_limits<GLuint>::max();
}

BufferObject::BufferObject(GLsizeiptr amountOfMemory) :
	allocatedMemory	{ amountOfMemory },
	m_id			{ INVALID_ID }
{
	glCreateBuffers(1, &m_id);
	glNamedBufferStorage(m_id, amountOfMemory, nullptr, GL_DYNAMIC_STORAGE_BIT);
}

BufferObject ::~BufferObject() {
	if (m_id == INVALID_ID) {
		return;
	}

	glDeleteBuffers(1, &m_id);
}

BufferObject::BufferObject(BufferObject&& other) noexcept :
	m_id			{ other.m_id },
	allocatedMemory { other.allocatedMemory }
{
	other.m_id = INVALID_ID;
}

BufferObject& BufferObject ::operator=(BufferObject&& other) noexcept {
	BufferObject tmp{ std::move(other) };
	swap(other);
	return *this;
}

void BufferObject::swap(BufferObject& rhs) {
	std::swap(m_id,				rhs.m_id);
	std::swap(allocatedMemory,	rhs.allocatedMemory);
}

GLuint BufferObject::id() const {
	return m_id;
}

GLsizeiptr BufferObject::size() const {
	return allocatedMemory;
}
