#include "kgpch.h"

#include "Kargono/Core/Base.h"
#include "RenderModule/InputBuffer.h"
#include "RenderModule/RenderingService.h"

#include "RenderModule/ExternalAPI/OpenGLBuffer.h"


namespace Kargono::Rendering
{
	Ref<VertexBuffer> VertexBuffer::Create(uint32_t size)
	{
#ifdef KG_RENDERER_OPENGL
		return CreateRef<API::RenderingAPI::OpenGLVertexBuffer>(size);
#endif
	}
	Ref<VertexBuffer> VertexBuffer::Create(float* vertices, uint32_t size)
	{
#ifdef KG_RENDERER_OPENGL
		return CreateRef<API::RenderingAPI::OpenGLVertexBuffer>(vertices, size);
#endif
	}

	Ref<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t count)
	{
#ifdef KG_RENDERER_OPENGL
		return CreateRef<API::RenderingAPI::OpenGLIndexBuffer>(indices, count);
#endif
	}

}
