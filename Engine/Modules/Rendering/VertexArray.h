#pragma once

#include "Modules/Rendering/InputBuffer.h"

namespace Kargono::Rendering
{
	class VertexArray
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		VertexArray() = default;
		~VertexArray();
	public:
		//==============================
		// Interact With Renderer
		//==============================
		// Register vertex array w/ renderer
		void RegisterArray();
		void DeregisterArray();
		// Binding w/ OpenGl state machine
		void Bind() const;
		void Unbind() const;
		// Add buffer(s) to renderer
		void AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer);
		void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer);
	public:
		//==============================
		// Getters/Setters
		//==============================
		const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const { return m_VertexBuffers; }
		const Ref<IndexBuffer>& GetIndexBuffer() const { return m_IndexBuffer; }
	private:
		//==============================
		// Internal Fields
		//==============================
		bool m_Registered{ false };
		std::vector<Ref<VertexBuffer>> m_VertexBuffers{};
		Ref<IndexBuffer> m_IndexBuffer{ nullptr };
		uint32_t m_VertexBufferIndex{ 0 };
		uint32_t m_RendererID{ 0 };
	};
}
