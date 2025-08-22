#pragma once

#include <spdlog/spdlog.h>
#include <limits>
#include <vector>

using GLuint = unsigned int;

// We use the copy-and-swap idiom (but really move and swap idiom since copy semantics are disabled) to implement move semantics.
class BufferObject {
public:
	enum class Type {
		VertexBuffer,
		ElememtBuffer,
		SSBO,
		UBO
	};

public:
	BufferObject(GLsizeiptr amountOfMemory, Type type);

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
void BufferObject::uploadData(std::vector<T> const& data) {
	GLsizeiptr memoryRequired = data.size() * sizeof(T);

	if (memoryRequired > allocatedMemory) {
		spdlog::error("Attempting to upload data of more memory than currently allocated!");
		spdlog::error("Allocated memory: {} bytes, size of uploaded data: {} bytes.", allocatedMemory, memoryRequired);
		spdlog::error("Number of data: {}, sizeof(Data): {} bytes.", data.size(), sizeof(T));
		return;
	}

	glNamedBufferSubData(m_id, 0, data.size() * sizeof(T), data.data());
}
