#pragma once
#include <limits>

using GLuint = unsigned int;

// A VBO holding INVALID_ID means it's not holding to any dynamically allocated resource.
constexpr GLuint INVALID_ID = std::numeric_limits<GLuint>::max();

// We use the copy-and-swap idiom (but really move and swap idiom since copy semantics are disabled) to implement move semantics.

class VertexBufferObject {
public:
	VertexBufferObject();

	~VertexBufferObject();
	VertexBufferObject(VertexBufferObject const& other) = delete;
	VertexBufferObject(VertexBufferObject&& other) noexcept;
	VertexBufferObject& operator=(VertexBufferObject const& other) = delete;
	VertexBufferObject& operator=(VertexBufferObject&& other) noexcept;

	void swap(VertexBufferObject& rhs);

public:
	GLuint id() const;

private:
	GLuint m_id;
};