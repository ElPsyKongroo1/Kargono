#include "kgpch.h"

#include "Kargono/Core/Base.h"
#include "Kargono/Rendering/InputBuffer.h"
#include "Kargono/Rendering/RenderingEngine.h"

#include "API/RenderingAPI/OpenGLBuffer.h"


namespace Kargono::Rendering
{
	Ref<VertexBuffer> VertexBuffer::Create(uint32_t size)
	{
		return CreateRef<API::RenderingAPI::OpenGLVertexBuffer>(size);
	}
	Ref<VertexBuffer> VertexBuffer::Create(float* vertices, uint32_t size)
	{
		return CreateRef<API::RenderingAPI::OpenGLVertexBuffer>(vertices, size);
	}

	Ref<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t count)
	{
		return CreateRef<API::RenderingAPI::OpenGLIndexBuffer>(indices, count);
	}

}
