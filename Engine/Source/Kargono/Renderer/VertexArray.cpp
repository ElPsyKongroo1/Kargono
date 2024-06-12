#include "kgpch.h"

#include "Kargono/Renderer/VertexArray.h"
#include "Kargono/Renderer/Renderer.h"

#include "API/RenderingAPI/OpenGLVertexArray.h"

namespace Kargono {

	Ref<VertexArray> VertexArray::Create()
	{
		return CreateRef<API::RenderingAPI::OpenGLVertexArray>();
	}


}
