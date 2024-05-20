#include "kgpch.h"

#include "API/OpenGL/OpenGLBuffer.h"

#include "glad/glad.h"

/// @namespace API::OpenGL
/// @brief Namespace for OpenGL related operations
namespace API::OpenGL
{
	//============================================================
	// Vertex Buffer
	//============================================================
	
/// @brief Constructor for the OpenGLVertexBuffer class.
/// @param size Size of the vertex buffer to be created.
	OpenGLVertexBuffer::OpenGLVertexBuffer(uint32_t size)
	{
		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
	}

	
/// @brief Overloaded constructor for the OpenGLVertexBuffer class.
/// @param vertices Pointer to the vertices array.
/// @param size Size of the vertices array.
	OpenGLVertexBuffer::OpenGLVertexBuffer(float* vertices, uint32_t size)
	{
		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
	}
/// @brief Destructor for the OpenGLVertexBuffer class.
	OpenGLVertexBuffer::~OpenGLVertexBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}
/// @brief Method to bind the OpenGL vertex buffer.
	void OpenGLVertexBuffer::Bind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
	}
/// @brief Method to unbind the OpenGL vertex buffer.
	void OpenGLVertexBuffer::Unbind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
/// @brief Method to set the data of the OpenGL vertex buffer.
/// @param data Pointer to the data to set.
/// @param size Size of the data.
	void OpenGLVertexBuffer::SetData(const void* data, uint32_t size)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
	}
	//============================================================
	// Index Buffer
	//============================================================
/// @brief Constructor for the OpenGLIndexBuffer class.
/// @param indices Pointer to the indices array.
/// @param count Number of indices.
	OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t* indices, uint32_t count)
		: m_Count{count}
	{
		glCreateBuffers(1, &m_RendererID);

		// GL_ELEMENT_ARRAY_BUFFER is not valid without an actively bound VAO
		// Binding with GL_ARRAY_BUFFER allows the data to be loaded regardless of VAO state. 
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
	}

/// @brief Destructor for the OpenGLIndexBuffer class.
	OpenGLIndexBuffer::~OpenGLIndexBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}

/// @brief Method to bind the OpenGL index buffer.
	void OpenGLIndexBuffer::Bind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
	}

/// @brief Method to unbind the OpenGL index buffer.
	void OpenGLIndexBuffer::Unbind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

}
