#include "Kargono/kgpch.h"
#include "Kargono/Renderer/Buffer.h"
#include "Kargono/Renderer/Renderer.h"
#include "Kargono/Core/Base.h"
#include "Kargono/Core/Log.h"
#include "Platform/OpenGL/OpenGLBuffer.h"


namespace Kargono {
	Ref<VertexBuffer> VertexBuffer::Create(uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			KG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
			break;

		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLVertexBuffer>(size);
			break;
		}
		KG_CORE_ASSERT(false, "RendererAPI:: Unknown RendererAPI!");
		return nullptr;
	}
	Ref<VertexBuffer> VertexBuffer::Create(float* vertices, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			KG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
			break;
		
		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLVertexBuffer>(vertices, size);
			break;
		}
		KG_CORE_ASSERT(false, "RendererAPI:: Unknown RendererAPI!");
		return nullptr;
	}

	Ref<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t count)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			KG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
			break;

		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLIndexBuffer>(indices, count);
			break;
		}
		KG_CORE_ASSERT(false, "RendererAPI:: Unknown RendererAPI!");
		return nullptr;
	}

}
