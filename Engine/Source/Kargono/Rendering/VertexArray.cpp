#include "kgpch.h"

#include "Kargono/Rendering/VertexArray.h"
#include "Kargono/Rendering/RenderingEngine.h"

#include "API/RenderingAPI/OpenGLVertexArray.h"

namespace Kargono::Rendering
{

	Ref<VertexArray> VertexArray::Create()
	{
		return CreateRef<API::RenderingAPI::OpenGLVertexArray>();
	}


}
