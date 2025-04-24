#include "kgpch.h"

#include "RenderingPlugin/VertexArray.h"
#include "RenderingPlugin/RenderingService.h"

#include "RenderingPlugin/ExternalAPI/OpenGLVertexArray.h"

namespace Kargono::Rendering
{

	Ref<VertexArray> VertexArray::Create()
	{
#ifdef KG_RENDERER_OPENGL
		return CreateRef<API::RenderingAPI::OpenGLVertexArray>();
#endif
	}


}
