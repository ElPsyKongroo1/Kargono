#include "kgpch.h"

#include "RenderModule/VertexArray.h"
#include "RenderModule/RenderingService.h"

#include "RenderModule/ExternalAPI/OpenGLVertexArray.h"

namespace Kargono::Rendering
{

	Ref<VertexArray> VertexArray::Create()
	{
#ifdef KG_RENDERER_OPENGL
		return CreateRef<API::RenderingAPI::OpenGLVertexArray>();
#endif
	}


}
