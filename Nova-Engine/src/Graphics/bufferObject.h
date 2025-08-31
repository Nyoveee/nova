#pragma once
#include <limits>
#include <vector>

#include "Logger.h"
using GLuint = unsigned int;

// We use the copy-and-swap idiom (but really move and swap idiom since copy semantics are disabled) to implement move semantics.
class BufferObject {
public:
	BufferObject(GLsizeiptr amountOfMemory);

	~BufferObject();
	BufferObject(BufferObject  const& other) = delete;
	BufferObject(BufferObject&& other) noexcept;
	BufferObject& operator=(BufferObject  const& other) = delete;
	BufferObject& operator=(BufferObject&& other) noexcept;

	void swap(BufferObject& rhs);

public:
	GLuint id() const;
	GLsizeiptr size() const;

	template <typename T>
	void uploadData(std::vector<T> const& vertices, GLintptr offset = 0);

private:
	GLuint m_id;
	GLsizeiptr allocatedMemory;
};

#include <iostream>

template<typename T>
void BufferObject::uploadData(std::vector<T> const& data, GLintptr offset) {
	GLsizeiptr memoryRequired = data.size() * sizeof(T);

	if (offset + memoryRequired > allocatedMemory) {
		Logger::error("Attempting to upload data of more memory than currently allocated!");
		Logger::error("Allocated memory: {} bytes, size of uploaded data: {} bytes.", allocatedMemory, memoryRequired);
		Logger::error("Number of data: {}, sizeof(Data): {} bytes.", data.size(), sizeof(T));
		return;
	}
	
	glNamedBufferSubData(m_id, offset, data.size() * sizeof(T), data.data());
}
