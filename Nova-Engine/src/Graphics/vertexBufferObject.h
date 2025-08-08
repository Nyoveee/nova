#pragma once
#include <limits>
#include <vector>
#include <iostream>

using GLuint = unsigned int;

// A VBO holding INVALID_ID means it's not holding to any dynamically allocated resource.
constexpr GLuint INVALID_ID = std::numeric_limits<GLuint>::max();

// We use the copy-and-swap idiom (but really move and swap idiom since copy semantics are disabled) to implement move semantics.

class VertexBufferObject {
public:
	enum class Usage {
		StaticDraw,
		DynamicDraw,
		StreamDraw
	};

public:
	VertexBufferObject(GLsizeiptr amountOfMemory, Usage usage = Usage::DynamicDraw);

	~VertexBufferObject();
	VertexBufferObject(VertexBufferObject const& other) = delete;
	VertexBufferObject(VertexBufferObject&& other) noexcept;
	VertexBufferObject& operator=(VertexBufferObject const& other) = delete;
	VertexBufferObject& operator=(VertexBufferObject&& other) noexcept;

	void swap(VertexBufferObject& rhs);

public:
	GLuint id() const;
	GLsizeiptr size() const;

	template <typename T>
	void uploadData(std::vector<T> const& vertices);

private:
	GLuint m_id;
	GLsizeiptr allocatedMemory;
};

template<typename T>
void VertexBufferObject::uploadData(std::vector<T> const& vertices) {
	GLsizeiptr memoryRequired = vertices.size() * sizeof(T);

	if (memoryRequired > allocatedMemory) {
		std::cerr << "Attempting to upload data of more memory than currently allocated!\n";
		std::cerr << "Allocated memory: " << allocatedMemory << " bytes, size of uploaded data: " << memoryRequired << " bytes.";
		std::cerr << "Vertices: " << vertices.size() << ", sizeof(Vertex): " << sizeof(T) << "\n";
		return;
	}

	glNamedBufferSubData(m_id, 0, vertices.size() * sizeof(T), vertices.data());
}
