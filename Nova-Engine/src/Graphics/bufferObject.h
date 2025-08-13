#pragma once
#include <limits>
#include <vector>

using GLuint = unsigned int;

// We use the copy-and-swap idiom (but really move and swap idiom since copy semantics are disabled) to implement move semantics.
class BufferObject {
public:
	enum class Usage {
		StaticDraw,
		DynamicDraw,
		StreamDraw
	};

	enum class Type {
		VertexBuffer,
		ElememtBuffer,
	};

public:
	BufferObject(GLsizeiptr amountOfMemory, Type type, Usage usage = Usage::DynamicDraw);

	~BufferObject();
	BufferObject(BufferObject  const& other) = delete;
	BufferObject(BufferObject&& other) noexcept;
	BufferObject& operator=(BufferObject  const& other) = delete;
	BufferObject& operator=(BufferObject&& other) noexcept;

	void swap(BufferObject& rhs);

public:
	GLuint id() const;
	GLsizeiptr size() const;
	Type type() const;

	template <typename T>
	void uploadData(std::vector<T> const& vertices);

private:
	GLuint m_id;
	GLsizeiptr allocatedMemory;
	Type m_type;
};

#include <iostream>

template<typename T>
void BufferObject::uploadData(std::vector<T> const& vertices) {
	GLsizeiptr memoryRequired = vertices.size() * sizeof(T);

	if (memoryRequired > allocatedMemory) {
		std::cerr << "Attempting to upload data of more memory than currently allocated!\n";
		std::cerr << "Allocated memory: " << allocatedMemory << " bytes, size of uploaded data: " << memoryRequired << " bytes.";
		std::cerr << "Vertices: " << vertices.size() << ", sizeof(Vertex): " << sizeof(T) << "\n";
		return;
	}

	glNamedBufferSubData(m_id, 0, vertices.size() * sizeof(T), vertices.data());
}
