#pragma once

#include "Modules/Rendering/InputBuffer.h"

namespace Kargono::Rendering
{
	class VertexArray
	{
	public:
		//==============================
		// Create Vertex Array
		//==============================
		static Ref<VertexArray> Create();
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		VertexArray() = default;
		virtual ~VertexArray() = default;
	public:
		//==============================
		// Interact With Renderer
		//==============================
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		virtual void AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) = 0;
		virtual void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer) = 0;
	public:
		//==============================
		// Getters/Setters
		//==============================
		virtual const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const = 0;
		virtual const Ref<IndexBuffer>& GetIndexBuffer() const = 0;
	};
}
