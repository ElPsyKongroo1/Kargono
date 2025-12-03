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
		void AddVertexBuffer(const VertexBuffer& vertexBuffer);
		void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer);
	public:
		//==============================
		// Getters/Setters
		//==============================
		std::vector<VertexBuffer>& GetVertexBuffers()  { return m_VertexBuffers; }
		const Ref<IndexBuffer>& GetIndexBuffer() const { return m_IndexBuffer; }
	private:
		//==============================
		// Internal Fields
		//==============================
		bool m_Registered{ false };
		std::vector<VertexBuffer> m_VertexBuffers{};
		Ref<IndexBuffer> m_IndexBuffer{ nullptr };
		uint32_t m_VertexBufferIndex{ 0 };
		uint32_t m_RendererID{ 0 };
	};
}
