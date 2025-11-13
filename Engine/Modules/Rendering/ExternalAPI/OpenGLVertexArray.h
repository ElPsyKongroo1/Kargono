#pragma once

#include "Kargono/Core/Base.h"
#include "Modules/Rendering/VertexArray.h"

#ifdef KG_RENDERER_OPENGL

namespace API::RenderingAPI
{
	class OpenGLVertexArray : public Kargono::Rendering::VertexArray
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		OpenGLVertexArray();
		virtual ~OpenGLVertexArray();
	public:
		//==============================
		// Binding Functionality
		//==============================
		virtual void Bind() const override;
		virtual void Unbind() const override;
		//==============================
		// Update OpenGL Context
		//==============================
		virtual void AddVertexBuffer(const Kargono::Ref<Kargono::Rendering::VertexBuffer>& vertexBuffer) override;
		virtual void SetIndexBuffer(const Kargono::Ref<Kargono::Rendering::IndexBuffer>& indexBuffer) override;

		//==============================
		// Getters
		//==============================
		virtual const std::vector<Kargono::Ref<Kargono::Rendering::VertexBuffer>>& GetVertexBuffers() const override { return m_VertexBuffers; }
		virtual const Kargono::Ref<Kargono::Rendering::IndexBuffer>& GetIndexBuffer() const  override { return m_IndexBuffer; }
	private:
		std::vector<Kargono::Ref<Kargono::Rendering::VertexBuffer>> m_VertexBuffers;
		Kargono::Ref<Kargono::Rendering::IndexBuffer> m_IndexBuffer;
		uint32_t m_VertexBufferIndex = 0;
		uint32_t m_RendererID;
	};
}

#endif

