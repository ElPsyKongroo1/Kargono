#include "Kargono/kgpch.h"
#include "Kargono/Renderer/Buffer.h"
#include "Renderer.h"
#include "Kargono/Core.h"
#include "Kargono/Log.h"
#include "Platform/OpenGL/OpenGLBuffer.h"


namespace Kargono {

	VertexBuffer* VertexBuffer::Create(float* vertices, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::None:
			KG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
			break;
		
		case RendererAPI::OpenGL:
			return new OpenGLVertexBuffer(vertices, size);
			break;
		}
		KG_CORE_ASSERT(false, "RendererAPI:: Unknown RendererAPI!");
		return nullptr;
	}

	IndexBuffer* IndexBuffer::Create(uint32_t* indices, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::None:
			KG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
			break;

		case RendererAPI::OpenGL:
			return new OpenGLIndexBuffer(indices, size);
			break;
		}
		KG_CORE_ASSERT(false, "RendererAPI:: Unknown RendererAPI!");
		return nullptr;
	}

}
