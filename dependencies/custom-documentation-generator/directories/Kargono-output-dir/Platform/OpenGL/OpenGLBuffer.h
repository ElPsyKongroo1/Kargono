#pragma once
#include "Kargono/Renderer/Buffer.h"

namespace Kargono {

/// @class OpenGLVertexBuffer
/// @brief This class is an implementation of the VertexBuffer interface
/// @details It is used to create and manage OpenGL vertex buffers
	class OpenGLVertexBuffer : public VertexBuffer
	{
	public:
/// @brief Constructs an OpenGL vertex buffer object
/// @param vertices An array of vertices
/// @param size The size of the vertex buffer
		OpenGLVertexBuffer(float* vertices, uint32_t size);
/// @brief Destructor for the OpenGL vertex buffer object
		virtual ~OpenGLVertexBuffer() override;
	public:
/// @brief Binds the vertex buffer object
/// @brief Binds the index buffer object
		virtual void Bind() const override;
/// @brief Unbinds the vertex buffer object
/// @brief Unbinds the index buffer object
		virtual void Unbind() const override;

/// @brief Retrieves the layout of the vertex buffer object
/// @return The layout of the vertex buffer object
		virtual const BufferLayout& GetLayout() const override { return m_Layout; }
/// @brief Sets the layout of the vertex buffer object
/// @param layout The layout of the vertex buffer object
		virtual void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }
	private:
		uint32_t m_RendererID;
		BufferLayout m_Layout;
	};

/// @class OpenGLIndexBuffer
/// @brief This class is an implementation of the IndexBuffer interface
/// @details It is used to create and manage OpenGL index buffers
	class OpenGLIndexBuffer : public IndexBuffer
	{
	public:
/// @brief Constructs an OpenGL index buffer object
/// @param indices An array of indices
/// @param count The number of indices in the index buffer
		OpenGLIndexBuffer(uint32_t* indices, uint32_t count);
/// @brief Destructor for the OpenGL index buffer object
		virtual ~OpenGLIndexBuffer() override;
	public:
		virtual void Bind() const override;
		virtual void Unbind() const override;

/// @brief Retrieves the number of indices in the index buffer
/// @return The number of indices in the index buffer
		virtual uint32_t GetCount() const override { return m_Count; }
	private:
		uint32_t m_RendererID;
		uint32_t m_Count;
	};
}

