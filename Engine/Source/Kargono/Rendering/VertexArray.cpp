#include "kgpch.h"

#include "Kargono/Rendering/VertexArray.h"
#include "Kargono/Rendering/RenderingEngine.h"

#include "API/RenderingAPI/OpenGLVertexArray.h"

namespace Kargono::Rendering
{

	Ref<VertexArray> VertexArray::Create()
	{
#ifdef KG_RENDERER_OPENGL
		return CreateRef<API::RenderingAPI::OpenGLVertexArray>();
#endif
	}


}
