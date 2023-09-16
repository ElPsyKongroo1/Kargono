#include "kgpch.h"

#include "Kargono/Renderer/VertexArray.h"
#include "Kargono/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"

namespace Kargono {

	Ref<VertexArray> VertexArray::Create()
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			KG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
			break;

		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLVertexArray>();
			break;
		}
		KG_CORE_ASSERT(false, "RendererAPI:: Unknown RendererAPI!");
		return nullptr;
	}


}
