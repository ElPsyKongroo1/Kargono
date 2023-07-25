#pragma once
#include "Kargono/Renderer/Buffer.h"

namespace Kargono
{
/// @class VertexArray
	class VertexArray
	{
	public:
/// @brief Destructor for the VertexArray class
		virtual ~VertexArray() {}
	public:

/// @brief Binds the vertex array for rendering
		virtual void Bind() const = 0;
/// @brief Unbinds the vertex array
		virtual void Unbind() const = 0;
/// @brief Adds a vertex buffer to the vertex array
		virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) = 0;
/// @brief Sets the index buffer for the vertex array
		virtual void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) = 0;

/// @brief Returns a reference to the vector of vertex buffers
		virtual const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const = 0;
/// @brief Returns a reference to the index buffer
		virtual const std::shared_ptr<IndexBuffer>& GetIndexBuffer() const = 0;

/// @brief Creates a new VertexArray object
		static VertexArray* Create();
	};
}