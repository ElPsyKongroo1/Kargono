#include "kgpch.h"

#include "Kargono/Core/Base.h"
#include "Kargono/Renderer/InputBuffer.h"
#include "Kargono/Renderer/Renderer.h"

#include "API/RenderingAPI/OpenGLBuffer.h"


namespace Kargono {
	Ref<VertexBuffer> VertexBuffer::Create(uint32_t size)
	{
		return CreateRef<API::OpenGL::OpenGLVertexBuffer>(size);
	}
	Ref<VertexBuffer> VertexBuffer::Create(float* vertices, uint32_t size)
	{
		return CreateRef<API::OpenGL::OpenGLVertexBuffer>(vertices, size);
	}

	Ref<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t count)
	{
		return CreateRef<API::OpenGL::OpenGLIndexBuffer>(indices, count);
	}

}
