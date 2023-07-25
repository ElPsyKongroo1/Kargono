#pragma once
#include "Kargono/Renderer/VertexArray.h"
namespace Kargono {

/// @class OpenGLVertexArray
/// @brief Concrete implementation of the VertexArray class for the OpenGL renderer
	class OpenGLVertexArray : public VertexArray
	{
/// @brief Public section of the OpenGLVertexArray class
	public:
/// @brief Default constructor of OpenGLVertexArray class
		OpenGLVertexArray();
/// @brief Virtual destructor of OpenGLVertexArray class
		virtual ~OpenGLVertexArray();
	public:

/// @brief Binds the vertex array
		virtual void Bind() const override;
/// @brief Unbinds the vertex array
		virtual void Unbind() const override;
/// @brief Adds a vertex buffer to the vertex array
/// @param vertexBuffer The vertex buffer to add
		virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) override;
/// @brief Sets the index buffer of the vertex array
/// @param indexBuffer The index buffer to set
		virtual void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) override;

/// @brief Gets the vertex buffers of the vertex array
/// @return A vector of shared pointers to vertex buffers
		virtual const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const override { return m_VertexBuffers; }
/// @brief Gets the index buffer of the vertex array
/// @return A shared pointer to the index buffer
		virtual const std::shared_ptr<IndexBuffer>& GetIndexBuffer() const  override { return m_IndexBuffer; }
/// @brief Private section of the OpenGLVertexArray class
	private:
/// @brief Vector of shared pointers to vertex buffers
		std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;
/// @brief Shared pointer to the index buffer
		std::shared_ptr<IndexBuffer> m_IndexBuffer;
/// @brief ID of the renderer
		uint32_t m_RendererID;
	};
}

